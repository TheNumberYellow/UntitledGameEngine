#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

class GameState : public BaseState
{
    //--------------------
    // BaseState Implementation
    //--------------------
public:
    virtual void OnInitialized() override;
    virtual void OnUninitialized() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update(float DeltaTime) override;
    virtual void OnResize() override;
    
    //--------------------
    // Public member functions
    //--------------------
public:
    void LoadScene(Scene& InScene);

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
};

