#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H
#define NOMINMAX
#pragma once
#include <winsock2.h>
#include <string>
#include <cstdint>
#include "definitions.h"
#include <unordered_map>
#include "User.h"
#include "MessageHandler.h"
#include <algorithm>
class Server;

class ClientHandler {
public:
    ClientHandler();
    void SetClientHandlerServer(Server* server);
    ClientHandler(SOCKET clientSocket);
    ~ClientHandler();
    char GetCommandCharacter();
    void SetCommandCharacter(const char commandChar);
    int handleClient(SOCKET& clientSocket);
    int sendMessage(const char* data, int32_t length, SOCKET &clientSocket);
    int sendBigMessage(const char* data, int32_t totalLength, SOCKET& clientSocket);
    std::unordered_map<SOCKET, User> GetConnectedUsers();
    void sendDirectMessage(SOCKET senderSocket, const std::string& targetUsername, const std::string& message);
    void logoutUser(SOCKET clientSocket);
    void sendActiveUserList(SOCKET clientSocket);
    void sendLogFile(SOCKET clientSocket);
private:
    Server* m_server;
    SOCKET m_clientSocket;
    char m_commandCharacter;
    std::unordered_map<SOCKET, User> userSession;
    std::unordered_map<std::string, User> userList;
    bool authenticateUser(SOCKET clientSocket, const std::string& username, const std::string& password);
    void processCommand(const std::string& command, SOCKET clientSocket);
    int readMessage(char* buffer, int32_t bufferSize, SOCKET clientSocket);
    void registerUser(SOCKET socket, const std::string& username, const std::string& password);
    std::string getUsernameForSocket(SOCKET clientSocket);
    void processMessage(const std::string& command, SOCKET clientSocket);
};

#endif // CLIENTHANDLER_H
