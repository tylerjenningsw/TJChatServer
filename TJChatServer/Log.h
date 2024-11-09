#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

class Log
{
public:
	
	static void logUserCommand(const std::string& message, std::string username = "Anonymous");
	static void logPublicMessage(const std::string& message);
	static void initializeLogFiles();
	static std::string getCurrentDateTime();
	static std::string m_commandLogFileName;
	static std::string m_publicMessageLogFileName;

private:
	
};
