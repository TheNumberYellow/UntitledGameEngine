#pragma once

#include "EnginePlatform.h"

//#include "..\GUID.hpp"

struct Packet;
struct ClientPacket;

using ClientID = size_t;

class NetworkInterface
{

public:
    NetworkInterface();
    ~NetworkInterface();

    void StartServer();

    bool StartClient(std::string ip);

    void ServerPing();
    void ClientPing();

    void ServerSendData(std::string data, ClientID clientID);
    void ServerSendDataAll(std::string data);

    void ClientSendData(std::string data);

    bool ServerPollData(ClientPacket& packet);
    bool ClientPollData(Packet& packet);

    void DisconnectAll();

private:
    void AcceptServerConnections();

    void ServerReceiveData(ClientID clientID);
    void ClientReceiveData();

    bool m_ServerRunning = false;
    bool m_ClientRunning = false;
};