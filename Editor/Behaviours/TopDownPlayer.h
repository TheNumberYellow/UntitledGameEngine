#pragma once
#include "Behaviour/Behaviour.h"
class TopDownPlayer :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(TopDownPlayer);

    void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime) override;

private:
    float CamHeight = 8.0f;

    float Speed = 8.0f;
};

