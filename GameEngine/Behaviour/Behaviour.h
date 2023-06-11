#pragma once

#include "GameEngine.h"
#include "Scene.h"

#include <unordered_map>
#include <string>

#define DEFINE_BEHAVIOUR(Type) static Behaviour* Type ## _Prototype; \
                               virtual Behaviour* Clone() const { return new Type(*this); }
#define REGISTER_BEHAVIOUR(Type) Behaviour* Type::Type ## _Prototype = BehaviourRegistry::Get()->AddBehaviourPrototype(#Type, new Type()); \
                                

class Behaviour
{
public:
    Behaviour();
    Behaviour(Transform* transform);

    virtual Behaviour* Clone() const = 0;
    virtual void Update(ModuleManager& Modules, Scene* Scene, float DeltaTime) {}
    
    Transform* m_Transform = nullptr;
private:

};

class BehaviourRegistry
{
public:

    static BehaviourRegistry* Get();

    Behaviour* AddBehaviourPrototype(std::string BehaviourName, Behaviour* NewBehaviour);

    void AttachNewBehaviour(std::string BehaviourName, Transform* Transform);

    void UpdateAllBehaviours(ModuleManager& Modules, Scene* Scene, float DeltaTime);

    std::unordered_map<std::string, Behaviour*> GetBehaviours();

private:

    BehaviourRegistry() = default;

    std::unordered_map<std::string, Behaviour*> m_BehaviourPrototypes;
    std::vector<Behaviour*> m_AttachedBehaviours;

    static BehaviourRegistry* Instance;    
};

