#include "Behaviour.h"

BehaviourRegistry* BehaviourRegistry::Instance = nullptr;

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

std::unordered_map<std::string, Behaviour*> BehaviourRegistry::GetBehaviours()
{
    return m_BehaviourPrototypes;
}

Behaviour::Behaviour()
{
    randomFloat = 420.0f;
}
