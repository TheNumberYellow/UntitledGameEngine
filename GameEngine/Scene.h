#pragma once

#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include <string>

typedef uint64_t Entity_ID;

struct SceneRayCastHit
{
    RayCastHit rayCastHit;
    Model* hitModel;
};

SceneRayCastHit Closer(const SceneRayCastHit& lhs, const SceneRayCastHit& rhs);

class Scene
{
public:
    Scene();
    ~Scene();

    void Init(GraphicsModule& graphics);

    Model* AddModel(Model model, std::string name = "");
    Model* GetModel(std::string name);

    void AddCamera(Camera* camera);

    Camera& GetCamera();
    void SetCamera(Camera* camera);

    void Update();

    void Draw(GraphicsModule& graphics, Framebuffer_ID buffer);
    void EditorDraw(GraphicsModule& graphics, Framebuffer_ID buffer);

    void SetDirectionalLight(DirectionalLight light);

    SceneRayCastHit RayCast(Ray ray, CollisionModule& collision);

    Model* MenuListEntities(UIModule& ui, Font& font);

private:
    std::unordered_map<std::string, Model> m_Models;
    std::vector<Model> m_UntrackedModels;
    DirectionalLight m_DirLight;

    std::vector<Camera*> m_Cameras;

    Camera m_ShadowCamera;
    Framebuffer_ID shadowBuffer;
};
