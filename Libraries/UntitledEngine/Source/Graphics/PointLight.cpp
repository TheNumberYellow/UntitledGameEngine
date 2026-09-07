#include "PointLight.h"

#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include "Scene/Scene.h"

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

    float lightRange = PointLightPtr->radius;

    Graphics->DebugDrawSphere(PointLightPtr->position, lightRange, PointLightPtr->colour);

}

void SelectedPointLight::Update()
{
    PointLightPtr->position = Trans.GetPosition();
}

bool SelectedPointLight::DrawInspectorPanel()
{
    return PointLightPtr->DrawInspectorPanel();
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

bool PointLight::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    Vec3f Pos = position;
    Vec3f Col = colour;

    UI->Text("Colour", Col);
    UI->NewLine();

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), colour.b);

    UI->Text("Intensity", intensity * Vec3f(1.0f));
    UI->NewLine();

    UI->FloatDragger("Intensity", Vec2f(400.0f, 20.0f), intensity, 0.1f, 0.0f);

    UI->FloatDragger("Radius", Vec2f(400.0f, 20.0f), radius, 0.1f, 0.0f);

    //UI->FloatDragger("Constant Attenuation", Vec2f(400.0f, 20.0f), constantAttenuation, 0.01f, 0.0f);
    //UI->FloatDragger("Linear Attenuation", Vec2f(400.0f, 20.0f), linearAttenuation, 0.01f, 0.0f);
    //UI->FloatDragger("Quadratic Attenuation", Vec2f(400.0f, 20.0f), quadraticAttenuation, 0.01f, 0.0f);
    UI->CheckBox("CastShadows", castShadows);

    return false;
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