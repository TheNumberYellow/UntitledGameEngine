#pragma once
#include "Platform/NetworkPlatform.h"

class NetworkModule
{
public:
    NetworkModule(NetworkInterface& NetworkInterface);
    ~NetworkModule();

private:

    NetworkInterface& m_NetworkInterface;
};

