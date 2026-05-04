#pragma once

#include "Behaviour\Behaviour.h"

#include "Modules/AudioModule.h"

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

    float JumpSpeed = 25.0f;

    float ImpulseForce = 30.0f;
    float Restitution = 0.0f;

    float LightIntensity = 4.0f;

    float CamXAxis = 0.0f;
    float CamYAxis = 0.0f;

    float DefaultCamDistance = 5.0f;
    float CamDistance;

    Vec3f CamFacingDir = Vec3f(1.0f, 0.0f, 0.0f);

    PointLight* MyLight = nullptr;

    bool TrailEnabled = false;
    bool LightEnabled = false;
    std::deque<Vec3f> LastXPositions;
    int QueueSize = 100000;

    bool Grounded = false;

    AudioSource JumpSound;
    AudioSource LandSound;
};

