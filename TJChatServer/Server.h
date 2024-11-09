#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ClientHandler.h"
#include "Helper.h"
#include "Log.h"
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

class Server {
public:
    Server();
    ~Server();
    int init();
    void run();
    void stop();
    void relayMessage(std::string message, SOCKET senderSocket, std::string username);
    void getConnectedUsers();
    void handleDisconnect(SOCKET &clientSocket);
    void broadcastServerInfo();
private:
    SOCKET m_listenSocket;
    SOCKET m_broadcastSocket;
    fd_set masterSet, readySet;
    ClientHandler clientHandler;
    MessageHandler messageHandler;
    uint16_t port;
    int chatCapacity;
    int currentUserCount;
    char commandChar;
    bool running;
    int acceptNewConnection();
    std::ostringstream serverInfo;
    std::string displayServerIP();
};