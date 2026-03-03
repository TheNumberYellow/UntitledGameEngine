#include "SpotLight.h"

#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include "Scene.h"

SelectedSpotLight::SelectedSpotLight(SpotLight* InSpotLight)
{
    SpotLightPtr = InSpotLight;

    Trans.SetPosition(SpotLightPtr->position);
    Vec3f up = Vec3f(0.0f, 0.0f, 1.0f);
    Quaternion quat = Math::VecDiffToQuat(InSpotLight->direction, up);

    Trans.SetRotation(quat);
}

void SelectedSpotLight::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    AABB LightAABB = AABB(SpotLightPtr->position - Vec3f(0.35f, 0.35f, 0.35f), SpotLightPtr->position + Vec3f(0.35f, 0.35f, 0.35f));

    Graphics->DebugDrawAABB(LightAABB, c_SelectedBoxColour);
}

void SelectedSpotLight::Update()
{
    SpotLightPtr->position = Trans.GetPosition();
    SpotLightPtr->direction = Vec3f::Up() * Trans.GetRotation();
}

bool SelectedSpotLight::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    static std::string ColourString = "Colour";
    UI->TextEntry("Colour", ColourString, Vec2f(250.0f, 20.0f), c_InspectorColour);

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), SpotLightPtr->colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), SpotLightPtr->colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), SpotLightPtr->colour.b);

    UI->NewLine();

    UI->FloatDragger("Intensity", Vec2f(400.0f, 20.0f), SpotLightPtr->intensity, 0.1f, 0.0f);

    UI->FloatDragger("Constant Attenuation", Vec2f(400.0f, 20.0f), SpotLightPtr->constantAttenuation, 0.01f, 0.0f);
    UI->FloatDragger("Linear Attenuation", Vec2f(400.0f, 20.0f), SpotLightPtr->linearAttenuation, 0.01f, 0.0f);
    UI->FloatDragger("Quadratic Attenuation", Vec2f(400.0f, 20.0f), SpotLightPtr->quadraticAttenuation, 0.01f, 0.0f);

    UI->NewLine();

    UI->FloatDragger("Inner Angle", Vec2f(400.0f, 20.0f), SpotLightPtr->innerAngle, 0.1f, 0.0f, SpotLightPtr->outerAngle);
    UI->FloatDragger("Outer Angle", Vec2f(400.0f, 20.0f), SpotLightPtr->outerAngle, 0.1f, SpotLightPtr->innerAngle, 90.0f);

    return false;
}

Transform* SelectedSpotLight::GetTransform()
{
    return &Trans;
}

void SelectedSpotLight::DeleteObject()
{
    ScenePtr->DeleteSpotLight(SpotLightPtr);
}

bool SelectedSpotLight::IsEqual(const ISelectedObject& Other) const
{
    return SpotLightPtr == static_cast<const SelectedSpotLight&>(Other).SpotLightPtr;
}

RayCastHit SpotLight::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    CollisionModule* collisions = CollisionModule::Get();

    AABB LightAABB = AABB(position - Vec3f(0.35f, 0.35f, 0.35f), position + Vec3f(0.35f, 0.35f, 0.35f));

    RayCastHit result = collisions->RayCast(mouseRay, LightAABB);

    if (result.hit)
    {
        outSelectedObject = new SelectedSpotLight(this);
    }

    return result;
}
