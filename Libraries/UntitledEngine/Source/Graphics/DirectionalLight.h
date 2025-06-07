#pragma once

#include "Interfaces/EditorClickable_i.h"

class DirectionalLight;

struct DirectionalLightRenderCommand
{
    Vec3f m_Direction;
    Vec3f m_Colour;
};

class SelectedDirectionalLight : public ISelectedObject
{
public:
    SelectedDirectionalLight(DirectionalLight* InDirectionalLight);

    virtual void Draw() override;
    virtual void Update() override;
    virtual bool DrawInspectorPanel() override;
    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:
    virtual bool IsEqual(const ISelectedObject& Other) const override;

    DirectionalLight* DirLightPtr;
    Transform Trans;
};

struct DirectionalLight : public IEditorClickable
{
    // Position is unused in rendering - only for editor purposes
    Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f direction = Vec3f(0.0f, 0.0f, -1.0f);
    Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f);

    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
};