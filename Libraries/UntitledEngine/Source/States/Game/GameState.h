#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

class GameState : public BaseState
{
    //--------------------
    // BaseState Implementation
    //--------------------
public:
    virtual void OnInitialized(ArgsList args) override;
    virtual void OnUninitialized() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update(double DeltaTime) override;
    virtual void OnResize() override;
    
    //--------------------
    // Public member functions
    //--------------------
public:
    void LoadScene(Scene& InScene);
    void LoadScene(FilePath path);

    //--------------------
    // Private member functions
    //--------------------
private:
    Rect GetViewportRect();

    //--------------------
    // Private member variables
    //--------------------
private:
    Scene RuntimeScene;

    Camera* ViewportCamera;
    GBuffer ViewportBuffer;

    // TEMP
    int PrevFrameTimeCount = 0;
    double PrevFrameTimeSum = 0.0f;
    int PrevAveFPS = 0;

    Font TestFont;
};

