#pragma once

#include "GameEngine.h"

#include <unordered_map>
#include <string>

#define DEFINE_BEHAVIOUR(Type) static Behaviour* Type ## _Prototype;
#define REGISTER_BEHAVIOUR(Type) Behaviour* Type::Type ## _Prototype = BehaviourRegistry::Get()->AddBehaviourPrototype(#Type, new Type());

class Behaviour
{
public:
    Behaviour();

    virtual void Update(float DeltaTime) {}
private:

};

class BehaviourRegistry
{
public:

    static BehaviourRegistry* Get();

    Behaviour* AddBehaviourPrototype(std::string BehaviourName, Behaviour* NewBehaviour);

    std::unordered_map<std::string, Behaviour*> GetBehaviours();

private:

    BehaviourRegistry() = default;

    std::unordered_map<std::string, Behaviour*> m_BehaviourPrototypes;

    static BehaviourRegistry* Instance;    
};

