#pragma once
#include "Behaviour/Behaviour.h"

class Lantern : public Behaviour
{
public:
    DEFINE_BEHAVIOUR(Lantern);

    virtual void Initialize(Scene* Scene) override;
    virtual void Update(Scene* Scene, double DeltaTime) override;
    virtual void DrawInspectorPanel() override;

private:
    PointLight* LanternLight = nullptr;
    Vec3f lightOffset = Vec3f(0.0f, 0.0f, 2.0f);

    float StrobeTimer = 0.0f;
};