#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"


class ServerGameState
    : public BaseState
{
    //--------------------
    // BaseState Implementation
    //--------------------
public:
    virtual void OnInitialized() override;
    virtual void OnUninitialized() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update(double DeltaTime) override;
    virtual void OnResize() override;

private:

    void SendLevelChangePacket(std::string levelName);

    bool InScene = false;
    Scene CurrentScene;
    Camera* ViewportCamera;
    GBuffer ViewportBuffer;

    std::vector<std::string> ReceivedMessages;

    std::unordered_map<ClientID, std::string> ClientNames;
};

