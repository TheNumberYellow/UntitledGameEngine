#pragma once
#include "Behaviour/Behaviour.h"
class TopDownPlayer :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(TopDownPlayer);

    void Update(Scene* Scene, double DeltaTime) override;

    void Hurt();

private:
    
    Model GhostModelPrototype;
    Model BulletModelPrototype;

    int Health = 3;

    Vec3f LastDir = Vec3f(1.0f, 0.0f, 0.0f);

    float CamHeight = 8.0f;

    float Speed = 10.0f;

    bool Started = false;

    const double GhostSpawnPeriod = 0.05f;
    double GhostSpawnTimer = GhostSpawnPeriod;

    const double BulletShootPeriod = 0.1f;
    double BulletShootTimer = BulletShootPeriod;

    int GhostCount = 0;

    double TimeAlive = 0.0f;
};

