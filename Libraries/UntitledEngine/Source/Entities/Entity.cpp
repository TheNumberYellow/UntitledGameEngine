#include "Entity.h"

void Entity::OnCreate()
{
}

void Entity::Update(float dt)
{
    if (m_Scene)
    {

    }
}

void Entity::OnDestroy()
{
}

void Entity::SetScene(Scene* scene)
{
    m_Scene = scene;
}

Scene* Entity::GetScene()
{
    return m_Scene;
}
