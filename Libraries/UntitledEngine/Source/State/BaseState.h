#pragma once

#include "GameEngine.h"
#include "StateMachine.h"

class BaseState
{
public:
    virtual void OnInitialized(ArgsList args);
    virtual void OnUninitialized();
    virtual void OnEnter();
    virtual void OnExit();
    virtual void Update(double DeltaTime);
    virtual void OnResize();

    void SetOwningStateMachine(StateMachine* StateMachinePtr);

protected:
    StateMachine* Machine = nullptr;
};

