#pragma once

//#include "Scene.h"

class Scene;

class SceneObject
{
public:
    virtual bool DrawInspectorPanel()
    {
        return false;
    }

};