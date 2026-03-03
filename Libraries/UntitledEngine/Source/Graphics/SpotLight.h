#pragma once

#include "Components/Component.h"
#include "Math/Vector.h"
#include "Interfaces/EditorClickable_i.h"

class SpotLight;

struct SpotLightRenderCommand
{
    Vec3f m_Colour;
    Vec3f m_Position;
    Vec3f m_Direction;

    float m_Intensity;

    float m_ConstantAttenuation;
    float m_LinearAttenuation;
    float m_QuadraticAttenuation;

    float m_InnerAngle;
    float m_OuterAngle;
    bool m_CastShadows;
};

class SelectedSpotLight : public ISelectedObject, public Component
{
public:
    SelectedSpotLight(SpotLight* InSpotLight);

    virtual void Draw() override;
    virtual void Update() override;
    virtual bool DrawInspectorPanel() override;
    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:

    virtual bool IsEqual(const ISelectedObject& Other) const override;
    SpotLight* SpotLightPtr;
    Transform Trans;
};

struct SpotLight : public IEditorClickable
{
    Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f direction = Vec3f(0.0f, 0.0f, -1.0f);
    Colour colour = Colour(1.0f, 1.0f, 1.0f);
    
    float intensity = 10.0f;

    float constantAttenuation = 1.0f;
    float linearAttenuation = 0.1f;
    float quadraticAttenuation = 0.1f;
    
    float innerAngle = 15.0f;
    float outerAngle = 30.0f;

    bool castShadows = false;
    
    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
};