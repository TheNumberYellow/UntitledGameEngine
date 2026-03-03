#include "Lantern.h"

REGISTER_BEHAVIOUR(Lantern);

void Lantern::Initialize(Scene* Scene)
{
    PointLight Light;
    Light.position = m_Model->GetTransform().GetPosition() + lightOffset;
    Light.colour = Vec3f(255.f / 255.f, 191.f / 255.f, 0.f / 255.f);
    Light.intensity = 2.0f;
    LanternLight = Scene->AddPointLight(Light);

}

void Lantern::Update(Scene* Scene, double DeltaTime)
{
    // Use sin and cos functions to create a flickering effect for the lantern light

    Vec3f LanternPos = m_Model->GetTransform().GetPosition() + lightOffset;
    LanternPos += Vec3f(sin(StrobeTimer * 5.0f) * 0.01f, cos(StrobeTimer * 7.0f) * 0.01f, sin(StrobeTimer * 3.0f) * 0.01f);
    LanternLight->position = LanternPos;

    StrobeTimer += (float)DeltaTime;
    LanternLight->intensity = Math::PerlinNoise1D(StrobeTimer * 2.0f) * 0.5f + 1.5f;
}

inline void Lantern::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    UI->FloatSlider("Offset X", Vec2f(400.0f, 20.0f), lightOffset.x, -5.0f, 5.0f);
    UI->FloatSlider("Offset Y", Vec2f(400.0f, 20.0f), lightOffset.y, -5.0f, 5.0f);
    UI->FloatSlider("Offset Z", Vec2f(400.0f, 20.0f), lightOffset.z, -5.0f, 5.0f);

    GraphicsModule* Graphics = GraphicsModule::Get();
    Graphics->DebugDrawPoint(m_Model->GetTransform().GetPosition() + lightOffset, Vec3f(1.0f, 0.5f, 0.0f));
}
