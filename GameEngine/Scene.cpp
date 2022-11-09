#include "Scene.h"

Scene::Scene()
    : m_Camera(nullptr)
{
}

Scene::~Scene()
{
}

Model* Scene::AddModel(Model model, std::string name)
{
    if (name == "")
    {
        m_UntrackedModels.push_back(model);
        return &m_UntrackedModels.back();
    }

    m_Models.insert(std::pair<std::string, Model>(name, model));
    return &m_Models[name];
}


Model* Scene::GetModel(std::string name)
{
    auto it = m_Models.find(name);
    if (it != m_Models.end())
    {
        return &(it->second);
    }
    else
    {
        return nullptr;
    }
}

Camera& Scene::GetCamera()
{
    if (!m_Camera)
    {
        m_Camera = new Camera();
    }

    return *m_Camera;
}

void Scene::SetCamera(Camera* camera)
{
    m_Camera = camera;
}

void Scene::Draw(GraphicsModule& graphics)
{
    graphics.SetCamera(m_Camera);

    graphics.SetDirectionalLight(m_DirLight);

    for (auto& it : m_Models)
    {
        graphics.Draw(it.second);
    }

    for (auto& it : m_UntrackedModels)
    {
        graphics.Draw(it);
    }
}

SceneRayCastHit Scene::RayCast(Ray ray, CollisionModule& collision)
{
    SceneRayCastHit finalHit;

    for (auto& it : m_Models)
    {
        RayCastHit newHit;
        CollisionMesh& colMesh = collision.GetCollisionMeshFromMesh(it.second.m_TexturedMeshes[0].m_Mesh);
        newHit = collision.RayCast(ray, colMesh, it.second.GetTransform());
        if (newHit.hit && newHit.hitDistance < finalHit.rayCastHit.hitDistance)
        {
            finalHit.rayCastHit = newHit;
            finalHit.hitModel = &it.second;
        }
    }

    for (auto& it : m_UntrackedModels)
    {
        RayCastHit newHit;
        CollisionMesh& colMesh = collision.GetCollisionMeshFromMesh(it.m_TexturedMeshes[0].m_Mesh);
        newHit = collision.RayCast(ray, colMesh, it.GetTransform());
        if (newHit.hit && newHit.hitDistance < finalHit.rayCastHit.hitDistance)
        {
            finalHit.rayCastHit = newHit;
            finalHit.hitModel = &it;
        }
    }

    return finalHit;
}
