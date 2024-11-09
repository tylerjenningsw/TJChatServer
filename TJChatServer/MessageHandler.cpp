#include "MessageHandler.h"

MessageHandler::MessageHandler()
{
}

MessageHandler::~MessageHandler()
{
}

std::string MessageHandler::formatChatMessage(const std::string& senderUsername, const std::string& message) {
    std::ostringstream ss;
    ss << senderUsername << ": " << message;
    return ss.str();
}

void MessageHandler::formatLogMessage(LogLevel level, const std::string& message) {
    std::ostringstream ss;
    switch (level) {
    case LOG_INFO:
        ss << "[INFO]: ";
        break;
    case LOG_DEBUG:
        ss << "[DEBUG]: ";
        break;
    case LOG_ERROR:
        ss << "[ERROR]: ";
        break;
    }
    ss << message;
    printf("%s\n", ss.str().c_str());
}