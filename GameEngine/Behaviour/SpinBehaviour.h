#pragma once
#include "Behaviour.h"
class SpinBehaviour :
    public Behaviour
{
public:
    DEFINE_BEHAVIOUR(SpinBehaviour);

    SpinBehaviour();

    virtual void Update(float DeltaTime);
};

