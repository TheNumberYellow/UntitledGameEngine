#include "RCCar.h"

REGISTER_BEHAVIOUR(RCCar);

void RCCar::Update(Scene* Scene, float DeltaTime)
{
    if (!Started)
    {
        Quaternion AimRot = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

        FrontLeftTire =     m_Model->GetTransform().GetPosition() + (Vec3f(1.58f, 1.07f, 0.0f) * AimRot);
        FrontRightTire =    m_Model->GetTransform().GetPosition() + (Vec3f(1.58f, -1.07f, 0.0f) * AimRot);
        BackLeftTire =      m_Model->GetTransform().GetPosition() + (Vec3f(-1.15f, 1.07f, 0.0f) * AimRot);
        BackRightTire =     m_Model->GetTransform().GetPosition() + (Vec3f(-1.15f, -1.07f, 0.0f) * AimRot);
    }

    InputModule* Input = InputModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();

    const float RotSpeed = 4.0f;

    if (Input->GetKeyState(Key::Left))
    {
        AimingDirection = Math::rotate(AimingDirection, RotSpeed * DeltaTime, Vec3f(0.0f, 0.0f, 1.0f));
        //m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.02f));
    }
    if (Input->GetKeyState(Key::Right))
    {
        AimingDirection = Math::rotate(AimingDirection, -RotSpeed * DeltaTime, Vec3f(0.0f, 0.0f, 1.0f));
        //m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -0.02f));
    }

    AimingDirection = Math::normalize(AimingDirection);
    const float Speed = 10.0f;

    Vec3f Displacement = Vec3f(0.0f, 0.0f, 0.0f);

    if (Input->GetKeyState(Key::Up))
    {
        Displacement += AimingDirection * Speed * DeltaTime;
    }

    GraphicsModule* Graphics = GraphicsModule::Get();

    m_Model->GetTransform().Move(Displacement);

    Quaternion AimRot = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

    //Graphics->DebugDrawLine(FrontLeftTire, FrontLeftTire + Vec3f(0.0f, 0.0f, 5.0f));
    //Graphics->DebugDrawLine(FrontRightTire, FrontRightTire + Vec3f(0.0f, 0.0f, 5.0f));
    //Graphics->DebugDrawLine(BackLeftTire, BackLeftTire + Vec3f(0.0f, 0.0f, 5.0f));
    //Graphics->DebugDrawLine(BackRightTire, BackRightTire + Vec3f(0.0f, 0.0f, 5.0f));


    std::vector<Model*> IgnoredModels;
    IgnoredModels.push_back(m_Model);

    SceneRayCastHit FrontLeftTireHit = Scene->RayCast(Ray(FrontLeftTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    SceneRayCastHit FrontRightTireHit = Scene->RayCast(Ray(FrontRightTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    SceneRayCastHit BackLeftTireHit = Scene->RayCast(Ray(BackLeftTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    SceneRayCastHit BackRightTireHit = Scene->RayCast(Ray(BackRightTire + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);

    //for (int i = 0; i < 100; i++)
    //{
    //    SceneRayCastHit TestHit = Scene->RayCast(Ray(m_Model->GetTransform().GetPosition() + Vec3f(0.0f, 0.0f, 5.0f), Vec3f(0.0f, 0.0f, -1.0f)), *Collisions, IgnoredModels);
    //    Engine::DEBUGPrint(std::to_string(TestHit.rayCastHit.hitPoint.x));
    //}

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

    //Graphics->DebugDrawPoint(NewFrontLeftTirePos, Vec3f(1.0f, 0.2f, 0.2f));
    //Graphics->DebugDrawPoint(NewFrontRightTirePos, Vec3f(1.0f, 0.2f, 0.2f));
    //Graphics->DebugDrawPoint(NewBackLeftTirePos, Vec3f(1.0f, 0.2f, 0.2f));
    //Graphics->DebugDrawPoint(NewBackRightTirePos, Vec3f(1.0f, 0.2f, 0.2f));

    Vec3f PlaneNormal1 = Math::cross((NewFrontLeftTirePos - NewFrontRightTirePos), (NewFrontLeftTirePos - NewBackLeftTirePos));
    Vec3f PlaneNormal2 = -Math::cross((NewFrontRightTirePos - NewFrontLeftTirePos), (NewFrontRightTirePos - NewBackRightTirePos));

    Vec3f PlaneNormal = (Math::normalize(PlaneNormal1) + Math::normalize(PlaneNormal2)) / 2.0f;

    PlaneNormal = Math::normalize(-PlaneNormal);

    //Graphics->DebugDrawLine(m_Model->GetTransform().GetPosition(), m_Model->GetTransform().GetPosition() + PlaneNormal, Vec3f(0.0f, 1.0f, 0.0f));

    Quaternion q = Math::VecDiffToQuat(PlaneNormal, Vec3f(0.0f, 0.0f, 1.0f));

    Quaternion r = Math::VecDiffToQuat(AimingDirection, Vec3f(1.0f, 0.0f, 0.0f));

    m_Model->GetTransform().SetRotation(q * r);

    Plane p = Plane(NewFrontLeftTirePos, PlaneNormal);

    Vec3f NewCarPos = Math::ClosestPointOnPlaneToPoint(p, m_Model->GetTransform().GetPosition());
    m_Model->GetTransform().SetPosition(NewCarPos);


    Camera* Cam = Scene->GetCamera();
    
    Cam->SetPosition(m_Model->GetTransform().GetPosition() + -AimingDirection * 5.0f + Vec3f(0.0f, 0.0f, 5.0f));

    Vec3f CamToCar = m_Model->GetTransform().GetPosition() - Cam->GetPosition();

    Cam->SetDirection(Math::normalize(CamToCar));

}
