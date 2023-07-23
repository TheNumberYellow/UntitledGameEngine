#pragma once
#include "Behaviour/Behaviour.h"
class TopDownPlayer :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(TopDownPlayer);

    void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime) override;

private:
    
    Model GhostModelPrototype;

    float CamHeight = 8.0f;

    float Speed = 8.0f;

    bool Started = false;

    const float GhostSpawnPeriod = 4.0f;
    float GhostSpawnTimer = GhostSpawnPeriod;

    int GhostCount = 0;
};

