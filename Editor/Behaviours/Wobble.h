#pragma once
#include "Behaviour/Behaviour.h"

#include "Math/Math.h"

class Wobble :
    public Behaviour
{
public:
    DEFINE_BEHAVIOUR(Wobble);

    Wobble();

    virtual void Update(Scene* Scene, double DeltaTime);

private:

    bool WobblingLeft = true;
};

