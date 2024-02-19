#pragma once
#include "Behaviour/Behaviour.h"

class TopDownBullet : public Behaviour
{
public:
    DEFINE_BEHAVIOUR(TopDownBullet);

    void Update(Scene* Scene, float DeltaTime) override;

    Vec3f Direction = Vec3f(1.0f, 0.0f, 0.0f);
private:

    const float BulletSpeed = 24.0f;
    float BulletLifeTime = 1.0f;
};

