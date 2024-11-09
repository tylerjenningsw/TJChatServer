#pragma once
#include <string>
#include <winsock2.h>

class User
{
public:
    User();
    User(const std::string& uname, const std::string& pword);
    std::string GetUsername();
    std::string GetPassword();
    bool GetIsLoggedIn();
    SOCKET GetCurrentSocket();
    void SetUsername(const std::string& username);
    void SetPassword(const std::string& password);
    void SetIsLoggedIn(const bool& status);
    void SetCurrentSocket(SOCKET curr);
private:
    std::string m_username;
    std::string m_password;
    SOCKET m_currentSocket;
    bool m_loggedIn;
};

