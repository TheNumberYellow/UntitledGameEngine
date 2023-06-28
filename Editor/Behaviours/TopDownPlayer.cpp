#include "TopDownPlayer.h"

REGISTER_BEHAVIOUR(TopDownPlayer);

void TopDownPlayer::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    InputModule& Input = *Modules.GetInput();
    GraphicsModule& Graphics = *Modules.GetGraphics();

    Vec3f InputDir;
    bool AnyInput = false;

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

    int DeltaWheel = Input.GetMouseState().GetDeltaMouseWheel();

    CamHeight += -DeltaWheel;

    if (AnyInput && Math::magnitude(InputDir) > 0.01f)
    {
        InputDir = Math::normalize(InputDir);
        m_Model->GetTransform().Move(InputDir * 0.05f);
    
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
