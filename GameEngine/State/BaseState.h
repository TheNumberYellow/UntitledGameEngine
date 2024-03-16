#pragma once

#include "StateMachine.h"
//class StateMachine;

class BaseState
{
public:
    virtual void OnInitialized();
    virtual void OnUninitialized();
    virtual void OnEnter();
    virtual void OnExit();
    virtual void Update(float DeltaTime);
    virtual void OnResize();

    void SetOwningStateMachine(StateMachine* StateMachinePtr);

protected:
    StateMachine* Machine = nullptr;
};

