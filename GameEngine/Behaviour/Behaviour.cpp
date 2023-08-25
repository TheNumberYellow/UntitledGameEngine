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

        m_AttachedBehaviours[Model].push_back(NewBehaviour);

        return NewBehaviour;
    }
    else
    {
        Engine::FatalError("Could not find behaviour " + BehaviourName);
    }

    return nullptr;
}

void BehaviourRegistry::UpdateModelBehaviours(Model* Model, ModuleManager& Modules, Scene* Scene, float DeltaTime)
{
    auto it = m_AttachedBehaviours.find(Model);
    if (it != m_AttachedBehaviours.end())
    {
        for (auto& Behaviour : it->second)
        {
            Behaviour->Update(Modules, Scene, DeltaTime);
        }
    }
}

Behaviour* BehaviourRegistry::GetBehaviourAttachedToEntity(Model* Model)
{
    auto it = m_AttachedBehaviours.find(Model);
    if (it != m_AttachedBehaviours.end())
    {
        return it->second[0];
    }
    return nullptr;
}

std::vector<std::string> BehaviourRegistry::GetBehavioursAttachedToEntity(Model* Model)
{
    std::vector<std::string> Result;

    auto it = m_AttachedBehaviours.find(Model);
    if (it != m_AttachedBehaviours.end())
    {
        for (auto& Behaviour : it->second)
        {
            Result.push_back(Behaviour->BehaviourName);
        }
    }

    return Result;
}

void BehaviourRegistry::ClearBehavioursOnEntity(Model* Model)
{
    m_AttachedBehaviours.erase(Model);
    //for (int i = (int)m_AttachedBehaviours.size() - 1; i >= 0; --i)
    //{
    //    if (m_AttachedBehaviours[i]->m_Model == Model)
    //    {
    //        m_AttachedBehaviours.erase(m_AttachedBehaviours.begin() + i);
    //        continue;
    //    }
    //}
}

void BehaviourRegistry::ClearAllAttachedBehaviours()
{
    m_AttachedBehaviours.clear();
}

std::unordered_map<std::string, Behaviour*> BehaviourRegistry::GetBehaviours()
{
    return m_BehaviourPrototypes;
}
