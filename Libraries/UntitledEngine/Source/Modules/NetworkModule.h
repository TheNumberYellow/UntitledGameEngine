#pragma once
#include "Platform/NetworkPlatform.h"

struct Packet
{
    std::string Data;
};

struct ClientPacket
{
    ClientID id;
    Packet packet;
};

class NetworkModule
{
public:
    NetworkModule(NetworkInterface& NetworkInterface);
    ~NetworkModule();

    void StartServer();

    bool StartClient(std::string ip);

    void ServerPing();
    void ClientPing();

    void ServerSendData(std::string data, ClientID id);
    void ServerSendDataAll(std::string data);
    void ClientSendData(std::string data);

    bool ServerPollData(ClientPacket& packet);
    bool ClientPollData(Packet& packet);

    void DisconnectAll();

    static NetworkModule* Get() { return s_Instance; }

private:

    NetworkInterface& m_NetworkInterface;

    static NetworkModule* s_Instance;
};

