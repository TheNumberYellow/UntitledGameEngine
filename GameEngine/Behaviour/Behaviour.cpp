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

Behaviour* BehaviourRegistry::AttachNewBehaviour(std::string BehaviourName, Model* Model)
{
    auto it = m_BehaviourPrototypes.find(BehaviourName);
    if (it != m_BehaviourPrototypes.end())
    {
        Behaviour* NewBehaviour = it->second->Clone();
        NewBehaviour->m_Model = Model;
        NewBehaviour->BehaviourName = BehaviourName;

        m_AttachedBehaviours.push_back(NewBehaviour);

        return NewBehaviour;
    }
    else
    {
        Engine::FatalError("Could not find behaviour " + BehaviourName);
    }

    return nullptr;
}

void BehaviourRegistry::UpdateAllBehaviours(ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    // TODO(Fraser): For some reason this _sometimes_ causes a nullptr exception on the Behaviour... look into later.
    //for (auto Behaviour : m_AttachedBehaviours)
    //{
    //    Behaviour->Update(Modules, Scene, DeltaTime);
    //}

    if (Scene->IsPaused())
    {
        return;
    }

    for (int i = (int)m_AttachedBehaviours.size() - 1; i >= 0; --i)
    {
        if (m_AttachedBehaviours[i]->m_Model == nullptr)
        {
            m_AttachedBehaviours.erase(m_AttachedBehaviours.begin() + i);
            continue;
        }
        m_AttachedBehaviours[i]->Update(Modules, Scene, DeltaTime);
    }
}

Behaviour* BehaviourRegistry::GetBehaviourAttachedToEntity(Model* Model)
{
    for (auto Behaviour : m_AttachedBehaviours)
    {
        if (Behaviour->m_Model == Model)
        {
            return Behaviour;
        }
    }
    return nullptr;
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

void BehaviourRegistry::ClearBehavioursOnEntity(Model* Model)
{
    for (int i = (int)m_AttachedBehaviours.size() - 1; i >= 0; --i)
    {
        if (m_AttachedBehaviours[i]->m_Model == Model)
        {
            m_AttachedBehaviours.erase(m_AttachedBehaviours.begin() + i);
            continue;
        }
    }
}

void BehaviourRegistry::ClearAllAttachedBehaviours()
{
    m_AttachedBehaviours.clear();
}

std::unordered_map<std::string, Behaviour*> BehaviourRegistry::GetBehaviours()
{
    return m_BehaviourPrototypes;
}
