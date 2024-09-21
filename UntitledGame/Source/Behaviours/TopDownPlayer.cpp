#include "TopDownPlayer.h"

#include "Ghost.h"
#include "TopDownBullet.h"

REGISTER_BEHAVIOUR(TopDownPlayer);

void TopDownPlayer::Update(Scene* Scene, double DeltaTime)
{
    if (Health <= 0)
    {
        Scene->DeleteModel(m_Model);
        return;
    }

    InputModule& Input = *InputModule::Get();
    GraphicsModule& Graphics = *GraphicsModule::Get();
    CollisionModule& Collisions = *CollisionModule::Get();
    //UIModule& UI = *Modules.GetUI();

    //TimeAlive += DeltaTime;
    //std::string TimeAliveString = "Time Alive: " + std::to_string(TimeAlive);
    //UI.Text(TimeAliveString, Vec2f(0.0f, 50.0f));
    //UI.TextButton(TimeAliveString, Rect(Vec2f(50.0f, 50.0f), Vec2f(100.0f, 100.0f)), 2.0f);
    if (!Started)
    {
        AssetRegistry* Registry = AssetRegistry::Get();
        GhostModelPrototype = Graphics.CreateModel(TexturedMesh(*Registry->LoadStaticMesh("Assets/models/Ghost.obj"), Graphics.CreateMaterial(*Registry->LoadTexture("Assets/textures/Ghost.png"))));
        BulletModelPrototype = Graphics.CreateModel(TexturedMesh(*Registry->LoadStaticMesh("Assets/models/Buckyball.obj"), Graphics.CreateMaterial(*Registry->LoadTexture("Assets/textures/transpink.png"))));

        Started = true;
    }

    GhostSpawnTimer -= DeltaTime;

    while (GhostSpawnTimer <= 0.0f)
    {
        GhostSpawnTimer += GhostSpawnPeriod;

        Model* NewGhostModel = new Model(Graphics.CloneModel(GhostModelPrototype));

        float Angle = Math::RandomFloat(0.0f, Deg2Rad(360.0f));
        Vec2f NewGhostPos2D = Vec2f(sin(Angle) * 60.0f, cos(Angle) * 60.0f);

        NewGhostModel->GetTransform().SetPosition(Vec3f(NewGhostPos2D.x, NewGhostPos2D.y, 3.0f));

        NewGhostModel->GetTransform().SetScale(Vec3f(0.5f, 0.5f, 0.5f));

        Model* What = Scene->AddModel(NewGhostModel);

        Ghost* GhostBehaviour = static_cast<Ghost*>(BehaviourRegistry::Get()->AttachNewBehaviour("Ghost", What));

        GhostBehaviour->GhostSpeed = Math::RandomFloat(3.0f, 6.0f);
        GhostBehaviour->SetTarget(m_Model);

        What->m_Name = "Ghost";

        GhostCount++;

        //std::string GhostText = "Ghosts: " + std::to_string(GhostCount);

        //Engine::DEBUGPrint(GhostText);
    }

    Vec3f InputDir = Vec3f(1.0f, 0.0f, 0.0f);
    Vec3f FacingDir;

    bool AnyInput = false;
    bool AnyFacingInput = false;

    GamepadState& GamepadState = Input.GetGamepadState();

    if (GamepadState.IsEnabled())
    {
        Vec2f ControllerAxis = Input.GetGamepadState().GetLeftStickAxis();
        Vec2f ControllerAxisRight = Input.GetGamepadState().GetRightStickAxis();

        InputDir = Vec3f(ControllerAxis.x, ControllerAxis.y, 0.0f);
        AnyInput = (ControllerAxis.x != 0.0f && ControllerAxis.y != 0.0f);

        FacingDir = Vec3f(ControllerAxisRight.x, ControllerAxisRight.y, 0.0f);
        AnyFacingInput = (ControllerAxisRight.x != 0.0f && ControllerAxisRight.y != 0.0f);
    }
    else
    {
        if (Input.GetKeyState(Key::W).pressed)
        {
            InputDir.y += 1.0f;
            AnyInput = true;
        }
        if (Input.GetKeyState(Key::S).pressed)
        {
            InputDir.y -= 1.0f;
            AnyInput = true;
        }
        if (Input.GetKeyState(Key::A).pressed)
        {
            InputDir.x -= 1.0f;
            AnyInput = true;
        }
        if (Input.GetKeyState(Key::D).pressed)
        {
            InputDir.x += 1.0f;
            AnyInput = true;
        }
    }

    if (Input.GetGamepadState().IsEnabled())
    {
        float LeftTrigger = Input.GetGamepadState().GetLeftTriggerAnalog();
        float RightTrigger = Input.GetGamepadState().GetRightTriggerAnalog();

        float Delta = RightTrigger - LeftTrigger;

        CamHeight -= 0.1f * Delta;

        //Vec2f RightStick = Input.GetGamepadState().GetRightStickAxis();
        //CamHeight -= 0.1f * RightStick.y;
    }
    else
    {
        int DeltaWheel = Input.GetMouseState().GetDeltaMouseWheel();
        CamHeight += -DeltaWheel;
    }

    CamHeight = Math::clamp(CamHeight, 1.0f, 20.0f);

    if (AnyInput && Math::magnitude(InputDir) > 0.01f)
    {
        float Mult = 1.0f;
        if (Input.GetKeyState(Key::Shift))
        {
            Mult = 2.0f;
        }
        if (!Input.GetGamepadState().IsEnabled())
        {
            InputDir = Math::normalize(InputDir);
        }
        Vec3f Pos = m_Model->GetTransform().GetPosition();
        Vec3f Movement = (InputDir * Speed * Mult) * DeltaTime;

        Vec3f Skin = Vec3f(0.0f, 0.0f, 0.01f);

        std::vector<Model*> IgnoredModels;
        IgnoredModels.push_back(m_Model);
        SceneRayCastHit MoveHit = Scene->RayCast(Ray(Pos + Skin, Math::normalize(Movement)), IgnoredModels);

        if (!MoveHit.rayCastHit.hit || MoveHit.rayCastHit.hitDistance > Math::magnitude(Movement))
        {
            m_Model->GetTransform().Move(Movement);
        }
        else
        {
            Vec3f HitPos = MoveHit.rayCastHit.hitPoint + (0.01f * MoveHit.rayCastHit.hitNormal);
            m_Model->GetTransform().SetPosition(HitPos - Skin);
        }
    }

    if (AnyFacingInput || AnyInput && Math::magnitude(InputDir) > 0.01f)
    {
        if (AnyFacingInput)
        {
            InputDir = FacingDir;
        }

        LastDir = Math::normalize(InputDir);

        Vec2f InputDir2D = Vec2f(InputDir.x, InputDir.y);
        Vec2f Left2D = Vec2f(-1.0f, 0.0f);

        float dot = Math::dot(InputDir2D, Left2D);
        float det = (InputDir2D.x * Left2D.y) - (InputDir2D.y * Left2D.x);

        float angle = atan2(det, dot);

        Quaternion r = Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -angle);
        m_Model->GetTransform().SetRotation(r);
        
    }

    BulletShootTimer -= DeltaTime;
    if (BulletShootTimer < 0.0f)
    {
        BulletShootTimer = 0.0f;

        bool Shooting = false;
        if (Input.GetGamepadState().IsEnabled())
        {
            if (Input.GetGamepadState().GetButtonState(Button::Shoulder_Right))
            {
                Shooting = true;
            }
        }
        else
        {
            if (Input.GetKeyState(Key::Space))
            {
                Shooting = true;
            }
        }
        if (Shooting)
        {
            BulletShootTimer += BulletShootPeriod;

            Model* NewBulletModel = new Model(Graphics.CloneModel(BulletModelPrototype));

            Vec3f MyPos = m_Model->GetTransform().GetPosition();
            MyPos.z += 2.5f;

            NewBulletModel->GetTransform().SetPosition(MyPos + (LastDir * 1.5f));
            NewBulletModel->GetTransform().SetScale(Vec3f(0.5f, 0.5f, 0.5f));

            Model* What = Scene->AddModel(NewBulletModel);

            TopDownBullet* BulletBehaviour = static_cast<TopDownBullet*>(BehaviourRegistry::Get()->AttachNewBehaviour("TopDownBullet", What));

            BulletBehaviour->Direction = LastDir;
        }

    }

    Camera* PlayerCam = Scene->GetCamera();

    Vec3f CamPos = m_Model->GetTransform().GetPosition() + Vec3f(0.0f, -3.0f, CamHeight);

    Vec3f CamToPlayer = m_Model->GetTransform().GetPosition() - CamPos;

    PlayerCam->SetPosition(CamPos);
    PlayerCam->SetDirection(Math::normalize(CamToPlayer));

}

void TopDownPlayer::Hurt()
{
    Health--;


}
