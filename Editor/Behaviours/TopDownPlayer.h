#pragma once
#include "Behaviour/Behaviour.h"
class TopDownPlayer :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(TopDownPlayer);

    void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime) override;

    void Hurt();

private:
    
    Model GhostModelPrototype;
    Model BulletModelPrototype;

    int Health = 3;

    Vec3f LastDir = Vec3f(1.0f, 0.0f, 0.0f);

    float CamHeight = 8.0f;

    float Speed = 10.0f;

    bool Started = false;

    const float GhostSpawnPeriod = 0.05f;
    float GhostSpawnTimer = GhostSpawnPeriod;

    const float BulletShootPeriod = 0.1f;
    float BulletShootTimer = BulletShootPeriod;

    int GhostCount = 0;
};

