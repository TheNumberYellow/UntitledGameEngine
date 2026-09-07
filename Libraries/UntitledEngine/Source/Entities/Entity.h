#pragma once

#include "Math/Transform.h"
#include "Scene/Scene.h"

#include <typeindex>

class Component;

class Entity
{
public:

    virtual void OnCreate();

    virtual void Update(float dt);

    virtual void OnDestroy();

    virtual void DrawEditorInspector();

    template<typename T, typename... Args> T* AddComponent(Args&&... args)
    {
        std::type_index typeIndex(typeid(T));
        if (m_Components.find(typeIndex) != m_Components.end())
        {
            // Component of this type already exists, return nullptr or handle as needed
            return nullptr;
        }
        T* newComponent = new T(std::forward<Args>(args)...);
        m_Components[typeIndex] = newComponent;
        return newComponent;
    }

    template<typename T> T* GetComponent()
    {
        auto it = m_Components.find(std::type_index(typeid(T)));
        if (it != m_Components.end())
        {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

private:
    std::string m_Name;

    Transform m_Transform;

    std::unordered_map<std::type_index, Component*> m_Components;
};

