#include "StateMachine.h"

#include "BaseState.h"

StateMachine::StateMachine()
{
}

void StateMachine::Update(float DeltaTime)
{
    if (!StateStack.empty())
    {
        StateStack.top()->Update(DeltaTime);
    }
}

void StateMachine::PushState(BaseState* State)
{
    //TODO: Should probably delay push/pop state to end of frame so states have the chance to complete their frame...
    State->SetOwningStateMachine(this);

    State->OnInitialized();
    State->OnEnter();

    StateStack.push(State);
}

void StateMachine::PopState()
{
    if (StateStack.empty())
    {
        return;
    }

    BaseState* TopState = StateStack.top();

    TopState->OnExit();
    TopState->OnUninitialized();

    delete TopState;

    StateStack.pop();
}

void StateMachine::Resize()
{
    if (!StateStack.empty())
    {
        StateStack.top()->OnResize();
    }
}

bool StateMachine::HasState()
{
    return !StateStack.empty();
}
