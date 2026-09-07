#pragma once

//#include "Scene.h"

class Scene;

class SceneObject
{
public:
    SceneObject(Scene* inScene)
        : ScenePtr(inScene)
    {}

    virtual bool DrawInspectorPanel()
    {
        return false;
    }

protected:
    Scene* ScenePtr = nullptr;
};