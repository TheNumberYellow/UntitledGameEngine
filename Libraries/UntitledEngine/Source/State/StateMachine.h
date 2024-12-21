#pragma once

#include "GameEngine.h"

#include <stack> 

class BaseState;

class StateMachine
{
    //--------------------
    // Public member functions
    //--------------------
public:
    StateMachine();

    void Initialize(ArgsList args);

    void Update(double DeltaTime);

    void PushState(BaseState* State);
    void PopState();

    void Resize();

    bool HasState();

    //--------------------
    // Private member functions
    //--------------------
private:

    //--------------------
    // Private member variables
    //--------------------
private:
    std::stack<BaseState*> StateStack;

    ArgsList Arguments;
};