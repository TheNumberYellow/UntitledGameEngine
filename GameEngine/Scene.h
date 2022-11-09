#pragma once

#include "Modules/GraphicsModule.h"
#include "Modules/CollisionModule.h"

#include <string>

struct SceneRayCastHit
{
    RayCastHit rayCastHit;
    Model* hitModel;
};

class Scene
{
public:
    Scene();
    ~Scene();

    Model* AddModel(Model model, std::string name = "");
    Model* GetModel(std::string name);

    Camera& GetCamera();
    void SetCamera(Camera* camera);

    void Draw(GraphicsModule& graphics);
    
    SceneRayCastHit RayCast(Ray ray, CollisionModule& collision);

private:
    std::unordered_map<std::string, Model> m_Models;
    std::vector<Model> m_UntrackedModels;
    DirectionalLight m_DirLight;
    Camera* m_Camera;
};
