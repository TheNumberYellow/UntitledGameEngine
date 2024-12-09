#pragma once

#include "GameEngine.h"
#include "Scene.h"
#include <unordered_map>
#include <string>

#define DEFINE_BEHAVIOUR(Type) static Behaviour* Type ## _Prototype; \
                               virtual Behaviour* Clone() const { return new Type(*this); }
#define REGISTER_BEHAVIOUR(Type) Behaviour* Type::Type ## _Prototype = BehaviourRegistry::Get()->AddBehaviourPrototype(#Type, new Type()); \


class Model;

class Behaviour
{
public:
    Behaviour();
    Behaviour(Model* transform);

    virtual Behaviour* Clone() const = 0;

    virtual void Initialize(Scene* Scene) {}
    virtual void Update(Scene* Scene, double DeltaTime) {}

    virtual void DrawInspectorPanel() {}

    std::string BehaviourName;
    Model* m_Model = nullptr;

private:

};

class BehaviourRegistry
{
public:

    static BehaviourRegistry* Get();

    Behaviour* AddBehaviourPrototype(std::string BehaviourName, Behaviour* NewBehaviour);

    Behaviour* AttachNewBehaviour(std::string BehaviourName, Model* Model);

    void InitializeModelBehaviours(Model* Model, Scene* Scene);
    void UpdateModelBehaviours(Model* Model, Scene* Scene, double DeltaTime);
    
    Behaviour* GetBehaviourAttachedToEntity(Model* Model);
    void DrawEntityInspectorPanel(Model* model);
    std::vector<std::string> GetBehavioursAttachedToEntity(Model* Model);
    void ClearBehavioursOnEntity(Model* Model);

    void ClearAllAttachedBehaviours();

    std::unordered_map<std::string, Behaviour*> GetBehaviours();

private:

    BehaviourRegistry() = default;

    std::unordered_map<std::string, Behaviour*> m_BehaviourPrototypes;
    std::unordered_map<Model*, std::vector<Behaviour*>> m_AttachedBehaviours;

    static BehaviourRegistry* Instance;    
};

