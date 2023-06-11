#pragma once
#include "Behaviour\Behaviour.h"
class RCCar :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(RCCar);

    void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime) override;

private:

    Vec3f AimingDirection = Vec3f(1.0f, 0.0f, 0.0f);
    float Speed = 0.0f;
    Quaternion Rotation;
};

