#include "ClientHandler.h"
#include <iostream>
#include "Server.h"


ClientHandler::ClientHandler()
{
}

void ClientHandler::SetClientHandlerServer(Server* server)
{
    m_server = server;
}

ClientHandler::ClientHandler(SOCKET clientSocket)
{
}

ClientHandler::~ClientHandler()
{
}

char ClientHandler::GetCommandCharacter()
{
    return m_commandCharacter;
}

void ClientHandler::SetCommandCharacter(const char commandChar)
{
    m_commandCharacter = commandChar;
}

int ClientHandler::handleClient(SOCKET& clientSocket) {
    const int bufferSize = 256;
    char buffer[bufferSize] = { 0 };

    int status = readMessage(buffer, bufferSize, clientSocket);

    switch (status) {
    case SUCCESS:
        std::cout << "Received message: SOCKET " << clientSocket << std::endl;

        // Process the command received
        processCommand(std::string(buffer), clientSocket);
        break;
    case TRY_AGAIN:
        break;
    case SHUTDOWN:
    case DISCONNECT:
        std::cout << "Client disconnected: SOCKET " << clientSocket << std::endl;
        return DISCONNECT;
        break;
    case PARAMETER_ERROR:
        std::cerr << "Parameter error with SOCKET " << clientSocket << std::endl;
        break;
    default:
        break;
    }
}


bool ClientHandler::authenticateUser(SOCKET clientSocket, const std::string& username, const std::string& password) {
    auto it = userList.find(username);
    if (it != userList.end() && it->second.GetPassword() == password) {
        if (!it->second.GetIsLoggedIn()) {
            it->second.SetIsLoggedIn(true);
            it->second.SetCurrentSocket(clientSocket);
            userSession[clientSocket] = it->second;
            return true;
        }
        else {
            std::string message = "User already logged in elsewhere.";
            sendMessage(message.c_str(), message.size(), clientSocket);
            return false;
        }
    }
    return false;
}


void ClientHandler::processCommand(const std::string& command, SOCKET clientSocket) {
    std::cout << "message: "<< command <<" SOCKET " << clientSocket << std::endl;
    if (command.empty()) {
        return;
    }

    if (command.at(0) != GetCommandCharacter()) {
		// not a command
        processMessage(command, clientSocket);
        return;
    }
    // Remove the command character for easier processing
    std::string trimmedCommand = command.substr(1);

    // Find the first space to separate the command from its parameters
    size_t firstSpace = trimmedCommand.find(' ');
    std::string cmd = trimmedCommand.substr(0, firstSpace);
    std::string params = trimmedCommand.substr(firstSpace + 1);
    std::string currUsername = "";
    auto userIt = userSession.find(clientSocket);
    bool isUserLoggedIn = userIt != userSession.end() && userIt->second.GetIsLoggedIn();
	// log to user_commands.txt
    if (isUserLoggedIn == true) {		
        currUsername = userIt->second.GetUsername();
        Log::logUserCommand(command, currUsername);
    }
    else {
        Log::logUserCommand(command);
    }
    size_t secondSpace = params.find(' ');
    std::string username = params.substr(0, secondSpace);
    if ((cmd == "register" || cmd == "login") && !params.empty()) {
        if (secondSpace != std::string::npos) {
            std::string password = params.substr(secondSpace + 1);

            if (isUserLoggedIn) {
                std::string message = "You are already logged in. Please logout first.";
                sendMessage(message.c_str(), message.size(), clientSocket);
                return;
            }

            if (cmd == "register") {
                registerUser(clientSocket, username, password);
            }
            else if (cmd == "login") {
                bool success = authenticateUser(clientSocket, username, password);
                std::string message = success ? "Login successful. You may now begin chatting." : "Login failed. Please try again.";
                sendMessage(message.c_str(), message.size(), clientSocket);
            }
        }
        else {
            std::string error = "Invalid command format.";
            sendMessage(error.c_str(), error.size(), clientSocket);
        }
    }
    else if (cmd == "logout") {
        if (!isUserLoggedIn) {
            std::string message = "You are not logged in.";
            sendMessage(message.c_str(), message.size(), clientSocket);
        }
        else {
            logoutUser(clientSocket);
        }
    }
    else if (cmd == "help") {
        std::ostringstream helpStream;
        helpStream << "\nThe command character is: " << GetCommandCharacter()
            << "\n" << GetCommandCharacter() << "register username password - Register to chat on the server!\n"
            << "Example usage: " << GetCommandCharacter() << "register user1 password123\n"
            << "\n" << GetCommandCharacter() << "login username password - Login to your account after registration to begin chatting!\n"
            << "Example usage: " << GetCommandCharacter() << "login user1 password123\n"
            << "\n" << GetCommandCharacter() << "getlist - Get a list of the currently connected users.\n"
            << "\n" << GetCommandCharacter() << "getlog - Get the chat logs from this session.\n"
            << "\n" << GetCommandCharacter() << "logout - Gracefully log out of your account.\n";

        std::string helpMessage = helpStream.str();
        sendBigMessage(helpMessage.c_str(), helpMessage.length(), clientSocket);
    }
    else if (cmd == "getlog") {
        sendLogFile(clientSocket);
    }
    else if (cmd == "getlist") {
        sendActiveUserList(clientSocket);
    }
    else if (cmd == "send") {
        std::string message = params.substr(secondSpace + 1);

        sendDirectMessage(clientSocket, username, message);
    }
    else {
        std::string error = "Unknown command.";
        sendMessage(error.c_str(), error.size(), clientSocket);
    }
}

int ClientHandler::readMessage(char* buffer, int32_t bufferSize, SOCKET clientSocket) {
    uint8_t size = 0;
    int receivedBytes = tcp_recv_whole(clientSocket, (char*)&size, 1);
    if (receivedBytes <= 0) {
        std::cerr << "Receive error or connection closed by client. Bytes received: " << receivedBytes << std::endl;
        return (receivedBytes == 0) ? SHUTDOWN : DISCONNECT;
    }

    if (size >= bufferSize) {
        std::cerr << "Message size " << (int)size << " exceeds buffer limit " << bufferSize << std::endl;
        return PARAMETER_ERROR;
    }
    // Clear buffer before reading new data
    memset(buffer, 0, bufferSize);
    receivedBytes = tcp_recv_whole(clientSocket, buffer, size);
    if (receivedBytes <= 0) {
        std::cerr << "Failed to receive the full message, expected " << (int)size << " bytes, received " << receivedBytes << std::endl;
        return (receivedBytes == 0) ? SHUTDOWN : DISCONNECT;
    }

    buffer[size] = '\0';
    std::cout << "Message received successfully: [" << buffer << "]" << std::endl;
    return SUCCESS;
}

int ClientHandler::sendMessage(const char* data, int32_t length, SOCKET& clientSocket)
{
    if (length < 0 || length > 254) {
        return PARAMETER_ERROR;
    }

    unsigned char lenByte = static_cast<unsigned char>(length + 1);
    int result = tcp_send_whole(clientSocket, reinterpret_cast<const char*>(&lenByte), sizeof(lenByte));
    if (result <= 0) {
        return (result == 0) ? SHUTDOWN : DISCONNECT;
    }

    char buffer[256];
    if (length < 256) {
        strncpy_s(buffer, sizeof(buffer), data, length);
        buffer[length] = '\0';
    }

    result = tcp_send_whole(clientSocket, buffer, length + 1);
    if (result <= 0) {
        return (result == 0) ? SHUTDOWN : DISCONNECT;
    }

    return SUCCESS;
}



int ClientHandler::sendBigMessage(const char* bigData, int32_t totalLength, SOCKET& clientSocket) {
    if (totalLength <= 0) {
        return PARAMETER_ERROR;
    }

    const int MAX_CHUNK_SIZE = 253;
    int32_t bytesRemaining = totalLength;
    int32_t offset = 0;

    while (bytesRemaining > 0) {
        int32_t chunkSize = std::min(bytesRemaining, MAX_CHUNK_SIZE);
        // Create a buffer that includes space for the null terminator
        char buffer[MAX_CHUNK_SIZE + 1];
        std::memcpy(buffer, bigData + offset, chunkSize);
        buffer[chunkSize] = '\0'; // Set the null terminator

        int sendResult = sendMessage(buffer, chunkSize + 1, clientSocket); // Send chunkSize + 1 to include the null terminator
        if (sendResult != SUCCESS) {
            return sendResult;
        }

        offset += chunkSize;
        bytesRemaining -= chunkSize;
    }

    return SUCCESS;
}

std::unordered_map<SOCKET, User> ClientHandler::GetConnectedUsers()
{
    return userSession;
}



void ClientHandler::registerUser(SOCKET socket, const std::string& username, const std::string& password)
{
    // Prevent registration if user is already registered
    if (userList.find(username) != userList.end()) {
        std::string error = "User already registered. Please login.";
        sendMessage(error.c_str(), error.size(), socket);
        return;
    }

    // Register new user
    User newUser(username, password);
    newUser.SetCurrentSocket(socket);
	// Add to total users
    userList[username] = newUser;
    // Add to active sessions
    userSession[socket] = newUser;

    std::string success = "Registration successful. Please log in using /login.";
    sendMessage(success.c_str(), success.size(), socket);
}

void ClientHandler::processMessage(const std::string& command, SOCKET clientSocket)
{
    auto it = userSession.find(clientSocket);
    if (it != userSession.end() && it->second.GetIsLoggedIn()) {
        std::string username = it->second.GetUsername();

        m_server->relayMessage(command.c_str(), clientSocket, username);
    }
    else {
        std::string error = "You must be logged in to chat. Use /help";
        sendMessage(error.c_str(), error.size(), clientSocket);
    }
}

void ClientHandler::sendDirectMessage(SOCKET senderSocket, const std::string& targetUsername, const std::string& message) {
    // Check if target username exists and user is logged in
    auto targetUserIt = userList.find(targetUsername);
    if (targetUserIt == userList.end() || !targetUserIt->second.GetIsLoggedIn()) {
        std::string error = "Target user not found or not logged in.";
        sendMessage(error.c_str(), error.size(), senderSocket);
        return;
    }

    // Check if target socket is valid and not the same as the sender's socket
    SOCKET targetSocket = targetUserIt->second.GetCurrentSocket();
    std::cout << "SOCKET: " << senderSocket << " sending to: " << targetUsername << " with target socket: " << targetSocket << std::endl;
    if (targetSocket == INVALID_SOCKET || targetSocket == senderSocket) {
        std::string error = "User is not connected or cannot send message to yourself.";
        sendMessage(error.c_str(), error.size(), senderSocket);
        return;
    }

    // Send the message to the target user
    std::string fullMessage = "From " + userSession[senderSocket].GetUsername() + ": " + message;
    sendMessage(fullMessage.c_str(), fullMessage.size(), targetSocket);
}

void ClientHandler::sendLogFile(SOCKET clientSocket) {
    std::ifstream logFile(Log::m_publicMessageLogFileName, std::ios::in | std::ios::binary);

    if (!logFile.is_open()) {
        std::string error = "Failed to open log file: " + Log::m_publicMessageLogFileName;
        sendMessage(error.c_str(), error.size(), clientSocket);
        return;
    }

    // Read the entire log file into a string
    std::string fileContents((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
    logFile.close();

    // Send the log file to the client in chunks
    sendBigMessage(fileContents.c_str(), fileContents.length(), clientSocket);
}


void ClientHandler::sendActiveUserList(SOCKET clientSocket) {
    std::ostringstream userListStream;
    userListStream << "Active users:\n";

    // Iterate over the user sessions to make a list of logged in users
    for (auto& userPair : userSession) {
        if (userPair.second.GetIsLoggedIn()) {
            userListStream << userPair.second.GetUsername() << "\n";
        }
    }

    std::string userList = userListStream.str();
    sendMessage(userList.c_str(), userList.length(), clientSocket);
}


void ClientHandler::logoutUser(SOCKET clientSocket) {
    auto it = userSession.find(clientSocket);
    std::string username = it->second.GetUsername();
	
    if (it != userSession.end()) {
        it->second.SetIsLoggedIn(false);
        std::string message = "Logout successful. Come back soon!";
        sendMessage(message.c_str(), message.size(), clientSocket);
        userList[username].SetIsLoggedIn(false);
        userList[username].SetCurrentSocket(INVALID_SOCKET);
        userSession.erase(clientSocket);
		//fin/ack/fin/ack
        m_server->handleDisconnect(clientSocket);
    }
    else {
        std::string error = "You are not currently logged in.";
        sendMessage(error.c_str(), error.size(), clientSocket);
    }
}
