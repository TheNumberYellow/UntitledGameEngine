#pragma once

#include <stack> 

class BaseState;

class StateMachine
{
    //--------------------
    // Public member functions
    //--------------------
public:
    StateMachine();

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
};