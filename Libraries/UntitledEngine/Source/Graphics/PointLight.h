#pragma once

#include "Components/Component.h"
#include "Math/Vector.h"
#include "Interfaces/EditorClickable_i.h"
#include "Scene/SceneObject.h"

class PointLight;

struct PointLightRenderCommand
{
    Vec3f m_Colour;
    Vec3f m_Position;
    float m_Intensity;

    float m_Radius;
    //float m_ConstantAttenuation;
    //float m_LinearAttenuation;
    //float m_QuadraticAttenuation;

    bool m_CastShadows;
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

struct PointLight 
    : public IEditorClickable
    , public SceneObject
{
    PointLight(Scene* inScene)
        : SceneObject(inScene)
    {}

    virtual bool DrawInspectorPanel() override;

    Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
    Colour colour = Colour(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    
    float radius = 5.0f;

    //float constantAttenuation = 1.0f;
    //float linearAttenuation = 0.1f;
    //float quadraticAttenuation = 0.1f;

    bool castShadows = true;

    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
};
