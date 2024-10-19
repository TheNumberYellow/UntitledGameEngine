#pragma once

#include "Graphics/Material.h"
#include "Math/Transform.h"
#include "Math/Vector.h"

#include "Math/Geometry.h"

class Scene;
class ISelectedObject;

class IEditorClickable
{
public:

    virtual RayCastHit ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject) = 0;
};

class ISelectedObject
{
public:

    // Any editor specific drawing that needs to be done for this selected object
    virtual void Draw() {};

    // Any per-frame update that needs to be done while this object is selected
    virtual void Update() {};

    // Fill the UI panel with object-specific data, returns true if data was changed
    virtual bool DrawInspectorPanel() = 0;

    virtual Transform* GetTransform() = 0;
    virtual void DeleteObject() = 0;

    // Drag'n'drop functionalities
    virtual void ApplyMaterial(Material& inMaterial) {}

    void SetScene(Scene* inScene) { ScenePtr = inScene; }

    virtual bool operator==(const ISelectedObject& Other) const
    {
        return typeid(*this) == typeid(Other) && IsEqual(Other);
    }

protected:

    virtual bool IsEqual(const ISelectedObject& Other) const = 0;

    Scene* ScenePtr;

    // Just some handy consts classes using this interface can use to have consistent colours when drawing a selection box or inspector widgets
    const Vec3f c_SelectedBoxColour = Vec3f(0.f / 255.f, 255.f / 255.f, 255.f / 255.f);
    const Vec3f c_InspectorColour = Vec3f(175.f / 255.f, 225.f / 255.f, 175.f / 255.f);
};
