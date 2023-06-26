#include "Behaviour.h"

BehaviourRegistry* BehaviourRegistry::Instance = nullptr;

Behaviour::Behaviour()
{
}

Behaviour::Behaviour(Model* transform)
{
    m_Model = transform;
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

void BehaviourRegistry::AttachNewBehaviour(std::string BehaviourName, Model* Model)
{
    auto it = m_BehaviourPrototypes.find(BehaviourName);
    if (it != m_BehaviourPrototypes.end())
    {
        Behaviour* NewBehaviour = it->second->Clone();
        NewBehaviour->m_Model = Model;
        NewBehaviour->BehaviourName = BehaviourName;

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

std::vector<std::string> BehaviourRegistry::GetBehavioursAttachedToEntity(Model* Model)
{
    std::vector<std::string> Result;
    for (auto Behaviour : m_AttachedBehaviours)
    {
        if (Behaviour->m_Model == Model)
        {
            Result.push_back(Behaviour->BehaviourName);
        }
    }

    return Result;
}

void BehaviourRegistry::ClearAllAttachedBehaviours()
{
    m_AttachedBehaviours.clear();
}

std::unordered_map<std::string, Behaviour*> BehaviourRegistry::GetBehaviours()
{
    return m_BehaviourPrototypes;
}
