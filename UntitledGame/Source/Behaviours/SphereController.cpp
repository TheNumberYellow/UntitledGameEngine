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
    bool JumpPressed = false;

    if (InputState->GetGamepadState().IsEnabled())
    {
        GamepadState Gamepad = InputState->GetGamepadState();

        Vec2f Stick = Gamepad.GetLeftStickAxis();

        InputForce += CamFacingDir * Stick.y;
        InputForce += Math::cross(CamFacingDir, Vec3f(0.0f, 0.0f, 1.0f)) * Stick.x;

        JumpPressed = Gamepad.GetButtonState(Button::Face_South).justPressed;

        if (!InputForce.IsNearlyZero())
        {
            //InputForce = Math::normalize(InputForce);

            InputForce *= ImpulseForce;

            Velocity += InputForce * (float)DeltaTime;
        }
    }
    else
    {
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

        JumpPressed = InputState->GetKeyState(Key::Space).justPressed;

        if (!InputForce.IsNearlyZero())
        {
            InputForce = Math::normalize(InputForce);

            InputForce *= ImpulseForce;

            Velocity += InputForce * (float)DeltaTime;
        }
    }


    Velocity.z -= 90.0f * (float)DeltaTime;

    if (Grounded && JumpPressed)
    {
        Vec3f t = Math::ProjectVecOnPlane(Velocity, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
        if (!t.IsNearlyZero())
        {
            //t = t.GetNormalized();
            Vec3f b = Math::cross(Vec3f(0.0f, 0.0f, 1.0f), t);
            
            b = b.GetNormalized();

            LastRot = Quaternion(b, t.Magnitude() * DeltaTime);
        }

        if (Velocity.z > 0.0f)
        {
            Velocity.z += JumpSpeed;
        }
        else
        {
            Velocity.z = JumpSpeed;
        }
    }

    Grounded = false;

    m_Model->GetTransform().Move(Velocity * (float)DeltaTime);

    Sphere MySphere;
    MySphere.position = m_Model->GetTransform().GetPosition();
    MySphere.radius = 1.0f;

    Intersection SceneIntersection = Scene->SphereIntersect(MySphere, { m_Model });

    if (SceneIntersection.hit)
    {
        if (Math::dot(SceneIntersection.penetrationNormal, Vec3f(0.0f, 0.0f, -1.0f)) > 0.6f)
        {
            Grounded = true;
        }
        m_Model->GetTransform().Move((SceneIntersection.penetrationNormal * 0.001f) + (SceneIntersection.penetrationNormal * -SceneIntersection.penetrationDepth));
        Velocity = Velocity - ((1.f + Restitution) * (Math::dot(Velocity, SceneIntersection.penetrationNormal)) * SceneIntersection.penetrationNormal);

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

    if (TrailEnabled)
    {
        Vec3f pos = m_Model->GetTransform().GetPosition();

        LastXPositions.push_back(pos);

        if (LastXPositions.size() > QueueSize)
        {
            LastXPositions.pop_front();
        }

        for (int i = 0; i < LastXPositions.size() - 1; ++i)
        {
            GraphicsModule::Get()->DebugDrawLine(LastXPositions[i], LastXPositions[i + 1], MakeColour(110, 255, 180));
        }
    }

    CamDistance = Velocity.Magnitude() * 0.15f;
    if (CamDistance < DefaultCamDistance)
    {
        CamDistance = DefaultCamDistance;
    }

    if (LightEnabled)
    {
        MyLight->position = m_Model->GetTransform().GetPosition() + Vec3f(0.0f, 0.0f, 0.0f);
    }

    // Begin camera stuff
    Vec3f CamCenterPoint = m_Model->GetTransform().GetPosition();

    Vec2f DeltaMouse;

    if (InputState->GetGamepadState().IsEnabled())
    {
        DeltaMouse = InputState->GetGamepadState().GetRightStickAxis();
        
        DeltaMouse.y = -DeltaMouse.y;

        //Engine::DEBUGPrint("X: " + std::to_string(DeltaMouse.x) + ", Y: " + std::to_string(DeltaMouse.y));
        
        DeltaMouse.x *= 0.03f;
        DeltaMouse.y *= 0.03f;
    }
    else
    {
        DeltaMouse = InputState->GetMouseState().GetDeltaMousePos();

        DeltaMouse.x *= 0.005f;
        DeltaMouse.y *= 0.005f;
    }

    CamXAxis -= DeltaMouse.x;
    CamYAxis -= DeltaMouse.y;

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

    UI->FloatSlider("Restitution", Vec2f(300.0f, 20.0f), Restitution, 0.0f, 2.0f);

    UI->FloatSlider("Jump Speed", Vec2f(300.0f, 20.0f), JumpSpeed, 0.0f, 200.0f);

    UI->FloatSlider("Light Intensity", Vec2f(300.0f, 20.0f), LightIntensity, 0.0f, 25.0f);

    UI->CheckBox("Light Enabled", LightEnabled);
}

void SphereController::Initialize(Scene* Scene)
{
    Engine::LockCursor();
    Engine::HideCursor();

    InputState = &InputModule::Get()->m_LocalSystemInputState;

    if (LightEnabled)
    {
        PointLight NewLight;

        NewLight.position = m_Model->GetTransform().GetPosition();
        //NewLight.colour = MakeColour(Math::RandomInt(0, 255), Math::RandomInt(0, 255), Math::RandomInt(0, 255));
        NewLight.colour = MakeColour(255, 255, 155);

        NewLight.intensity = LightIntensity;

        MyLight = Scene->AddPointLight(NewLight);
    }

    CamDistance = DefaultCamDistance;
}
