#pragma once
#include <string>
#include <stdexcept>
#include <chrono>
#include <iomanip>
inline bool isValidTcpPort(const std::string& input) {
    try {
        // Convert input to an integer
        int port = std::stoi(input);
        // Check if the port is in the valid range and not a well-known port
        if (1024 <= port && port <= 65535) {
            return true;
        }
        else {
            return false;
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return false;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return false;
    }
}

inline int ValidatePort() {
    std::string userInput;
    

    while (1) {
        std::cout << "Enter a TCP port number: ";
        std::cin >> userInput;

        if (isValidTcpPort(userInput)) {
            std::cout << "This is a valid TCP port that can be used." << std::endl;
            return std::stoi(userInput);
        }
        else {
            std::cout << "Invalid TCP port. Please enter a number between 1024 and 65535." << std::endl;
        }
    }
}

inline bool isValidChatCapacity(const std::string& input) {
    try {
        // Convert input to an integer
        int capacity = std::stoi(input);
        // Check if the capacity is in the valid range
        if (1 <= capacity && capacity <= 64) {
            return true;
        }
        else {
            return false;
        }
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return false;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return false;
    }
}

inline int ValidateCapacity() {
    std::string userInput;

    while (1) {
        std::cout << "Enter a chat capacity (1-64): ";
        std::cin >> userInput;

        if (isValidChatCapacity(userInput)) {
            std::cout << "Valid chat capacity entered." << std::endl;
            return std::stoi(userInput);
        }
        else {
            std::cout << "Invalid capacity. Please enter a number between 1 and 64." << std::endl;
        }
    }
}
inline int tcp_recv_whole(SOCKET s, char* buf, int len)
{
    int total = 0;

    do
    {
        int ret = recv(s, buf + total, len - total, 0);
        if (ret < 1)
            return ret;
        else
            total += ret;

    } while (total < len);

    return total;
}

// loop send func
inline int tcp_send_whole(SOCKET skSocket, const char* data, uint16_t length)
{
    int result;
    int bytesSent = 0;

    while (bytesSent < length)
    {
        result = send(skSocket, (const char*)data + bytesSent, length - bytesSent, 0);

        if (result <= 0)
            return result;

        bytesSent += result;
    }

    return bytesSent;
}