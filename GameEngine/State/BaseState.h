#pragma once
class BaseState
{
public:
    virtual void OnInitialized();
    virtual void OnUninitialized();
    virtual void OnEnter();
    virtual void OnExit();
    virtual void Update(float DeltaTime);
    virtual void OnResize();

protected:
    void ChangeState(BaseState* NewState);
};

