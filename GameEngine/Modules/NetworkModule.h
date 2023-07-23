#pragma once
#include "Platform/NetworkPlatform.h"

struct Packet
{
    std::string Data;
};

class NetworkModule
{
public:
    NetworkModule(NetworkInterface& NetworkInterface);
    ~NetworkModule();

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

    NetworkInterface& m_NetworkInterface;
};

