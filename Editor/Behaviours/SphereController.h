#pragma once

#include "Behaviour\Behaviour.h"

class SphereController :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(SphereController);

    SphereController();

    void Initialize(Scene* Scene) override;
    void Update(Scene* Scene, float DeltaTime) override;


private:

    Vec3f Velocity = Vec3f(0.0f, 0.0f, 0.0f);

    PointLight* MyLight = nullptr;
};

