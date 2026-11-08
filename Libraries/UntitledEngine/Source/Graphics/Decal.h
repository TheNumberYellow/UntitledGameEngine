#pragma once

#include "Interfaces/EditorClickable_i.h"
#include "Math/Geometry.h"
#include "Scene/SceneObject.h"

class Decal;
class Model;

class SelectedDecal : public ISelectedObject
{
public:
    SelectedDecal(std::shared_ptr<Decal> inDecalPtr);

    virtual void Draw() override;
    virtual void Update() override;
    virtual bool DrawInspectorPanel() override;
    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:
    virtual bool IsEqual(const ISelectedObject& Other) const override;
    
    std::shared_ptr<Decal> DecalPtr;
    Transform Trans;
};

struct Decal 
    : public IEditorClickable
    , public SceneObject
{
    Decal();

    std::vector<Plane> GetProjectionCubePlanes();

    Vec3f GetLocalDecalSpacePosition(Vec3f worldPos);

    Model* m_RepModel = nullptr;

    Mat4x4f orientation;


    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
};