#pragma once

#include "Behaviour\Behaviour.h"

class TopDownController :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(TopDownController);

    TopDownController();

    void Initialize(Scene* Scene) override;
    void Update(Scene* Scene, double DeltaTime) override;


private:

    Vec3f Velocity = Vec3f(0.0f, 0.0f, 0.0f);
    bool Grounded = false;
    bool Sliding = false;
};

