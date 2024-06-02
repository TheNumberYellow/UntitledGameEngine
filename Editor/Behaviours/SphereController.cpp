#include "SphereController.h"

REGISTER_BEHAVIOUR(SphereController);

SphereController::SphereController()
{
}

void SphereController::Update(Scene* Scene, float DeltaTime)
{
    DeltaTime = Math::Min(DeltaTime, 0.015f);

    InputModule* Inputs = InputModule::Get();

    Vec3f InputForce = Vec3f(0.0f, 0.0f, 0.0f);

    if (Inputs->GetKeyState(Key::W))
    {
        InputForce.y += 1.0f;
    }
    if (Inputs->GetKeyState(Key::S))
    {
        InputForce.y -= 1.0f;
    }
    if (Inputs->GetKeyState(Key::A))
    {
        InputForce.x -= 1.0f;
    }
    if (Inputs->GetKeyState(Key::D))
    {
        InputForce.x += 1.0f;
    }

    if (!InputForce.IsNearlyZero())
    {
        InputForce = Math::normalize(InputForce);

        InputForce *= 30.0f;

        Velocity += InputForce * DeltaTime;
    }

    Velocity.z -= 140.0f * DeltaTime;

    if (Inputs->GetKeyState(Key::Space).justPressed)
    {
        Velocity.z = 50.f;
    }

    m_Model->GetTransform().Move(Velocity * DeltaTime);
    
    Sphere MySphere;
    MySphere.position = m_Model->GetTransform().GetPosition();
    MySphere.radius = 1.0f;

    Intersection SceneIntersection = Scene->SphereIntersect(MySphere, { m_Model });

    if (SceneIntersection.hit)
    {
        m_Model->GetTransform().Move((SceneIntersection.penetrationNormal * 0.001f) + (SceneIntersection.penetrationNormal * -SceneIntersection.penetrationDepth));
        Velocity = Velocity - (2.f * (Math::dot(Velocity, SceneIntersection.penetrationNormal)) * SceneIntersection.penetrationNormal) * 0.9f;
        Engine::DEBUGPrint("Ball hit something");
    }

    Scene->GetCamera()->SetPosition(m_Model->GetTransform().GetPosition() + Vec3f(0.0f, -5.0f, 7.5f));
    Scene->GetCamera()->SetDirection(Math::normalize(m_Model->GetTransform().GetPosition() - Scene->GetCamera()->GetPosition()));
}

void SphereController::Initialize(Scene* Scene)
{
}
