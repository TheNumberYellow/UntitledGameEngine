#include "Wobble.h"

REGISTER_BEHAVIOUR(Wobble);

Wobble::Wobble()
{
}

void Wobble::Update(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    if (WobblingLeft)
    {
        m_Transform->Rotate(Quaternion(Vec3f(0.0f, 1.0f, 0.0f), 0.025f));
    }
    else
    {
        m_Transform->Rotate(Quaternion(Vec3f(0.0f, 1.0f, 0.0f), -0.025f));
    }
    if (Math::RandomFloat(0.0f, 1.0f) < 0.025f)
    {
        WobblingLeft = !WobblingLeft;
    }
}
