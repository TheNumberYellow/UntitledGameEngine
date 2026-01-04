#include "SphereController.h"

REGISTER_BEHAVIOUR(SphereController);

SphereController::SphereController()
{
}

void SphereController::Update(Scene* Scene, double DeltaTime)
{
    //InputModule::Get()->SetMouseLocked(true);
    //InputModule::Get()->SetMouseCenter(Vec2i(50, 50));


    DeltaTime = Math::Min(DeltaTime, 0.015);





    Vec3f InputForce = Vec3f(0.0f, 0.0f, 0.0f);

    if (InputState->GetKeyState(Key::W))
    {
        InputForce += CamFacingDir;
    }
    if (InputState->GetKeyState(Key::S))
    {
        InputForce -= CamFacingDir;
    }
    if (InputState->GetKeyState(Key::A))
    {
        InputForce -= Math::cross(CamFacingDir, Vec3f(0.0f, 0.0f, 1.0f));
    }
    if (InputState->GetKeyState(Key::D))
    {
        InputForce += Math::cross(CamFacingDir, Vec3f(0.0f, 0.0f, 1.0f));
    }

    if (!InputForce.IsNearlyZero())
    {
        InputForce = Math::normalize(InputForce);

        InputForce *= ImpulseForce;

        Velocity += InputForce * (float)DeltaTime;
    }

    Velocity.z -= 90.0f * (float)DeltaTime;

    if (InputState->GetKeyState(Key::Space).justPressed)
    {
        Vec3f t = Math::ProjectVecOnPlane(Velocity, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
        if (!t.IsNearlyZero())
        {
            //t = t.GetNormalized();
            Vec3f b = Math::cross(Vec3f(0.0f, 0.0f, 1.0f), t);
            
            b = b.GetNormalized();

            LastRot = Quaternion(b, t.Magnitude() * DeltaTime);
        }
        Velocity.z = 30.f;
    }

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


        //Plane hitPlane = Plane(m_Model->GetTransform().GetPosition() + (MySphere.radius * -SceneIntersection.penetrationNormal), -SceneIntersection.penetrationNormal);
        //Vec3f floorNormal = -SceneIntersection.penetrationNormal;
        //
        //Vec3f movementDirection = Math::ProjectVecOnPlane(Velocity, hitPlane);

        //Vec3f rotAxis = Math::cross(movementDirection, floorNormal);


        //AngularVelocity = Quaternion(rotAxis, -0.01f);

        //AngularVelocity = AngularVelocity.GetNormalized();
        //
                        
        //Engine::DEBUGPrint("Ball hit something");
    }
    else
    {
        m_Model->GetTransform().Rotate(LastRot);
    }

    CamDistance = Velocity.Magnitude() * 0.15f;
    if (CamDistance < DefaultCamDistance)
    {
        CamDistance = DefaultCamDistance;
    }

    MyLight->position = m_Model->GetTransform().GetPosition();

    // Begin camera stuff

    Vec3f CamCenterPoint = m_Model->GetTransform().GetPosition();

    Vec2f DeltaMouse = InputState->GetMouseState().GetDeltaMousePos();

    CamXAxis -= DeltaMouse.x * 0.005f;
    CamYAxis -= DeltaMouse.y * 0.005f;

    CamYAxis = Math::ClampRadians(CamYAxis, -M_PI_2 + 0.001f, M_PI_2 - 0.001f);

    Quaternion Rotation = Quaternion::FromEuler(CamYAxis, 0.0f, CamXAxis);
    
    
    Vec3f NegDistance = Vec3f(0.0f, -CamDistance, 0.0f);

    Vec3f NewCamPos;
    // Test cam against level geo
    SceneRayCastHit CamHitTest = Scene->RayCast(Ray(CamCenterPoint, (NegDistance * Rotation).GetNormalized()), { m_Model });
    
    if (CamHitTest.rayCastHit.hit && CamHitTest.rayCastHit.hitDistance < CamDistance)
    {
        NewCamPos = CamHitTest.rayCastHit.hitPoint + (CamHitTest.rayCastHit.hitNormal * 0.01f);
    }
    else
    {
        NewCamPos = (NegDistance * Rotation) + CamCenterPoint;

    }
    Vec3f NewCamDir = (CamCenterPoint - NewCamPos).GetNormalized();

    Scene->GetCamera()->SetPosition(NewCamPos);
    Scene->GetCamera()->SetDirection(NewCamDir);

    CamFacingDir = Math::ProjectVecOnPlane(NewCamDir, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
    CamFacingDir = CamFacingDir.GetNormalized();

    // End camera stuff

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
    Engine::LockCursor();
    Engine::HideCursor();

    InputState = &InputModule::Get()->m_LocalSystemInputState;

    PointLight NewLight;

    NewLight.position = m_Model->GetTransform().GetPosition();
    //NewLight.colour = MakeColour(Math::RandomInt(0, 255), Math::RandomInt(0, 255), Math::RandomInt(0, 255));
    NewLight.colour = MakeColour(155, 255, 155);

    NewLight.intensity = 0.5f;

    MyLight = Scene->AddPointLight(NewLight);

    CamDistance = DefaultCamDistance;
}
