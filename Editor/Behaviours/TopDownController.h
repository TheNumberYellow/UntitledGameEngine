#pragma once

#include "Behaviour\Behaviour.h"

class TopDownController :
    public Behaviour
{
public:

    DEFINE_BEHAVIOUR(TopDownController);

    TopDownController();

    void Initialize(Scene* Scene) override;
    void Update(Scene* Scene, float DeltaTime) override;

};

