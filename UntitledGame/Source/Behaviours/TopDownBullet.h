#pragma once
#include "Behaviour/Behaviour.h"

class TopDownBullet : public Behaviour
{
public:
    DEFINE_BEHAVIOUR(TopDownBullet);

    void Update(Scene* Scene, double DeltaTime) override;

    Vec3f Direction = Vec3f(1.0f, 0.0f, 0.0f);
private:
    PointLight* Light = nullptr;

    bool Initialized = false;
    const float BulletSpeed = 24.0f;
    float BulletLifeTime = 1.0f;
};

