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
    Quaternion LastRot;

    Vec3f AngVel = Vec3f(0.0f, 0.0f, 0.0f);

    float JumpSpeed = 30.0f;

    float ImpulseForce = 30.0f;
    float Restitution = 0.5f;

    float CamXAxis = 0.0f;
    float CamYAxis = 0.0f;

    float DefaultCamDistance = 5.0f;
    float CamDistance;

    Vec3f CamFacingDir = Vec3f(1.0f, 0.0f, 0.0f);

    PointLight* MyLight = nullptr;

    std::deque<Vec3f> LastXPositions;
    int QueueSize = 100000;
};

