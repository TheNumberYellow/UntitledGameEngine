#include "Entity.h"

#include "Components/Component.h"
#include "Modules/UIModule.h"

void Entity::OnCreate()
{
}

void Entity::Update(float dt)
{

}

void Entity::OnDestroy()
{
}

void Entity::DrawEditorInspector()
{
    UIModule* UI = UIModule::Get();

    UI->TextButton(m_Name, Vec2f(120.0f, 40.0f), 8.0f);

    for (auto* component : m_Components)
    {
        component->DrawEditorInspector();
    }
}

//void Entity::SetScene(Scene* scene)
//{
//    m_Scene = scene;
//}
//
//Scene* Entity::GetScene()
//{
//    return m_Scene;
//}
