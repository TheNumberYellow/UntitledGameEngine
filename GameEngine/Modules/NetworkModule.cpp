#include "NetworkModule.h"

NetworkModule* NetworkModule::s_Instance = nullptr;

NetworkModule::NetworkModule(NetworkInterface& NetworkInterface)
    : m_NetworkInterface(NetworkInterface)
{
    s_Instance = this;
}

NetworkModule::~NetworkModule()
{
}

void NetworkModule::StartServer()
{
    m_NetworkInterface.StartServer();
}

bool NetworkModule::StartClient(std::string ip)
{
    return m_NetworkInterface.StartClient(ip);
}

void NetworkModule::ServerPing()
{
    m_NetworkInterface.ServerPing();
}

void NetworkModule::ClientPing()
{
    m_NetworkInterface.ClientPing();
}

void NetworkModule::ServerSendData(std::string data, ClientID id)
{
    m_NetworkInterface.ServerSendData(data, id);
}

void NetworkModule::ServerSendDataAll(std::string data)
{
    m_NetworkInterface.ServerSendDataAll(data);
}

void NetworkModule::ClientSendData(std::string data)
{
    m_NetworkInterface.ClientSendData(data);
}

bool NetworkModule::ServerPollData(ClientPacket& packet)
{
    return m_NetworkInterface.ServerPollData(packet);
}

bool NetworkModule::ClientPollData(Packet& packet)
{
    return m_NetworkInterface.ClientPollData(packet);
}

void NetworkModule::DisconnectAll()
{
    m_NetworkInterface.DisconnectAll();
}
