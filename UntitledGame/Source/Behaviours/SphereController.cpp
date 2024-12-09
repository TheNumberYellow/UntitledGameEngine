#include "SphereController.h"

REGISTER_BEHAVIOUR(SphereController);

SphereController::SphereController()
{
}

void SphereController::Update(Scene* Scene, double DeltaTime)
{
    DeltaTime = Math::Min(DeltaTime, 0.015);

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

        InputForce *= ImpulseForce;

        Velocity += InputForce * (float)DeltaTime;
    }

    Velocity.z -= 140.0f * (float)DeltaTime;

    if (Inputs->GetKeyState(Key::Space).justPressed)
    {
        Velocity.z = 50.f;
    }

    m_Model->GetTransform().Move(Velocity * (float)DeltaTime);
    
    Sphere MySphere;
    MySphere.position = m_Model->GetTransform().GetPosition();
    MySphere.radius = 1.0f;

    Intersection SceneIntersection = Scene->SphereIntersect(MySphere, { m_Model });

    if (SceneIntersection.hit)
    {
        m_Model->GetTransform().Move((SceneIntersection.penetrationNormal * 0.001f) + (SceneIntersection.penetrationNormal * -SceneIntersection.penetrationDepth));
        Velocity = Velocity - (2.f * (Math::dot(Velocity, SceneIntersection.penetrationNormal)) * SceneIntersection.penetrationNormal) * 0.9f;
        //Engine::DEBUGPrint("Ball hit something");
    }

    MyLight->position = m_Model->GetTransform().GetPosition();

    Scene->GetCamera()->SetPosition(m_Model->GetTransform().GetPosition() + Vec3f(0.0f, -5.0f, 7.5f));
    Scene->GetCamera()->SetDirection(Math::normalize(m_Model->GetTransform().GetPosition() - Scene->GetCamera()->GetPosition()));
}

void SphereController::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    UI->TextButton("Sphere Controller Settings", Vec2f(300.0f, 20.0f), 2.0f);
    UI->NewLine();

    UI->FloatSlider("Impulse", Vec2f(300.0f, 20.0f), ImpulseForce, 0.0f, 100.0f);
}

void SphereController::Initialize(Scene* Scene)
{
    PointLight NewLight;

    NewLight.position = m_Model->GetTransform().GetPosition();
    //NewLight.colour = MakeColour(Math::RandomInt(0, 255), Math::RandomInt(0, 255), Math::RandomInt(0, 255));
    NewLight.colour = MakeColour(155, 255, 155);

    NewLight.intensity = 0.5f;

    MyLight = Scene->AddPointLight(NewLight);
}
