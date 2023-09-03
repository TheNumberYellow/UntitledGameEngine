#include "RCCar.h"

REGISTER_BEHAVIOUR(RCCar);

void RCCar::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    if (!Started)
    {
        Quaternion AimRot = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

        FrontLeftTire =     m_Model->GetTransform().GetPosition() + (Vec3f(1.58f, 1.07f, 0.0f) * AimRot);
        FrontRightTire =    m_Model->GetTransform().GetPosition() + (Vec3f(1.58f, -1.07f, 0.0f) * AimRot);
        BackLeftTire =      m_Model->GetTransform().GetPosition() + (Vec3f(-1.15f, 1.07f, 0.0f) * AimRot);
        BackRightTire =     m_Model->GetTransform().GetPosition() + (Vec3f(-1.15f, -1.07f, 0.0f) * AimRot);
    }

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

    Quaternion AimRot = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

    //Vec3f FrontLeftTirePos =    m_Model->GetTransform().GetPosition() + (Vec3f(1.58f, 1.07f, 0.0f) * AimRot);
    //Vec3f FrontRightTirePos =   m_Model->GetTransform().GetPosition() + (Vec3f(1.58f, -1.07f, 0.0f) * AimRot);
    //Vec3f BackLeftTirePos =     m_Model->GetTransform().GetPosition() + (Vec3f(-1.15f, 1.07f, 0.0f) * AimRot);
    //Vec3f BackRightTirePos =    m_Model->GetTransform().GetPosition() + (Vec3f(-1.15f, -1.07f, 0.0f) * AimRot);

    Graphics->DebugDrawLine(FrontLeftTire, FrontLeftTire + Vec3f(0.0f, 0.0f, 5.0f));
    Graphics->DebugDrawLine(FrontRightTire, FrontRightTire + Vec3f(0.0f, 0.0f, 5.0f));
    Graphics->DebugDrawLine(BackLeftTire, BackLeftTire + Vec3f(0.0f, 0.0f, 5.0f));
    Graphics->DebugDrawLine(BackRightTire, BackRightTire + Vec3f(0.0f, 0.0f, 5.0f));


    std::vector<Model*> IgnoredModels;
    IgnoredModels.push_back(m_Model);

    SceneRayCastHit FrontLeftTireHit = Scene->RayCast(Ray(FrontLeftTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    SceneRayCastHit FrontRightTireHit = Scene->RayCast(Ray(FrontRightTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    SceneRayCastHit BackLeftTireHit = Scene->RayCast(Ray(BackLeftTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    SceneRayCastHit BackRightTireHit = Scene->RayCast(Ray(BackRightTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);

    Vec3f NewFrontLeftTirePos = FrontLeftTire;
    Vec3f NewFrontRightTirePos = FrontRightTire;
    Vec3f NewBackLeftTirePos = BackLeftTire;
    Vec3f NewBackRightTirePos = BackRightTire;

    if (FrontLeftTireHit.rayCastHit.hit)
    {
        NewFrontLeftTirePos = FrontLeftTireHit.rayCastHit.hitPoint;
    }

    if (FrontRightTireHit.rayCastHit.hit)
    {
        NewFrontRightTirePos = FrontRightTireHit.rayCastHit.hitPoint;
    }

    if (BackLeftTireHit.rayCastHit.hit)
    {
        NewBackLeftTirePos = BackLeftTireHit.rayCastHit.hitPoint;
    }

    if (BackRightTireHit.rayCastHit.hit)
    {
        NewBackRightTirePos = BackRightTireHit.rayCastHit.hitPoint;
    }

    Graphics->DebugDrawPoint(NewFrontLeftTirePos, Vec3f(1.0f, 0.2f, 0.2f));
    Graphics->DebugDrawPoint(NewFrontRightTirePos, Vec3f(1.0f, 0.2f, 0.2f));
    Graphics->DebugDrawPoint(NewBackLeftTirePos, Vec3f(1.0f, 0.2f, 0.2f));
    Graphics->DebugDrawPoint(NewBackRightTirePos, Vec3f(1.0f, 0.2f, 0.2f));

    Vec3f PlaneNormal1 = Math::cross((NewFrontLeftTirePos - NewFrontRightTirePos), (NewFrontLeftTirePos - NewBackLeftTirePos));
    Vec3f PlaneNormal2 = -Math::cross((NewFrontRightTirePos - NewFrontLeftTirePos), (NewFrontRightTirePos - NewBackRightTirePos));

    Vec3f PlaneNormal = (Math::normalize(PlaneNormal1) + Math::normalize(PlaneNormal2)) / 2.0f;

    PlaneNormal = Math::normalize(-PlaneNormal);

    Graphics->DebugDrawLine(m_Model->GetTransform().GetPosition(), m_Model->GetTransform().GetPosition() + PlaneNormal, Vec3f(0.0f, 1.0f, 0.0f));

    Quaternion q = Math::VecDiffToQuat(PlaneNormal, Vec3f(0.0f, 0.0f, 1.0f));

    Quaternion r = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

    m_Model->GetTransform().SetRotation(q * r);

    Plane p = Plane(NewFrontLeftTirePos, PlaneNormal);

    Vec3f NewCarPos = Math::ClosestPointOnPlaneToPoint(p, m_Model->GetTransform().GetPosition());
    m_Model->GetTransform().SetPosition(NewCarPos);

    //SceneRayCastHit Hit = Scene->RayCast(Ray(m_Model->GetTransform().GetPosition() + Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    //if (Hit.rayCastHit.hit)
    //{
    //    Vec3f newPos = Hit.rayCastHit.hitPoint;
    //    m_Model->GetTransform().SetPosition(newPos);
    //    
    //    Quaternion q = Math::VecDiffToQuat(Hit.rayCastHit.hitNormal, Vec3f(0.0f, 0.0f, 1.0f));

    //    Quaternion r = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

    //    m_Model->GetTransform().SetRotation(q * r);

    //    std::string ModelName = Hit.hitModel->m_Name;

    //    Engine::DEBUGPrint(ModelName);
    //}
    //else
    //{
    //    Quaternion r = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

    //    m_Model->GetTransform().SetRotation(r);
    //}

    Camera* Cam = Scene->GetCamera();
    
    Cam->SetPosition(m_Model->GetTransform().GetPosition() + -AimingDirection * 5.0f + Vec3f(0.0f, 0.0f, 5.0f));

    Vec3f CamToCar = m_Model->GetTransform().GetPosition() - Cam->GetPosition();

    Cam->SetDirection(Math::normalize(CamToCar));

}
