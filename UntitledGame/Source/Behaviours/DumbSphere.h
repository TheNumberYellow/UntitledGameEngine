#pragma once

#include "Behaviour\Behaviour.h"

class DumbSphere :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(DumbSphere);

    DumbSphere();

    void Initialize(Scene* Scene) override;
    void Update(Scene* Scene, double DeltaTime) override;
    virtual void DrawInspectorPanel() override;

private:

    Vec3f Velocity = Vec3f(0.0f, 0.0f, 0.0f);
    Quaternion LastRot;

    Vec3f AngVel = Vec3f(0.0f, 0.0f, 0.0f);

    float ImpulseForce = 30.0f;

    PointLight* MyLight = nullptr;
};

