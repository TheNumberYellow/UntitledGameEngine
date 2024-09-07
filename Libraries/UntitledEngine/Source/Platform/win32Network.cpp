#include "NetworkPlatform.h"

#include "Modules/NetworkModule.h"

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <thread>
#include <queue>

static WSADATA wsaData;

static SOCKET ServerConnectionAcceptSocket = INVALID_SOCKET;
static std::unordered_map<ClientID, SOCKET> ServerConnectionSockets;
static std::thread ServerConnectionAcceptThread;
static std::vector<std::thread> ServerConnectionThreads;
//bool ServerHasConnection;
int ServerNumConnections = 0;

static SOCKET ClientConnectionSocket = INVALID_SOCKET;
static std::thread ClientReceiveThread;

static std::queue<ClientPacket> ServerPacketQueue;
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
    if (ServerConnectionAcceptThread.joinable())
    {
        // Currently blocking on accept() - use select() to check before calling accept() later
        ServerConnectionAcceptThread.join();
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

    ServerConnectionAcceptSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ServerConnectionAcceptSocket == INVALID_SOCKET)
    {
        std::string errorString = "server connection accept socket creation failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        freeaddrinfo(result);
        return;
    }

    iResult = bind(ServerConnectionAcceptSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::string errorString = "socket bind failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        freeaddrinfo(result);
        closesocket(ServerConnectionAcceptSocket);
        return;
    }

    if (listen(ServerConnectionAcceptSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::string errorString = "socket listen failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);

        closesocket(ServerConnectionAcceptSocket);
        return;
    }

    m_ServerRunning = true;
    ServerConnectionAcceptThread = std::thread(&NetworkInterface::AcceptServerConnections, this);
    
}

bool NetworkInterface::StartClient(std::string ip)
{
    if (m_ServerRunning)
    {
        return false;
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
        return false;
    }

    ClientConnectionSocket = INVALID_SOCKET;

    // Create a SOCKET for connecting to server
    ClientConnectionSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ClientConnectionSocket == INVALID_SOCKET) {
        std::string errorString = "socket creation failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        freeaddrinfo(result);
        return false;
    }

    // Connect to server.
    iResult = connect(ClientConnectionSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::string errorString = "socket connect failed: " + std::to_string(WSAGetLastError());
        Engine::Error(errorString);
        closesocket(ClientConnectionSocket);
        ClientConnectionSocket = INVALID_SOCKET;
        return false;
    }

    freeaddrinfo(result);

    if (ClientConnectionSocket == INVALID_SOCKET) {
        std::string errorString = "could not connect to server";
        Engine::Error(errorString);
        return false;
    }

    m_ClientRunning = true;

    ClientReceiveThread = std::thread(&NetworkInterface::ClientReceiveData, this);

    return true;
}

void NetworkInterface::ServerPing()
{
    if (m_ServerRunning)
    {
        for (auto& SocketPair : ServerConnectionSockets)
        {
            auto Socket = SocketPair.second;

            const char* sendbuf = "PING";
            int iResult;
            iResult = send(Socket, sendbuf, (int)strlen(sendbuf), 0);
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

void NetworkInterface::ServerSendData(std::string data, ClientID clientID)
{
    if (m_ServerRunning)
    {
        if (ServerConnectionSockets.find(clientID) != ServerConnectionSockets.end())
        {
            const char* sendbuf = data.c_str();
            int iResult;
            iResult = send(ServerConnectionSockets[clientID], sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                std::string errorString = "send failed: " + std::to_string(WSAGetLastError());
                Engine::Error(errorString);
                return;
            }
        }
    }
}

void NetworkInterface::ServerSendDataAll(std::string data)
{
    if (m_ServerRunning)
    {
        for (auto SocketPair : ServerConnectionSockets)
        {
            auto Socket = SocketPair.second;

            const char* sendbuf = data.c_str();
            int iResult;
            iResult = send(Socket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                std::string errorString = "send all failed: " + std::to_string(WSAGetLastError());
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

bool NetworkInterface::ServerPollData(ClientPacket& packet)
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
        //const char enable = 1;
        //iResult = setsockopt(ServerConnectionAcceptSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, &enable, sizeof(enable));
        //if (iResult == SOCKET_ERROR)
        //{
        //    std::string errorString = "server listen setsockopt failed: " + std::to_string(WSAGetLastError());
        //    Engine::Error(errorString);
        //}

        //iResult = shutdown(ServerConnectionAcceptSocket, SD_BOTH);
        //if (iResult == SOCKET_ERROR) 
        //{
        //    std::string errorString = "server listen shutdown failed: " + std::to_string(WSAGetLastError());
        //    Engine::Error(errorString);
        //}

        for (auto& SocketPair : ServerConnectionSockets)
        {
            auto Socket = SocketPair.second;

            iResult = shutdown(Socket, SD_BOTH);
            if (iResult == SOCKET_ERROR) 
            {
                std::string errorString = "server shutdown failed: " + std::to_string(WSAGetLastError());
                Engine::Error(errorString);
            }
            closesocket(Socket);

        }
        
        ServerConnectionSockets.clear();

        iResult = closesocket(ServerConnectionAcceptSocket);
        if (iResult == SOCKET_ERROR)
        {
            std::string errorString = "server listen closesocket failed: " + std::to_string(WSAGetLastError());
            Engine::Error(errorString);
        }

        m_ServerRunning = false;
    }

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

    if (ServerConnectionAcceptThread.joinable())
    {
        ServerConnectionAcceptThread.join();
    }

    for (auto& ConnectionThread : ServerConnectionThreads)
    {
        if (ConnectionThread.joinable())
        {
            ConnectionThread.join();
        }
    }

    if (ClientReceiveThread.joinable())
    {
        ClientReceiveThread.join();
    }

}

void NetworkInterface::AcceptServerConnections()
{
    while (m_ServerRunning)
    {
        SOCKET NewServerConnectionSocket = INVALID_SOCKET;

        // Accept a client socket
        NewServerConnectionSocket = accept(ServerConnectionAcceptSocket, NULL, NULL);
        if (NewServerConnectionSocket == INVALID_SOCKET) {

            int lastError = WSAGetLastError();

            if (lastError == WSAEINTR)
            {
                // accept is interrupted when closing, this is normal
                closesocket(NewServerConnectionSocket);
                m_ServerRunning = false;
                return;
            }

            std::string errorString = "accept failed: " + std::to_string(lastError);
            Engine::Error(errorString);
            closesocket(ServerConnectionAcceptSocket);
            m_ServerRunning = false;
            return;
        }
        //Engine::Error("Client connected.");

        ClientID newClientID = ServerNumConnections++;

        ServerConnectionSockets[newClientID] = NewServerConnectionSocket;
        ServerConnectionThreads.push_back(std::thread(&NetworkInterface::ServerReceiveData, this, newClientID));
    }
}

void NetworkInterface::ServerReceiveData(ClientID clientID)
{
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (m_ServerRunning)
    {
        iResult = recv(ServerConnectionSockets[clientID], recvbuf, recvbuflen, 0);

        if (iResult <= 0)
        {
            Engine::Error("Client closed connection.");
            closesocket(ServerConnectionSockets[clientID]);

            ServerConnectionSockets.erase(clientID);
            //auto thisSocketIter = std::find(ServerConnectionSockets.begin(), ServerConnectionSockets.end(), )
            //ServerConnectionSockets.erase()
             
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
                ClientPacket NewPacket;
                NewPacket.packet.Data = ReceivedData;
                NewPacket.id = clientID;
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
            Engine::Error("Connection closed.");
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
