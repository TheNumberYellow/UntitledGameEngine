#include "DirectionalLight.h"

#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include "Scene.h"

SelectedDirectionalLight::SelectedDirectionalLight(DirectionalLight* InDirLight)
{
    DirLightPtr = InDirLight;

    Trans.SetPosition(InDirLight->position);
    Vec3f up = Vec3f(0.0f, 0.0f, 1.0f);
    Quaternion quat = Math::VecDiffToQuat(InDirLight->direction, up);

    Trans.SetRotation(quat);
}

void SelectedDirectionalLight::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    AABB LightAABB = AABB(DirLightPtr->position - Vec3f(0.35f, 0.35f, 0.35f), DirLightPtr->position + Vec3f(0.35f, 0.35f, 0.35f));

    Graphics->DebugDrawAABB(LightAABB, c_SelectedBoxColour);
}

void SelectedDirectionalLight::Update()
{
    DirLightPtr->position = Trans.GetPosition();
    DirLightPtr->direction = Vec3f::Up() * Trans.GetRotation();
}

bool SelectedDirectionalLight::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    static std::string ColourString = "Colour";

    UI->TextEntry("Colour", ColourString, Vec2f(250.0f, 20.0f), c_InspectorColour);

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), DirLightPtr->colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), DirLightPtr->colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), DirLightPtr->colour.b);

    return false;
}

Transform* SelectedDirectionalLight::GetTransform()
{
    return &Trans;
}

void SelectedDirectionalLight::DeleteObject()
{
    ScenePtr->DeleteDirectionalLight(DirLightPtr);
}

bool SelectedDirectionalLight::IsEqual(const ISelectedObject& Other) const
{
    return DirLightPtr == static_cast<const SelectedDirectionalLight&>(Other).DirLightPtr;
}

RayCastHit DirectionalLight::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    CollisionModule* collisions = CollisionModule::Get();

    AABB LightAABB = AABB(position - Vec3f(0.35f, 0.35f, 0.35f), position + Vec3f(0.35f, 0.35f, 0.35f));

    RayCastHit result = collisions->RayCast(mouseRay, LightAABB);

    if (result.hit)
    {
        outSelectedObject = new SelectedDirectionalLight(this);
    }

    return result;
}