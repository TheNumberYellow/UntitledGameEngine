#pragma once

#include "Interfaces/EditorClickable_i.h"
#include "Math/Geometry.h"

class Decal;

class SelectedDecal : public ISelectedObject
{
public:
    SelectedDecal(Decal* inDecalPtr);

    virtual void Draw() override;
    virtual void Update() override;
    virtual bool DrawInspectorPanel() override;
    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:
    virtual bool IsEqual(const ISelectedObject& Other) const override;
    
    Decal* DecalPtr;
    Transform Trans;
};

struct Decal : public IEditorClickable
{
    Decal();

    std::vector<Plane> GetProjectionCubePlanes();

    Mat4x4f orientation;


    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) override;
};