#pragma once

class ControlInputs;

class State
{
public:

    virtual void Initialize() = 0;
    virtual void Update(ControlInputs& inputs) = 0;
};

