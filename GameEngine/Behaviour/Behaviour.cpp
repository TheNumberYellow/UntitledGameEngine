#include "Behaviour.h"

BehaviourRegistry* BehaviourRegistry::Instance = nullptr;

Behaviour::Behaviour()
{
}

Behaviour::Behaviour(Transform* transform)
{
    m_Transform = transform;
}

BehaviourRegistry* BehaviourRegistry::Get()
{
    if (!Instance)
    {
        Instance = new BehaviourRegistry();
    }

    return Instance;
}

Behaviour* BehaviourRegistry::AddBehaviourPrototype(std::string BehaviourName, Behaviour* NewBehaviour)
{
    m_BehaviourPrototypes[BehaviourName] = NewBehaviour;
    return m_BehaviourPrototypes[BehaviourName];
}

void BehaviourRegistry::AttachNewBehaviour(std::string BehaviourName, Transform* Transform)
{
    auto it = m_BehaviourPrototypes.find(BehaviourName);
    if (it != m_BehaviourPrototypes.end())
    {
        Behaviour* NewBehaviour = it->second->Clone();
        NewBehaviour->m_Transform = Transform;

        m_AttachedBehaviours.push_back(NewBehaviour);
    }
    else
    {
        Engine::FatalError("Could not find behaviour " + BehaviourName);
    }
}

void BehaviourRegistry::UpdateAllBehaviours(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    for (auto Behaviour : m_AttachedBehaviours)
    {
        Behaviour->Update(Modules, Scene, DeltaTime);
    }
}

std::unordered_map<std::string, Behaviour*> BehaviourRegistry::GetBehaviours()
{
    return m_BehaviourPrototypes;
}
