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

    float lightRange;

    // TODO(fraser): test attenuation ranges
    if (PointLightPtr->quadraticAttenuation > 0.0f)
    {
        lightRange = sqrt(PointLightPtr->intensity / (PointLightPtr->quadraticAttenuation * 0.01f));
    }
    else if (PointLightPtr->linearAttenuation > 0.0f)
    {
        lightRange = PointLightPtr->intensity / (PointLightPtr->linearAttenuation * 0.01f);
    }
    else
    {
        lightRange = 200.0f; // Arbitrary large distance if no attenuation
    }
    Graphics->DebugDrawSphere(PointLightPtr->position, lightRange, PointLightPtr->colour);

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


    UI->Text("Colour", c_InspectorColour);
    UI->NewLine();

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), PointLightPtr->colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), PointLightPtr->colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), PointLightPtr->colour.b);

    UI->Text("Intensity", c_InspectorColour);
    UI->NewLine();

    UI->FloatDragger("Intensity", Vec2f(400.0f, 20.0f), PointLightPtr->intensity, 0.1f, 0.0f);

    UI->FloatDragger("Constant Attenuation", Vec2f(400.0f, 20.0f), PointLightPtr->constantAttenuation, 0.01f, 0.0f);
    UI->FloatDragger("Linear Attenuation", Vec2f(400.0f, 20.0f), PointLightPtr->linearAttenuation, 0.01f, 0.0f);
    UI->FloatDragger("Quadratic Attenuation", Vec2f(400.0f, 20.0f), PointLightPtr->quadraticAttenuation, 0.01f, 0.0f);

    UI->CheckBox("CastShadows", PointLightPtr->castShadows);

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