#include "Spin.h"

REGISTER_BEHAVIOUR(Spin);

Spin::Spin()
{
}

void Spin::Update(Scene* Scene, double DeltaTime)
{
    float Speed = 0.5f * 3.1415f;
    m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), Speed * (float)DeltaTime));
}
