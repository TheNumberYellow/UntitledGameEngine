#pragma once

#include "Math/Transform.h"
#include "Scene.h"

class Component;

class Entity
{
public:

    virtual void OnCreate();

    virtual void Update(float dt);

    virtual void OnDestroy();

    virtual void DrawEditorInspector();

private:
    std::string m_Name;

    Transform m_Transform;

    std::vector<Component*> m_Components;

//    static void SetScene(Scene* scene);
//
//
//protected:
//
//    static Scene* GetScene();
//
//private:
//
//    static Scene* m_Scene;
};

