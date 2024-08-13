#include "Wobble.h"

REGISTER_BEHAVIOUR(Wobble);

Wobble::Wobble()
{
}

void Wobble::Update(Scene* Scene, double DeltaTime)
{
    if (WobblingLeft)
    {
        m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 1.0f, 0.0f), 0.025f));
    }
    else
    {
        m_Model->GetTransform().Rotate(Quaternion(Vec3f(0.0f, 1.0f, 0.0f), -0.025f));
    }
    if (Math::RandomFloat(0.0f, 1.0f) < 0.025f)
    {
        WobblingLeft = !WobblingLeft;
    }
}
