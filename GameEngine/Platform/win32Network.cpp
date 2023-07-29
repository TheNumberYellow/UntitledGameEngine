#include "NetworkPlatform.h"

#include "Modules/NetworkModule.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <queue>

static WSADATA wsaData;

static SOCKET ServerListenSocket = INVALID_SOCKET;
static SOCKET ServerConnectionSocket = INVALID_SOCKET;
static std::thread ConnectionAcceptThread;
static std::thread ServerReceiveThread;
bool ServerHasConnection;

static SOCKET ClientConnectionSocket = INVALID_SOCKET;
static std::thread ClientReceiveThread;

static std::queue<Packet> ServerPacketQueue;
static std::queue<Packet> ClientPacketQueue;

#define DEFAULT_PORT "25903"
#define DEFAULT_BUFLEN 512

NetworkInterface::NetworkInterface()
{
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(23, 2), &wsaData);

    if (iResult != 0)
    {
        Engine::FatalError("Winsock failed to initialize.");
    }
}

NetworkInterface::~NetworkInterface()
{
    DisconnectAll();

    WSACleanup();
}

void NetworkInterface::StartServer()
{
    if (m_ClientRunning)
    {
        return;
    }

    if (m_ServerRunning)
    {
        m_ServerRunning = false;
    }
    if (ConnectionAcceptThread.joinable())
    {
        // Currently blocking on accept() - use select() to check before calling accept() later
        ConnectionAcceptThread.join();
    }

    addrinfo* result = nullptr;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    INT iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        std::string errorString = "getaddrinfo failed: " + std::to_string(iResult);
        Engine::Error(errorString);
        return;
    }

    ServerListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ServerListenSocket == INVALID_SOCKET)
    {
        std::string errorString = "socket creation failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        freeaddrinfo(result);
        return;
    }

    iResult = bind(ServerListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::string errorString = "socket bind failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        freeaddrinfo(result);
        closesocket(ServerListenSocket);
        return;
    }

    if (listen(ServerListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::string errorString = "socket listen failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);

        closesocket(ServerListenSocket);
        return;
    }

    m_ServerRunning = true;
    ConnectionAcceptThread = std::thread(&NetworkInterface::AcceptServerConnections, this);
    
}

void NetworkInterface::StartClient(std::string ip)
{
    if (m_ServerRunning)
    {
        return;
    }

    addrinfo* result = nullptr;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    INT iResult = getaddrinfo(ip.c_str(), DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        std::string errorString = "getaddrinfo failed: " + std::to_string(iResult);
        Engine::Error(errorString);
        return;
    }

    ClientConnectionSocket = INVALID_SOCKET;

    // Create a SOCKET for connecting to server
    ClientConnectionSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ClientConnectionSocket == INVALID_SOCKET) {
        std::string errorString = "socket creation failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        freeaddrinfo(result);
        return;
    }

    // Connect to server.
    iResult = connect(ClientConnectionSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::string errorString = "socket connect failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        closesocket(ClientConnectionSocket);
        ClientConnectionSocket = INVALID_SOCKET;
        return;
    }

    freeaddrinfo(result);

    if (ClientConnectionSocket == INVALID_SOCKET) {
        std::string errorString = "could not connect to server";
        Engine::Error(errorString);
        return;
    }

    m_ClientRunning = true;

    ClientReceiveThread = std::thread(&NetworkInterface::ClientReceiveData, this);
}

void NetworkInterface::ServerPing()
{
    if (m_ServerRunning)
    {
        if (ServerHasConnection)
        {
            const char* sendbuf = "PING";
            int iResult;
            iResult = send(ServerConnectionSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                std::string errorString = "send failed: " + std::to_string(WSAGetLastError());
                Engine::Error(errorString);
                return;
            }
        }
    }
}

void NetworkInterface::ClientPing()
{
    if (m_ClientRunning)
    {
        const char* sendbuf = "PING";
        int iResult;
        iResult = send(ClientConnectionSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            std::string errorString = "send failed: " + std::to_string(WSAGetLastError());
            Engine::Error(errorString);
            return;
        }
    }
}

void NetworkInterface::ServerSendData(std::string data)
{
    if (m_ServerRunning)
    {
        if (ServerHasConnection)
        {
            const char* sendbuf = data.c_str();
            int iResult;
            iResult = send(ServerConnectionSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                std::string errorString = "send failed: " + std::to_string(WSAGetLastError());
                Engine::Error(errorString);
                return;
            }
        }
    }
}

void NetworkInterface::ClientSendData(std::string data)
{
    if (m_ClientRunning)
    {
        const char* sendbuf = data.c_str();
        int iResult;
        iResult = send(ClientConnectionSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            std::string errorString = "send failed: " + std::to_string(WSAGetLastError());
            Engine::Error(errorString);
            return;
        }
    }
}

bool NetworkInterface::ServerPollData(Packet& packet)
{
    if (ServerPacketQueue.empty())
    {
        return false;
    }
    packet = ServerPacketQueue.front();
    ServerPacketQueue.pop();

    return true;
}

bool NetworkInterface::ClientPollData(Packet& packet)
{
    if (ClientPacketQueue.empty())
    {
        return false;
    }
    packet = ClientPacketQueue.front();
    ClientPacketQueue.pop();

    return true;
}

void NetworkInterface::DisconnectAll()
{
    int iResult;

    if (m_ServerRunning)
    {
        iResult = shutdown(ServerListenSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            std::string errorString = "server listen shutdown failed: " + std::to_string(WSAGetLastError());
            Engine::Error(errorString);
        }

        if (ServerHasConnection)
        {
            iResult = shutdown(ServerConnectionSocket, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                std::string errorString = "server shutdown failed: " + std::to_string(WSAGetLastError());
                Engine::Error(errorString);
            }
        }

        m_ServerRunning = false;
    }
    closesocket(ServerListenSocket);
    closesocket(ServerConnectionSocket);

    if (m_ClientRunning)
    {
        iResult = shutdown(ClientConnectionSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            std::string errorString = "client shutdown failed: " + std::to_string(WSAGetLastError());
            Engine::Error(errorString);            
        }
        m_ClientRunning = false;
    }
    closesocket(ClientConnectionSocket);

    ServerHasConnection = false;

    if (ConnectionAcceptThread.joinable())
    {
        ConnectionAcceptThread.join();
    }

    if (ServerReceiveThread.joinable())
    {
        ServerReceiveThread.join();
    }

    if (ClientReceiveThread.joinable())
    {
        ClientReceiveThread.join();
    }

    closesocket(ServerListenSocket);
}

void NetworkInterface::AcceptServerConnections()
{
    while (m_ServerRunning )
    {
        ServerConnectionSocket = INVALID_SOCKET;

        // Accept a client socket
        ServerConnectionSocket = accept(ServerListenSocket, NULL, NULL);
        if (ServerConnectionSocket == INVALID_SOCKET) {
            std::string errorString = "accept failed: " + std::to_string(WSAGetLastError());
            Engine::Error(errorString);
            closesocket(ServerListenSocket);
            m_ServerRunning = false;
            return;
        }
        Engine::Error("Client connected.");
        ServerHasConnection = true;
        ServerReceiveThread = std::thread(&NetworkInterface::ServerReceiveData, this);
        break;
    }
}

void NetworkInterface::ServerReceiveData()
{
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (m_ServerRunning)
    {
        iResult = recv(ServerConnectionSocket, recvbuf, recvbuflen, 0);

        if (iResult <= 0)
        {
            Engine::Error("Client closed connection.");
            closesocket(ServerConnectionSocket);
            ServerHasConnection = false;
            break;
        }
        else
        {
            std::string ReceivedData;
            for (int i = 0; i < iResult; ++i)
            {
                ReceivedData += recvbuf[i];
            }
            if (ReceivedData == "PING")
            {
                Engine::Error("Received ping from client.");
            }
            else
            {
                Packet NewPacket;
                NewPacket.Data = ReceivedData;
                ServerPacketQueue.push(NewPacket);
            }
        }
    }
}

void NetworkInterface::ClientReceiveData()
{
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (m_ClientRunning)
    {
        iResult = recv(ClientConnectionSocket, recvbuf, recvbuflen, 0);
    
        if (iResult <= 0)
        {
            Engine::Error("Server closed connection.");
            break;
        }
        else
        {
            std::string ReceivedData = recvbuf;
            if (ReceivedData == "PING")
            {
                Engine::Error("Received ping from server.");
            }
            else
            {
                Packet NewPacket;
                NewPacket.Data = ReceivedData;
                ClientPacketQueue.push(NewPacket);
            }
        }
    }
}
