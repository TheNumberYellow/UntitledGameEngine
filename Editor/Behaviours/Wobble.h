#pragma once
#include "Behaviour/Behaviour.h"

class Wobble :
    public Behaviour
{
public:
    DEFINE_BEHAVIOUR(Wobble);

    Wobble();

    virtual void Update(float DeltaTime);
};

