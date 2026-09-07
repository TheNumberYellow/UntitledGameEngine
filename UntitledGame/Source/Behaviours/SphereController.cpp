#include "SphereController.h"

REGISTER_BEHAVIOUR(SphereController);

SphereController::SphereController()
{
}

void SphereController::Update(Scene* Scene, double DeltaTime)
{
    DeltaTime = Math::Min(DeltaTime, 0.015);


    Vec3f InputForceKeys = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f InputForceStick = Vec3f(0.0f, 0.0f, 0.0f);
    bool JumpPressed = false;

    if (InputState->GetGamepadState().IsEnabled())
    {
        GamepadState Gamepad = InputState->GetGamepadState();

        Vec2f Stick = Gamepad.GetLeftStickAxis();

        InputForceStick += CamFacingDir * (Stick.y * abs(Stick.y));
        InputForceStick += Math::cross(CamFacingDir, Vec3f(0.0f, 0.0f, 1.0f)) * (Stick.x * abs(Stick.x));

        JumpPressed |= Gamepad.GetButtonState(Button::Face_South).justPressed;

        if (!InputForceStick.IsNearlyZero())
        {
            InputForceStick *= ImpulseForce;

            //Velocity += InputForceStick * (float)DeltaTime;
        }

        if (Gamepad.GetButtonState(Button::Face_North))
        {
            m_Model->GetTransform().SetPosition(Vec3f(0.0f, 0.0f, 5.0f));
            Velocity = Vec3f(0.0f, 0.0f, 0.0f);
        }
    }
    {
        if (InputState->GetKeyState(Key::W))
        {
            InputForceKeys += CamFacingDir;
        }
        if (InputState->GetKeyState(Key::S))
        {
            InputForceKeys -= CamFacingDir;
        }
        if (InputState->GetKeyState(Key::A))
        {
            InputForceKeys -= Math::cross(CamFacingDir, Vec3f(0.0f, 0.0f, 1.0f));
        }
        if (InputState->GetKeyState(Key::D))
        {
            InputForceKeys += Math::cross(CamFacingDir, Vec3f(0.0f, 0.0f, 1.0f));
        }

        JumpPressed |= InputState->GetKeyState(Key::Space).justPressed;

        if (!InputForceKeys.IsNearlyZero())
        {
            InputForceKeys = Math::normalize(InputForceKeys);

            InputForceKeys *= ImpulseForce;

            //Velocity += InputForceKeys * (float)DeltaTime;
        }

        if (InputState->GetKeyState(Key::R))
        {
            m_Model->GetTransform().SetPosition(Vec3f(0.0f, 0.0f, 5.0f));
            Velocity = Vec3f(0.0f, 0.0f, 0.0f);
        }
    }

    // Faster with both lololol
    Velocity += (InputForceKeys + InputForceStick) * (float)DeltaTime;

    Velocity.z -= 90.0f * (float)DeltaTime;

    if (Grounded && JumpPressed)
    {
        AudioModule::Get()->PlayAudioSource(JumpSound);
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
        // Get velocity in direction of hit surface
        Vec3f surface = -SceneIntersection.penetrationNormal;

        Vec3f velTowardSurface = Math::ProjectVecOnVec(Velocity, surface);
        if (velTowardSurface.Magnitude() > 16.0f)
        {
            Engine::DEBUGPrint("X: " + std::to_string(velTowardSurface.x) + ", Y: " + std::to_string(velTowardSurface.y) + ", Z: " + std::to_string(velTowardSurface.z));
            AudioModule::Get()->PlayAudioSource(LandSound);
        }


        if (Math::dot(SceneIntersection.penetrationNormal, Vec3f(0.0f, 0.0f, -1.0f)) > 0.6f)
        {
            Grounded = true;

            // Apply some friction when grounded based on delta time
            //Velocity = Velocity * (1.0f - (0.2f * (float)DeltaTime));

            //if (Velocity.XYOnly().Magnitude() < 0.1f)
            //{
            //    Velocity.x = 0.0f;
            //    Velocity.y = 0.0f;
            //}
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

    Vec2f CamMovement = Vec2f(0.0f, 0.0f);;

    if (InputState->GetGamepadState().IsEnabled())
    {
        Vec2f StickMovement = InputState->GetGamepadState().GetRightStickAxis();
        
        StickMovement.y = -StickMovement.y;
        
        StickMovement.x *= 0.03f;
        StickMovement.y *= 0.03f;

        CamMovement += StickMovement;
    }
    {
        Vec2f MouseMovement = InputState->GetMouseState().GetDeltaMousePos();

        MouseMovement.x *= 0.005f;
        MouseMovement.y *= 0.005f;
        CamMovement += MouseMovement;
    }

    CamXAxis -= CamMovement.x;
    CamYAxis -= CamMovement.y;

    CamYAxis = Math::ClampRadians(CamYAxis, -M_PI_2 + 0.001f, M_PI_2 - 0.001f);

    Quaternion Rotation = Quaternion::FromEuler(CamYAxis, 0.0f, CamXAxis);
    
    Vec3f NegDistance = Vec3f(0.0f, -CamDistance, 0.0f);

    Vec3f NewCamPos;

    // Test cam against level geo
    SceneRayCastHit CamHitTest = Scene->RayCast(Ray(CamCenterPoint, (NegDistance * Rotation).GetNormalized()), { m_Model });
    
    if (CamHitTest.rayCastHit.hit && CamHitTest.rayCastHit.hitDistance < CamDistance)
    {
        NewCamPos = CamHitTest.rayCastHit.hitPoint + (CamHitTest.rayCastHit.hitNormal * 0.1f);
    }
    else
    {
        NewCamPos = (NegDistance * Rotation) + CamCenterPoint;

        // Also test radius around cam against level geo (to prevent clipping when close to walls)
        Sphere CamSphere;
        CamSphere.position = NewCamPos;
        CamSphere.radius = 0.2f;
        Intersection CamSphereTest = Scene->SphereIntersect(CamSphere, { m_Model });
        if (CamSphereTest.hit)
        {
            NewCamPos = NewCamPos + (CamSphereTest.penetrationNormal * (0.1f - CamSphereTest.penetrationDepth));
        }
    }



    Vec3f NewCamDir = (CamCenterPoint - NewCamPos).GetNormalized();

    GetCamera()->SetPosition(NewCamPos);
    GetCamera()->SetDirection(NewCamDir);
    //Scene->GetCamera()->SetPosition(NewCamPos);
    //Scene->GetCamera()->SetDirection(NewCamDir);

    CamFacingDir = Math::ProjectVecOnPlane(NewCamDir, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
    CamFacingDir = CamFacingDir.GetNormalized();

    // End camera stuff
}

void SphereController::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    UI->Text("Sphere Controller");
    //UI->TextButton("Sphere Controller Settings", Vec2f(300.0f, 20.0f), 2.0f);
    UI->NewLine();

    UI->FloatSlider("Impulse", Vec2f(300.0f, 20.0f), ImpulseForce, 0.0f, 100.0f);

    UI->FloatSlider("Restitution", Vec2f(300.0f, 20.0f), Restitution, 0.0f, 2.0f);

    UI->FloatSlider("Jump Speed", Vec2f(300.0f, 20.0f), JumpSpeed, 0.0f, 200.0f);

    UI->FloatSlider("Light Intensity", Vec2f(300.0f, 20.0f), LightIntensity, 0.0f, 25.0f);

    UI->NewLine();

    UI->CheckBox("Light Enabled", LightEnabled);
}

void SphereController::Initialize(Scene* Scene)
{
    if (IsRunningLocally())
    {
        // Temp: make this default
        InputState = &InputModule::Get()->m_LocalSystemInputState;

        InputState->SetMouseLocked(true);
        Engine::HideCursor();
        
        SetCamera(Scene->GetCamera());
    }

    if (LightEnabled)
    {
        PointLight* NewLight = Scene->AddPointLight();

        NewLight->position = m_Model->GetTransform().GetPosition();
        //NewLight->colour = MakeColour(Math::RandomInt(0, 255), Math::RandomInt(0, 255), Math::RandomInt(0, 255));
        NewLight->colour = MakeColour(255, 255, 155);
        NewLight->intensity = LightIntensity;

        MyLight = NewLight;
    }

    CamDistance = DefaultCamDistance;

    // Load sounds
    AudioModule* Audio = AudioModule::Get();
    JumpSound = Audio->CreateAudioSource(Audio->LoadWaveFile("Assets/sound/Jump.wav"), 0.5f, 1.0f, false);
    LandSound = Audio->CreateAudioSource(Audio->LoadWaveFile("Assets/sound/Land.wav"), 1.0f, 1.0f, false);
}
