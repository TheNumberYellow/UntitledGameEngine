#include "PointLight.h"

#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include "Scene.h"

SelectedPointLight::SelectedPointLight(PointLight* InPointLight)
{
    PointLightPtr = InPointLight;

    Trans.SetPosition(PointLightPtr->position);
}

void SelectedPointLight::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    AABB LightAABB = AABB(PointLightPtr->position - Vec3f(0.35f, 0.35f, 0.35f), PointLightPtr->position + Vec3f(0.35f, 0.35f, 0.35f));

    Graphics->DebugDrawAABB(LightAABB, c_SelectedBoxColour);
}

void SelectedPointLight::Update()
{
    PointLightPtr->position = Trans.GetPosition();
}

bool SelectedPointLight::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    Vec3f Pos = PointLightPtr->position;
    Vec3f Col = PointLightPtr->colour;

    static std::string ColourString = "Colour";
    static std::string IntensityString = "Intensity";

    UI->TextEntry("Colour", ColourString, Vec2f(250.0f, 20.0f), c_InspectorColour);

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), PointLightPtr->colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), PointLightPtr->colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), PointLightPtr->colour.b);

    UI->TextEntry("Intensity", IntensityString, Vec2f(250.0f, 20.0f), c_InspectorColour);

    UI->FloatSlider("Intensity", Vec2f(400.0f, 20.0f), PointLightPtr->intensity, 0.0f, 10.0f);

    return false;
}

Transform* SelectedPointLight::GetTransform()
{
    return &Trans;
}

void SelectedPointLight::DeleteObject()
{
    ScenePtr->DeletePointLight(PointLightPtr);
}

bool SelectedPointLight::IsEqual(const ISelectedObject& Other) const
{
    return PointLightPtr == static_cast<const SelectedPointLight&>(Other).PointLightPtr;
}

RayCastHit PointLight::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    CollisionModule* collisions = CollisionModule::Get();

    AABB LightAABB = AABB(position - Vec3f(0.35f, 0.35f, 0.35f), position + Vec3f(0.35f, 0.35f, 0.35f));

    RayCastHit result = collisions->RayCast(mouseRay, LightAABB);

    if (result.hit)
    {
        outSelectedObject = new SelectedPointLight(this);
    }

    return result;
}