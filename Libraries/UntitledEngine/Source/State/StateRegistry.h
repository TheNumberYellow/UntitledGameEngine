#pragma once

#include "BaseState.h"
#include "GameEngine.h"

#include <unordered_map>

#define DEFINE_STATE(StateName) static BaseState* StateName ## _Prototype; \
                                //virtual BaseState* Clone() const { return new StateName(*this); }
#define REGISTER_STATE(StateName) BaseState* StateName::StateName ## _Prototype = StateRegistry::Get()->AddState(#StateName, new StateName());

class StateRegistry
{
public:

    static StateRegistry* Get();

    BaseState* AddState(std::string StateName, BaseState* NewState);
    BaseState* GetState(std::string StateName);

    std::vector<std::string> GetStateNames();

private:

    StateRegistry() = default;

    std::unordered_map<std::string, BaseState*> m_StatePrototypes;

    static StateRegistry* Instance;
};