#pragma once

#include "Math/Vector.h"
#include "Interfaces/EditorClickable_i.h"

class PointLight;

struct PointLightRenderCommand
{
    Vec3f m_Colour;
    Vec3f m_Position;
    float m_Intensity;
};

class SelectedPointLight : public ISelectedObject
{
public:
    SelectedPointLight(PointLight* InPointLight);

    virtual void Draw() override;
    virtual void Update() override;
    virtual bool DrawInspectorPanel() override;
    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:
    virtual bool IsEqual(const ISelectedObject& Other) const override;

    PointLight* PointLightPtr;
    Transform Trans;
};

struct PointLight : public IEditorClickable
{
    Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
    Colour colour = Colour(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;

    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
};
