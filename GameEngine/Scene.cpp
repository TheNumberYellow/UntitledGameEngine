#include "Scene.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <numbers>

SceneRayCastHit Closer(const SceneRayCastHit& lhs, const SceneRayCastHit& rhs)
{
    return (lhs.rayCastHit.hitDistance <= rhs.rayCastHit.hitDistance ? lhs : rhs);
}


Scene::Scene()
{
    m_Cameras.resize(1);

    // TEMP
    m_ShadowCamera = Camera(Projection::Orthographic);
    m_ShadowCamera.SetScreenSize(Vec2f(100.0f, 100.0f));
    m_ShadowCamera.SetNearPlane(0.0f);
    m_ShadowCamera.SetFarPlane(100.0f);
    m_ShadowCamera.SetPosition(Vec3f(0.0f, -30.0f, 6.0f));

    m_DirLight.colour = Vec3f(1.0, 1.0, 1.0);
    m_DirLight.direction = m_ShadowCamera.GetDirection();
}

Scene::~Scene()
{
}

void Scene::Init(GraphicsModule& graphics)
{
    shadowBuffer = graphics.CreateFBuffer(Vec2i(8000, 8000), FBufferFormat::DEPTH);
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

void Scene::AddCamera(Camera* camera)
{
    m_Cameras.push_back(camera);

    // If in editor


}

Camera& Scene::GetCamera()
{
    if (!m_Cameras[0])
    {
        m_Cameras[0] = new Camera();
    }

    return *m_Cameras[0];
}

void Scene::SetCamera(Camera* camera)
{
    m_Cameras[0] = camera;
}

void Scene::Update()
{
}

void Scene::Draw(GraphicsModule& graphics, Framebuffer_ID buffer)
{
    Vec3f shadowCamPos = m_Cameras[0]->GetPosition() + (-m_ShadowCamera.GetDirection() * 40.0f);
    m_ShadowCamera.SetPosition(shadowCamPos);
    m_ShadowCamera.SetDirection(m_DirLight.direction);

    // THIS IS ALL TEMPORARY WHILE I WORK ON A SHADOW SYSTEM IN THE GRAPHICS MODULE :^) 
    graphics.SetActiveFrameBuffer(shadowBuffer);
    //graphics.m_Renderer.SetCulling(Cull::Front);
    graphics.m_Renderer.SetActiveShader(graphics.m_ShadowShader);
    {
        graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_ShadowShader, "LightSpaceMatrix", m_ShadowCamera.GetCamMatrix());
        for (auto& it : m_Models)
        {
            graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_ShadowShader, "Transformation", it.second.GetTransform().GetTransformMatrix());
            graphics.m_Renderer.DrawMesh(it.second.m_TexturedMeshes[0].m_Mesh);
        }
        for (auto& it : m_UntrackedModels)
        {
            graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_ShadowShader, "Transformation", it.GetTransform().GetTransformMatrix());
            graphics.m_Renderer.DrawMesh(it.m_TexturedMeshes[0].m_Mesh);
        }
    }
    graphics.SetActiveFrameBuffer(buffer);

    graphics.m_Renderer.SetActiveFBufferTexture(shadowBuffer, 1);
    graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_TexturedMeshShader, "LightSpaceMatrix", m_ShadowCamera.GetCamMatrix());

    graphics.SetCamera(m_Cameras[0]);

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

void Scene::EditorDraw(GraphicsModule& graphics, Framebuffer_ID buffer)
{
    Draw(graphics, buffer);


}

void Scene::SetDirectionalLight(DirectionalLight light)
{
    m_DirLight = light;
}

SceneRayCastHit Scene::RayCast(Ray ray, CollisionModule& collision)
{
    SceneRayCastHit finalHit;

    for (auto& it : m_Models)
    {
        CollisionMesh& colMesh = collision.GetCollisionMeshFromMesh(it.second.m_TexturedMeshes[0].m_Mesh);

        finalHit = Closer(finalHit, SceneRayCastHit{ collision.RayCast(ray, colMesh, it.second.GetTransform()), &it.second });
    }

    for (auto& it : m_UntrackedModels)
    {
        CollisionMesh& colMesh = collision.GetCollisionMeshFromMesh(it.m_TexturedMeshes[0].m_Mesh);
        
        finalHit = Closer(finalHit, SceneRayCastHit{ collision.RayCast(ray, colMesh, it.GetTransform()), &it });
    }

    return finalHit;
}

Model* Scene::MenuListEntities(UIModule& ui, Font& font)
{
    Vec2f cursor = Vec2f(0.0f, 0.0f);

    Model* result = nullptr;
    for (int i = 0; i < m_UntrackedModels.size(); ++i)
    {
        Model model = m_UntrackedModels[i];
        Vec3f pos = model.GetTransform().GetPosition();

        std::string modelDesc = model.m_Name.empty() ? "<unnamed>" : model.m_Name;

        Rect rect;
        rect.location = cursor;
        rect.size = Vec2f(160.0f, 20.0f);

        if (ui.TextButton(modelDesc, rect, 5.0f))
        {
            result = &m_UntrackedModels[i];
        }
        cursor.y += 20.0f;
    }

    return result;
}

