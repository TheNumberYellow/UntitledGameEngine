#include "TopDownPlayer.h"

REGISTER_BEHAVIOUR(TopDownPlayer);

void TopDownPlayer::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    InputModule& Input = *Modules.GetInput();
    GraphicsModule& Graphics = *Modules.GetGraphics();

    if (!Started)
    {
        GhostModelPrototype = Graphics.CreateModel(TexturedMesh(Graphics.LoadMesh("models/Ghost.obj"), Graphics.CreateMaterial(Graphics.LoadTexture("textures/Ghost.png"))));

        Started = true;
    }

    GhostSpawnTimer -= DeltaTime;


    while (GhostSpawnTimer <= 0.0f)
    {
        GhostSpawnTimer += GhostSpawnPeriod;

        Model* NewGhostModel = new Model(Graphics.CloneModel(GhostModelPrototype));

        NewGhostModel->GetTransform().SetPosition(Vec3f(Math::RandomFloat(-10.0f, 10.0f), Math::RandomFloat(-10.0f, 10.0f), 3.0f));

        NewGhostModel->GetTransform().SetScale(Vec3f(0.5f, 0.5f, 0.5f));

        Model* What = Scene->AddModel(*NewGhostModel);
        
        BehaviourRegistry::Get()->AttachNewBehaviour("Ghost", What);

        GhostCount++;

        std::string GhostText = "Ghosts: " + std::to_string(GhostCount);

        Engine::DEBUGPrint(GhostText);
    }

    Vec3f InputDir;
    bool AnyInput = false;

    GamepadState& GamepadState = Input.GetGamepadState();

    if (GamepadState.IsEnabled())
    {
        Vec2f ControllerAxis = Input.GetGamepadState().GetLeftStickAxis();

        InputDir = Vec3f(ControllerAxis.x, ControllerAxis.y, 0.0f);
        AnyInput = (ControllerAxis.x != 0.0f && ControllerAxis.y != 0.0f);
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
        m_Model->GetTransform().Move((InputDir * Speed * Mult) * DeltaTime);
    
        Vec2f InputDir2D = Vec2f(InputDir.x, InputDir.y);
        Vec2f Left2D = Vec2f(-1.0f, 0.0f);

        float dot = Math::dot(InputDir2D, Left2D);
        float det = (InputDir2D.x * Left2D.y) - (InputDir2D.y * Left2D.x);

        float angle = atan2(det, dot);

        Quaternion r = Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -angle);
        m_Model->GetTransform().SetRotation(r);
        
    }

    Camera* PlayerCam = Scene->GetCamera();

    Vec3f CamPos = m_Model->GetTransform().GetPosition() + Vec3f(0.0f, -3.0f, CamHeight);

    Vec3f CamToPlayer = m_Model->GetTransform().GetPosition() - CamPos;

    PlayerCam->SetPosition(CamPos);
    PlayerCam->SetDirection(Math::normalize(CamToPlayer));

}
