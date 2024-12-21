#include "StateRegistry.h"

StateRegistry* StateRegistry::Instance = nullptr;

StateRegistry* StateRegistry::Get()
{
    if (!Instance)
    {
        Instance = new StateRegistry();
    }

    return Instance;
}

BaseState* StateRegistry::AddState(std::string StateName, BaseState* NewState)
{
    auto it = m_StatePrototypes.find(StateName);
    if (it != m_StatePrototypes.end())
    {
        Engine::DEBUGPrint("That base state prototype already exists.");
    }
    m_StatePrototypes[StateName] = NewState;
    return m_StatePrototypes[StateName];
}

BaseState* StateRegistry::GetState(std::string StateName)
{
    auto it = m_StatePrototypes.find(StateName);
    if (it != m_StatePrototypes.end())
    {
        return it->second;
    }
    else
    {
        Engine::FatalError("Could not find state " + StateName);
    }

    return nullptr;
}

std::vector<std::string> StateRegistry::GetStateNames()
{
    std::vector<std::string> Result;

    for (auto& it : m_StatePrototypes)
    {
        Result.push_back(it.first);
    }

    return Result;
}
