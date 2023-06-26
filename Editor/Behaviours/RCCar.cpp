#include "RCCar.h"

REGISTER_BEHAVIOUR(RCCar);

void RCCar::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    InputModule* Input = Modules.GetInput();
    CollisionModule* Collisions = Modules.GetCollision();

    if (Input->GetKeyState(Key::Left))
    {
        AimingDirection = Math::rotate(AimingDirection, 0.02f, Vec3f(0.0f, 0.0f, 1.0f));
        //m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.02f));
    }
    if (Input->GetKeyState(Key::Right))
    {
        AimingDirection = Math::rotate(AimingDirection, -0.02f, Vec3f(0.0f, 0.0f, 1.0f));
        //m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -0.02f));
    }

    AimingDirection = Math::normalize(AimingDirection);
    const float Speed = 0.1f;

    Vec3f Displacement = Vec3f(0.0f, 0.0f, 0.0f);

    if (Input->GetKeyState(Key::Up))
    {
        Displacement += AimingDirection * Speed;
    }

    GraphicsModule* Graphics = Modules.GetGraphics();

    //Graphics->DebugDrawLine(m_Transform->GetPosition(), m_Transform->GetPosition() + Math::normalize(Displacement));

    //SceneRayCastHit Hit = Scene->RayCast(Ray(m_Transform->GetPosition(), Math::normalize(Displacement)), *Collisions);

    //if (false && Hit.rayCastHit.hit && Hit.rayCastHit.hitDistance <= Math::magnitude(Displacement) + 0.0001f)
    //{
    //    Vec3f newPos = Hit.rayCastHit.hitPoint + (2.0f * Hit.rayCastHit.hitNormal);

    //    m_Transform->SetPosition(newPos);
    //    Engine::DEBUGPrint("HIT SOMETHING");
    //}
    //else
    //{
    m_Model->GetTransform().Move(Displacement);
    //}

    std::vector<Model*> IgnoredModels;
    IgnoredModels.push_back(m_Model);

    SceneRayCastHit Hit = Scene->RayCast(Ray(m_Model->GetTransform().GetPosition() + Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);

    if (Hit.rayCastHit.hit)
    {
        Vec3f newPos = Hit.rayCastHit.hitPoint;
        m_Model->GetTransform().SetPosition(newPos);
        
        Quaternion q = Math::VecDiffToQuat(Hit.rayCastHit.hitNormal, Vec3f(0.0f, 0.0f, 1.0f));

        Quaternion r = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

        m_Model->GetTransform().SetRotation(q * r);

        std::string ModelName = Hit.hitModel->m_Name;

        Engine::DEBUGPrint(ModelName);
    }
    else
    {
        Quaternion r = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

        m_Model->GetTransform().SetRotation(r);
    }

    Camera* Cam = Scene->GetCamera();
    
    //Cam->SetPosition(m_Transform->GetPosition() + Vec3f(0.0f, 3.0f, 10.0f));

    Vec3f CamToCar = m_Model->GetTransform().GetPosition() - Cam->GetPosition();

    Cam->SetDirection(Math::normalize(CamToCar));

}
