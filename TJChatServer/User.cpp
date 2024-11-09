#include "User.h"

User::User()
{
}

User::User(const std::string& uname, const std::string& pword)
    : m_username(uname), m_password(pword), m_loggedIn(false), m_currentSocket(INVALID_SOCKET){
}

std::string User::GetUsername()
{
    return m_username;
}

std::string User::GetPassword()
{
    return m_password;
}

bool User::GetIsLoggedIn()
{
    return m_loggedIn;
}

SOCKET User::GetCurrentSocket()
{
    return m_currentSocket;
}

void User::SetUsername(const std::string& username)
{
    m_username = username;
}

void User::SetPassword(const std::string& password)
{
    m_password = password;
}

void User::SetIsLoggedIn(const bool& status)
{
    m_loggedIn = status;
}

void User::SetCurrentSocket(SOCKET curr)
{
	m_currentSocket = curr;
}
