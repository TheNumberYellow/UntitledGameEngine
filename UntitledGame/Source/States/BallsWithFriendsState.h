#pragma once
#include "State/BaseState.h"
#include "State/StateRegistry.h"

class BallsWithFriendsState :
    public BaseState
{

public:

    DEFINE_STATE(BallsWithFriendsState);

    void OnInitialized(ArgsList args) override;
    void OnUninitialized() override;

    void OnEnter() override;
    void OnExit() override;

    void Update(double DeltaTime) override;

    void OnResize() override;
};