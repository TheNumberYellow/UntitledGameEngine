#include "Spin.h"

REGISTER_BEHAVIOUR(Spin);

Spin::Spin()
{
}

void Spin::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    float Speed = 2.0f * 3.1415f;
    m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), Speed * DeltaTime));
}
