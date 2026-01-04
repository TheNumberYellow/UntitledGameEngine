#include "DumbSphere.h"

REGISTER_BEHAVIOUR(DumbSphere);

DumbSphere::DumbSphere()
{
}

void DumbSphere::Update(Scene* Scene, double DeltaTime)
{
    DeltaTime = Math::Min(DeltaTime, 0.015);

    //InputModule* Inputs = InputModule::Get();

    Velocity.z -= 90.0f * (float)DeltaTime;


    m_Model->GetTransform().Move(Velocity * (float)DeltaTime);

    Sphere MySphere;
    MySphere.position = m_Model->GetTransform().GetPosition();
    MySphere.radius = 1.0f;

    Intersection SceneIntersection = Scene->SphereIntersect(MySphere, { m_Model });

    if (SceneIntersection.hit)
    {
        m_Model->GetTransform().Move((SceneIntersection.penetrationNormal * 0.001f) + (SceneIntersection.penetrationNormal * -SceneIntersection.penetrationDepth));
        Velocity = Velocity - (2.f * (Math::dot(Velocity, SceneIntersection.penetrationNormal)) * SceneIntersection.penetrationNormal) * 0.85f;

        Vec3f n = -SceneIntersection.penetrationNormal;
        //Vec3f t = Velocity;
        Vec3f t = Math::ProjectVecOnPlane(Velocity, Plane(MySphere.position, n));
        Vec3f b = Math::cross(n, t);


        if (!b.IsNearlyZero())
        {
            b = b.GetNormalized();
            n = n.GetNormalized();
            //GraphicsModule::Get()->DebugDrawArrow(MySphere.position, MySphere.position + b * 2.0f);

            LastRot = Quaternion(b, t.Magnitude() * DeltaTime);
            m_Model->GetTransform().Rotate(LastRot);
            //m_Model->GetTransform().SetRotation(m_Model->GetTransform().GetRotation().GetNormalized());

        }
    }
    else
    {
        m_Model->GetTransform().Rotate(LastRot);
    }


    MyLight->position = m_Model->GetTransform().GetPosition();
}

void DumbSphere::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    UI->TextButton("Sphere Controller Settings", Vec2f(300.0f, 20.0f), 2.0f);
    UI->NewLine();

    UI->FloatSlider("Impulse", Vec2f(300.0f, 20.0f), ImpulseForce, 0.0f, 100.0f);
}

void DumbSphere::Initialize(Scene* Scene)
{
    InputState = &InputModule::Get()->m_LocalSystemInputState;

    PointLight NewLight;

    NewLight.position = m_Model->GetTransform().GetPosition();
    //NewLight.colour = MakeColour(Math::RandomInt(0, 255), Math::RandomInt(0, 255), Math::RandomInt(0, 255));
    NewLight.colour = MakeColour(155, 255, 155);

    NewLight.intensity = 0.5f;

    MyLight = Scene->AddPointLight(NewLight);
}
