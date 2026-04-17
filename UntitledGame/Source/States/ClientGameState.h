#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

class ClientGameState
    : public BaseState
{


public:
    virtual void OnInitialized(ArgsList args) override;
    virtual void OnUninitialized() override;


    virtual void OnEnter() override;
    virtual void OnExit() override;


    virtual void Update(double DeltaTime) override;

    virtual void OnResize() override;

private:
    Rect GetViewportRect();

    void SendInputPacket();
    void ProcessPacketData(const std::string& data);

    bool InScene = false;

    Scene CurrentScene;
    GBuffer ViewportBuffer;
    Camera* ViewportCamera;

    std::string NameEntry = "";
    std::string IPEntry = "";
    std::string MessageEntry = "";
    bool Connected = false;

    std::vector<std::string> ReceivedMessages;

    bool InLobby = true;

    // TEMP
    int PrevFrameTimeCount = 0;
    double PrevFrameTimeSum = 0.0f;
    int PrevAveFPS = 0;

    Font TestFont;
};
