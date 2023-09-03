#pragma once
#include "Behaviour\Behaviour.h"
class RCCar :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(RCCar);

    void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime) override;

private:

    Vec3f FrontLeftTire;
    Vec3f FrontRightTire;
    Vec3f BackLeftTire;
    Vec3f BackRightTire;

    Vec3f AimingDirection = Vec3f(1.0f, 0.0f, 0.0f);
    float Speed = 0.0f;
    Quaternion Rotation;

    bool Started = false;
};

