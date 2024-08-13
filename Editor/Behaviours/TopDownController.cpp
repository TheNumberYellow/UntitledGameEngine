#include "TopDownController.h"

REGISTER_BEHAVIOUR(TopDownController);

TopDownController::TopDownController()
{
}

void TopDownController::Initialize(Scene* Scene)
{
}

void TopDownController::Update(Scene* Scene, double DeltaTime)
{
    InputModule* Inputs = InputModule::Get();
    UIModule* UI = UIModule::Get();

    Grounded = false;
    Sliding = false;

    // Apply input force

    Vec3f InputForce = Vec3f(0.0f, 0.0f, 0.0f);



    // Apply gravity
    Velocity.z -= 9.8f * DeltaTime;

    m_Model->GetTransform().Move(Velocity * DeltaTime);

    Sphere MySphere;
    MySphere.position = m_Model->GetTransform().GetPosition();
    MySphere.radius = 1.0f;

    Intersection SceneIntersection = Scene->SphereIntersect(MySphere, { m_Model });

    if (SceneIntersection.hit)
    {
        // Check penetration normal closeness to up vector
        float UpCloseness = Math::dot(-SceneIntersection.penetrationNormal, Vec3f(0.0f, 0.0f, 1.0f));
        if (UpCloseness > 0.8)
        {
            Grounded = true;
            m_Model->GetTransform().Move((SceneIntersection.penetrationNormal * 0.001f) + (SceneIntersection.penetrationNormal * -SceneIntersection.penetrationDepth));
            Velocity.z = 0.0f;
        }
        else
        {
            Sliding = true;
            m_Model->GetTransform().Move((SceneIntersection.penetrationNormal * 0.001f) + (SceneIntersection.penetrationNormal * -SceneIntersection.penetrationDepth));
            Velocity = Velocity - (2.f * (Math::dot(Velocity, SceneIntersection.penetrationNormal)) * SceneIntersection.penetrationNormal) * 0.9f;
        }
    }

    if (Grounded && Inputs->GetKeyState(Key::Space).justPressed)
    {
        Velocity.z = 10.f;
    }

    if (!Sliding)
    {
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
    }

    if (!InputForce.IsNearlyZero())
    {
        InputForce = Math::normalize(InputForce);

        InputForce *= 8.0f;

        Velocity.x = InputForce.x;
        Velocity.y = InputForce.y;
    }
    else
    {
        if (Grounded)
        {
            Velocity.x = 0.0f;
            Velocity.y = 0.0f;
        }
    }


    Scene->GetCamera()->SetPosition(m_Model->GetTransform().GetPosition() + Vec3f(0.0f, -5.0f, 7.5f));
    Scene->GetCamera()->SetDirection(Math::normalize(m_Model->GetTransform().GetPosition() - Scene->GetCamera()->GetPosition()));

    UI->TextButton("Yesyseysey", Vec2f(40.0f, 40.0f), 12.f);
}
