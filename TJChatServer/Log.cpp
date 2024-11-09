#include "Log.h"
std::string Log::m_commandLogFileName = "";
std::string Log::m_publicMessageLogFileName = "";

void Log::logUserCommand(const std::string& message, std::string username) {
	if (m_commandLogFileName.empty()) {
		std::cerr << "Command log file name is not initialized." << std::endl;
		return;
	}
	std::ofstream logFile(m_commandLogFileName, std::ios::app);
	if (logFile.is_open()) {
		logFile << "User " << username << " used command: " << message << std::endl;
	}
	else {
		std::cerr << "Failed to open command log file." << std::endl;
	}
	logFile.close();
}

void Log::logPublicMessage(const std::string& message) {
	if (m_publicMessageLogFileName.empty()) {
		std::cerr << "Public message log file name is not initialized." << std::endl;
		return;
	}
	std::ofstream logFile(m_publicMessageLogFileName, std::ios::app);
	if (logFile.is_open()) {
		logFile << message << std::endl;
	}
	else {
		std::cerr << "Failed to open public message log file." << std::endl;
	}
	logFile.close();
}


void Log::initializeLogFiles()
{
	std::string timestamp = getCurrentDateTime();
	m_commandLogFileName = "CommandLog_" + timestamp + ".txt";
	m_publicMessageLogFileName = "PublicMessageLog_" + timestamp + ".txt";
}

std::string Log::getCurrentDateTime() {
	// Get current time
	auto now = std::chrono::system_clock::now(); 
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	// localtime_s struct
	std::tm tm_snapshot;
	errno_t ret = localtime_s(&tm_snapshot, &in_time_t);

	std::stringstream ss;
	if (ret == 0) {
		ss << std::put_time(&tm_snapshot, "%Y-%m-%d_%H-%M-%S"); // Use put_time with pointer to tm_snapshot
	}
	else {
		std::cerr << "Localtime_s failed to convert time" << std::endl;
	}
	return ss.str();
}