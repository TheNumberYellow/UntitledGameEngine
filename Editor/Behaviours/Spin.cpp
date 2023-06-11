#include "Spin.h"

REGISTER_BEHAVIOUR(Spin);

Spin::Spin()
{
}

void Spin::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    m_Transform->Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.025f));
}
