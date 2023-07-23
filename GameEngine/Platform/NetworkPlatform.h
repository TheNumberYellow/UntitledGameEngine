#pragma once

#include "EnginePlatform.h"

//#include "..\GUID.hpp"

struct Packet;

class NetworkInterface
{
public:
    NetworkInterface();
    ~NetworkInterface();

    void StartServer();

    void StartClient(std::string ip);

    void ServerPing();
    void ClientPing();

    void ServerSendData(std::string data);
    void ClientSendData(std::string data);

    bool ServerPollData(Packet& packet);
    bool ClientPollData(Packet& packet);

    void DisconnectAll();

private:
    void AcceptServerConnections();

    void ServerReceiveData();
    void ClientReceiveData();

    bool m_ServerRunning = false;
    bool m_ClientRunning = false;
};