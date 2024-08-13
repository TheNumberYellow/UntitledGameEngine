#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

class ClientGameState
    : public BaseState
{


public:
    virtual void OnInitialized() override;
    virtual void OnUninitialized() override;


    virtual void OnEnter() override;
    virtual void OnExit() override;


    virtual void Update(double DeltaTime) override;

    virtual void OnResize() override;

private:

    void ProcessPacketData(const std::string& data);

    Scene CurrentScene;
    GBuffer ViewportBuffer;


    std::string NameEntry = "";
    std::string IPEntry = "";
    std::string MessageEntry = "";
    bool Connected = false;

    std::vector<std::string> ReceivedMessages;

    bool InLobby = true;
};
