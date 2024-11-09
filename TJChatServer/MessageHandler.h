#pragma once
#include <string>
#include <sstream>

class MessageHandler
{
public:
    MessageHandler();
    ~MessageHandler();
    enum LogLevel {
        LOG_INFO,
        LOG_DEBUG,
        LOG_ERROR 
    };

    std::string formatChatMessage(const std::string& senderUsername, const std::string& message);
    void formatLogMessage(LogLevel level, const std::string& message);
};