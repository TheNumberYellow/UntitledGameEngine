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
    auto it = m_BehaviourPrototypes.find(BehaviourName);
    if (it != m_BehaviourPrototypes.end())
    {
        Engine::DEBUGPrint("That behaviour prototype already exists...wtf");
    }
    m_BehaviourPrototypes[BehaviourName] = NewBehaviour;
    return m_BehaviourPrototypes[BehaviourName];
}

Behaviour* BehaviourRegistry::AttachNewBehaviour(std::string BehaviourName, Model* Model)
{
    auto it = m_BehaviourPrototypes.find(BehaviourName);
    if (it != m_BehaviourPrototypes.end())
    {
        for (auto AttachedBehaviour : m_AttachedBehaviours[Model])
        {
            if (AttachedBehaviour->BehaviourName == BehaviourName)
            {
                Engine::Alert(BehaviourName + " already exists on model.");
            }
        }

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

void BehaviourRegistry::InitializeModelBehaviours(Model* Model, Scene* Scene)
{
    auto it = m_AttachedBehaviours.find(Model);
    if (it != m_AttachedBehaviours.end())
    {
        for (auto& Behaviour : it->second)
        {
            Behaviour->Initialize(Scene);
        }
    }
}

void BehaviourRegistry::UpdateModelBehaviours(Model* Model, Scene* Scene, double DeltaTime)
{
    auto it = m_AttachedBehaviours.find(Model);
    if (it != m_AttachedBehaviours.end())
    {
        for (auto& Behaviour : it->second)
        {
            Behaviour->Update(Scene, DeltaTime);
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

void BehaviourRegistry::DrawEntityInspectorPanel(Model* model)
{
    auto it = m_AttachedBehaviours.find(model);
    if (it != m_AttachedBehaviours.end())
    {
        for (auto& Behaviour : it->second)
        {
            Behaviour->DrawInspectorPanel();
        }
    }
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
    auto it = m_AttachedBehaviours.find(Model);

    if (it != m_AttachedBehaviours.end())
    {
        for (auto B : m_AttachedBehaviours[Model])
        {
            delete B;
        }
        m_AttachedBehaviours[Model].clear();
        m_AttachedBehaviours.erase(Model);
    }
}

void BehaviourRegistry::ClearAllAttachedBehaviours()
{
    for (auto M : m_AttachedBehaviours)
    {
        for (auto B : M.second)
        {
            delete B;
        }
    }
    m_AttachedBehaviours.clear();
}

std::unordered_map<std::string, Behaviour*> BehaviourRegistry::GetBehaviours()
{
    return m_BehaviourPrototypes;
}
