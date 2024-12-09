#pragma once

#include "Behaviour\Behaviour.h"

class SphereController :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(SphereController);

    SphereController();

    void Initialize(Scene* Scene) override;
    void Update(Scene* Scene, double DeltaTime) override;
    virtual void DrawInspectorPanel() override;

private:

    Vec3f Velocity = Vec3f(0.0f, 0.0f, 0.0f);

    float ImpulseForce = 30.0f;

    PointLight* MyLight = nullptr;
};

