#pragma once
#include "Behaviour/Behaviour.h"
class Spin :
    public Behaviour
{
public:
    DEFINE_BEHAVIOUR(Spin);

    Spin();

    virtual void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime);
};

