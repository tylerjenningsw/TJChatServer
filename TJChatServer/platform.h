#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <cstdint>

// Platform-specific library export line (in Windows, DLL export.)
#define EXPORTED extern "C" __declspec(dllexport) 

// Winsock startup function.
inline int startup()
{
	WSADATA wsadata;
	return WSAStartup(WINSOCK_VERSION, &wsadata);
}

// Winsock shutdown function
inline int shutdown()
{
	return WSACleanup();
}

// Standard sockets close() function
inline int close(SOCKET s)
{
	return closesocket(s);
}

// Standard io-control function
inline int ioctl(SOCKET s, long cmd, u_long *argp)
{
	return ioctlsocket(s, cmd, argp);
}

// A helper function to identify the last network error.
inline int getError()
{
	return WSAGetLastError();
}

/*****************
 **  Functions  **
 *****************/

// Helper function for sending TCP data.
int sendTcpData(SOCKET skSocket, const char *data, uint16_t length);


