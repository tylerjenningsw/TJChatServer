#include "Server.h"

Server::Server() : m_listenSocket(INVALID_SOCKET), port(0), chatCapacity(0), commandChar('/'), running(true), currentUserCount(0) {

}

Server::~Server()
{
}                       

int Server::init() {
    std::cout << "=====================" << std::endl;
    std::cout << "=TJChatserver v0.1.0=" << std::endl;
    std::cout << "=====================" << std::endl;

    port = ValidatePort();
    chatCapacity = ValidateCapacity();
    clientHandler.SetClientHandlerServer(this);
    clientHandler.SetCommandCharacter('/');
    if (startup() != SUCCESS)
    {
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, "WSAStartup failed.");
        return STARTUP_ERROR;
    }

    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        std::ostringstream error;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        error << "Error at socket(): " << getError() << std::endl;
        shutdown();
        return SETUP_ERROR;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        
        std::ostringstream error;
        error << "Bind failed with error: " << getError() << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        close(m_listenSocket);
        shutdown();
        return BIND_ERROR;
    }

    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::ostringstream error;
        error << "Listen failed with error: " << getError() << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        close(m_listenSocket);
        shutdown();
        return SETUP_ERROR;
    }

    // Initialize the master set and ready set
    FD_ZERO(&masterSet);
    FD_ZERO(&readySet);
    FD_SET(m_listenSocket, &masterSet);

    std::cout << displayServerIP();

    m_broadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_broadcastSocket == INVALID_SOCKET) {
        std::ostringstream error;
        error << "Failed to create UDP socket" << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        return SETUP_ERROR;
    }

    // Set SO_REUSEADDR
    char broadcastOpt = 1;
    if (setsockopt(m_broadcastSocket, SOL_SOCKET, SO_REUSEADDR, &broadcastOpt, sizeof(broadcastOpt)) < 0) {
        std::ostringstream error;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        error << "Error setting SO_REUSEADDR" << std::endl;
        close(m_broadcastSocket);
        return SETUP_ERROR;
    }

    // Set SO_BROADCAST
    if (setsockopt(m_broadcastSocket, SOL_SOCKET, SO_BROADCAST, &broadcastOpt, sizeof(broadcastOpt)) < 0) {
        std::ostringstream error;
        error << "Error setting SO_BROADCAST" << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        close(m_broadcastSocket);
        return SETUP_ERROR;
    }

    // Bind UDP socket
    sockaddr_in broadcasterAddr;
    broadcasterAddr.sin_family = AF_INET;
    broadcasterAddr.sin_port = htons(0);
    broadcasterAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(m_broadcastSocket, (sockaddr*)&broadcasterAddr, sizeof(broadcasterAddr)) < 0) {
        std::ostringstream error;
        error << "UDP bind failed" << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        close(m_broadcastSocket);
        return SETUP_ERROR;
    }

    // Start broadcast interval
    std::thread broadcastThread(&Server::broadcastServerInfo, this);
    broadcastThread.detach();
    return SUCCESS;
}

void Server::run()
{
    while (running) {
        readySet = masterSet;

        if (select(0, &readySet, nullptr, nullptr, nullptr) == SOCKET_ERROR) {
            std::ostringstream error;
            error << "Select failed with error: " << getError() << std::endl;
            messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
            break;
        }

        for (int i = 0; i < masterSet.fd_count; i++) {
            SOCKET clientSocket = masterSet.fd_array[i];
            if (FD_ISSET(clientSocket, &readySet)) {
                if (clientSocket == m_listenSocket) {
                    // Accept a new connection
                    acceptNewConnection();
                }
                else {
                    // Read from client socket
                    int iResult = clientHandler.handleClient(clientSocket);
                    // Remove from set if disconnect
                    if (iResult == DISCONNECT) {
                        handleDisconnect(clientSocket);
                        std::ostringstream info;
                        info << "Client disconnected, socket removed: " << clientSocket << std::endl;
                        messageHandler.formatLogMessage(MessageHandler::LOG_INFO, info.str());
                    }
                }
            }
        }
    }

    // Close all sockets when server is no longer active
    for (int i = 0; i < masterSet.fd_count; i++) {
        close(masterSet.fd_array[i]);
    }
}

void Server::stop()
{
    shutdown(m_listenSocket, SD_BOTH);
    close(m_listenSocket);
}

int Server::acceptNewConnection()
{
	SOCKET clientSocket = accept(m_listenSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET) {
        std::ostringstream errorMsg;
		errorMsg << "Accept failed with error: " << getError() << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, errorMsg.str());

        int error = getError();
        close(m_listenSocket);
        shutdown();
        return (error == WSAEINTR) ? SHUTDOWN : CONNECT_ERROR;
    }
    if (currentUserCount >= chatCapacity) {
        std::string deny = "The Server is at max capacity.";
        const char* data = deny.c_str();

        int32_t length = static_cast<int32_t>(deny.size());
        clientHandler.sendMessage(data, length, clientSocket);
        close(clientSocket);
        return MAX_CAPACITY;
    }
	FD_SET(clientSocket, &masterSet);
    currentUserCount++;
    messageHandler.formatLogMessage(MessageHandler::LOG_INFO, "New connection accepted.\n");
    std::string welcome = "Welcome to the Server! Use /help to view the command list.";
    const char* data = welcome.c_str();

    int32_t length = static_cast<int32_t>(welcome.size());
    clientHandler.sendMessage(data, length, clientSocket);
    return SUCCESS;
}


std::string Server::displayServerIP() {
    serverInfo << "Full IP of the TCP server: 127.0.0.1:" << port << std::endl;
    serverInfo << "Chat capacity: " << chatCapacity << std::endl;
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
        std::cerr << "GetHostName failed with error: " << getError() << std::endl;
        return "";
    }
    serverInfo << "Server hostname: " << hostName << "\n";

    struct addrinfo* result = NULL, * ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostName, NULL, &hints, &result) != 0) {
        std::ostringstream error;
        error << "GetAddrInfo failed with error." << std::endl;
        messageHandler.formatLogMessage(MessageHandler::LOG_ERROR, error.str());
        return "";
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        void* addr;
        std::string ipver;
        char ipstr[INET6_ADDRSTRLEN];

        if (ptr->ai_family == AF_INET) { // IPv4
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else { // IPv6
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)ptr->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(ptr->ai_family, addr, ipstr, sizeof(ipstr));
        serverInfo << ipver << " Address: " << ipstr << "\n";
    }

    freeaddrinfo(result);
    return serverInfo.str();
}


void Server::relayMessage(std::string message, SOCKET senderSocket, std::string username)
{
    // Format the message for relay
    std::string data = messageHandler.formatChatMessage(username, message);
    Log::logPublicMessage(data);
    // Retrieve the map of connected users
    auto connectedUsers = clientHandler.GetConnectedUsers();

    // Iterate over all sockets in the master set
    for (int i = 0; i < masterSet.fd_count; i++) {
        SOCKET outSock = masterSet.fd_array[i];
        // Ensure we're not sending the message back to the sender or to the listening socket
        if (outSock != m_listenSocket && outSock != senderSocket) {
            // Check if the user associated with the socket is logged in
            auto it = connectedUsers.find(outSock);
            if (it != connectedUsers.end() && it->second.GetIsLoggedIn()) {
                // User is logged in, send the message
                int sendResult = clientHandler.sendMessage(data.c_str(), data.size(), outSock);
                if (sendResult != SUCCESS) {
                    handleDisconnect(outSock);
                }
            }
        }
    }
}


void Server::handleDisconnect(SOCKET &clientSocket)
{
    std::ostringstream info;
    info << "Handling disconnection for socket: " << clientSocket << std::endl;
    messageHandler.formatLogMessage(MessageHandler::LOG_INFO, info.str());

    close(clientSocket);
	// Remove from set to avoid select error
    FD_CLR(clientSocket, &masterSet);
    currentUserCount--;
}

void Server::broadcastServerInfo()
{
    sockaddr_in broadcastAddr;
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(8888);
    if (InetPton(AF_INET, TEXT("255.255.255.255"), &broadcastAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address. Error: " << getError() << std::endl;
        return;
    }
    while (running) {
        std::string broadcastMessage = serverInfo.str();
        int sendResult = sendto(m_broadcastSocket, broadcastMessage.c_str(), broadcastMessage.size(), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        if (sendResult < 0) {
            std::cerr << "Failed to send broadcast" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
