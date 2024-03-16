#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

class GameState : public BaseState
{
public:
    virtual void OnInitialized() override;
    virtual void OnUninitialized() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update(float DeltaTime) override;
    virtual void OnResize() override;
};

