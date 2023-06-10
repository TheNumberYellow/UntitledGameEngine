#include "NetworkPlatform.h"

#include <winsock2.h>
#include <ws2tcpip.h>

static WSADATA wsaData;

NetworkInterface::NetworkInterface()
{
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0)
    {
        Engine::FatalError("Winsock failed to initialize.");
    }
}

NetworkInterface::~NetworkInterface()
{

}