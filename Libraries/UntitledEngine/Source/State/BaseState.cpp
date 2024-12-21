#include "BaseState.h"

//#include "StateMachine.h"

void BaseState::OnInitialized(ArgsList args)
{
}

void BaseState::OnUninitialized()
{

}

void BaseState::OnEnter()
{

}

void BaseState::OnExit()
{

}

void BaseState::Update(double DeltaTime)
{

}

void BaseState::OnResize()
{

}

void BaseState::SetOwningStateMachine(StateMachine* StateMachinePtr)
{
    Machine = StateMachinePtr;
}