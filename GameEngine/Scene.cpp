#include "Scene.h"

#include "Behaviour/Behaviour.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numbers>
#include <set>
#include <filesystem>

const std::string Separator = " ";

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

Scene::Scene(Scene& other)
{
    m_Cameras.resize(1);

    m_Graphics = other.m_Graphics;
    m_Collisions = other.m_Collisions;

    m_ShadowCamera = other.m_ShadowCamera;
    m_DirLight = other.m_DirLight;
    m_ShadowBuffer = other.m_ShadowBuffer;

    for (auto& model : other.m_UntrackedModels)
    {
        Model* newModel = new Model(*model);
        m_UntrackedModels.push_back(newModel);

        Behaviour* oldBehaviour = BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(model);
        if (oldBehaviour)
        {
            BehaviourRegistry::Get()->AttachNewBehaviour(oldBehaviour->BehaviourName, newModel);
        }
    }
}

Scene& Scene::operator=(const Scene& other)
{
    for (auto& model : m_UntrackedModels)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(model);
    }
    m_UntrackedModels.clear();

    m_Cameras.clear();
    m_Cameras.resize(1);

    m_Graphics = other.m_Graphics;
    m_Collisions = other.m_Collisions;

    m_ShadowCamera = other.m_ShadowCamera;
    m_DirLight = other.m_DirLight;
    m_ShadowBuffer = other.m_ShadowBuffer;

    for (auto& model : other.m_UntrackedModels)
    {
        Model* newModel = new Model(*model);
        m_UntrackedModels.push_back(newModel);

        Behaviour* oldBehaviour = BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(model);
        if (oldBehaviour)
        {
            BehaviourRegistry::Get()->AttachNewBehaviour(oldBehaviour->BehaviourName, newModel);
        }
    }

    return *this;
}

Scene::~Scene()
{
    for (auto& model : m_UntrackedModels)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(model);
    }
    m_UntrackedModels.clear();

}

void Scene::Init(GraphicsModule& graphics, CollisionModule& collisions)
{
    m_Graphics = &graphics;
    m_Collisions = &collisions;
    m_ShadowBuffer = graphics.CreateFBuffer(Vec2i(8000, 8000), FBufferFormat::DEPTH);
}

void Scene::Pause()
{
    m_Paused = true;
}

void Scene::UnPause()
{
    m_Paused = false;
}

bool Scene::IsPaused()
{
    return m_Paused;
}

Model* Scene::AddModel(Model model, std::string name)
{
    if (name == "")
    {
        m_UntrackedModels.push_back(new Model(model));
        return m_UntrackedModels.back();
    }

    m_Models.insert(std::pair<std::string, Model*>(name, new Model(model)));
    return m_Models[name];
}


Model* Scene::GetModel(std::string name)
{
    auto it = m_Models.find(name);
    if (it != m_Models.end())
    {
        return (it->second);
    }
    else
    {
        return nullptr;
    }
}

Model* Scene::GetModelByTag(std::string tag)
{
    for (auto& model : m_UntrackedModels)
    {
        if (model->m_Name == tag)
        {
            return model;
        }
    }
    return nullptr;
}

std::vector<Model*> Scene::GetModelsByTag(std::string tag)
{
    std::vector<Model*> result;
    for (auto& model : m_UntrackedModels)
    {
        if (model->m_Name == tag)
        {
            result.push_back(model);
        }
    }
    return result;
}

void Scene::DeleteModel(Model* model)
{
    auto it = std::find(m_UntrackedModels.begin(), m_UntrackedModels.end(), model);
    if (it != m_UntrackedModels.end())
    {
        m_UntrackedModels.erase(it);

        BehaviourRegistry::Get()->ClearBehavioursOnEntity(model);
    }
}

void Scene::AddCamera(Camera* camera)
{
    m_Cameras.push_back(camera);

    // If in editor


}

Camera* Scene::GetCamera()
{
    if (!m_Cameras[0])
    {
        m_Cameras[0] = new Camera();
    }

    return m_Cameras[0];
}

void Scene::SetCamera(Camera* camera)
{
    m_Cameras[0] = camera;
}

void Scene::UpdateBehaviours(ModuleManager& Modules, float DeltaTime)
{
    BehaviourRegistry* Registry = BehaviourRegistry::Get();
    for (auto& it : m_UntrackedModels)
    {
        Registry->UpdateModelBehaviours(it, Modules, this, DeltaTime);
    }
}

void Scene::Draw(GraphicsModule& graphics, Framebuffer_ID buffer)
{

    for (auto& it : m_Models)
    {
        RenderCommand command;
        command.material = it.second->m_TexturedMeshes[0].m_Material;
        command.mesh = it.second->m_TexturedMeshes[0].m_Mesh.Id;
        command.transform = it.second->GetTransform();
        command.model = it.second;

        graphics.AddRenderCommand(command);
    }

    for (auto& it : m_UntrackedModels)
    {
        RenderCommand command;
        command.material = it->m_TexturedMeshes[0].m_Material;
        command.mesh = it->m_TexturedMeshes[0].m_Mesh.Id;
        command.transform = it->GetTransform();
        command.model = it;

        //if (it->Type == ModelType::PLANE)
        //{
        //    std::vector<Vertex*> Vertices = graphics.m_Renderer.MapMeshVertices(command.mesh);
        //    
        //    Vec3f Pos = it->GetTransform().GetPosition();

        //    for (auto& Vert : Vertices)
        //    {
        //        Vec3f A = Vert->position + Pos;
        //        Vec3f B = (A + Vert->normal);

        //        graphics.DebugDrawLine(A, B);
        //    }


        //    graphics.m_Renderer.UnmapMeshVertices(command.mesh);
        //}

        graphics.AddRenderCommand(command);

    }

    graphics.Render(buffer, *(m_Cameras[0]), m_DirLight);
    
#if 0
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
            graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_ShadowShader, "Transformation", it.second->GetTransform().GetTransformMatrix());
            graphics.m_Renderer.DrawMesh(it.second->m_TexturedMeshes[0].m_Mesh.Id);
        }
        for (auto& it : m_UntrackedModels)
        {
            graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_ShadowShader, "Transformation", it->GetTransform().GetTransformMatrix());
            graphics.m_Renderer.DrawMesh(it->m_TexturedMeshes[0].m_Mesh.Id);
        }
    }
    graphics.SetActiveFrameBuffer(buffer);

    graphics.m_Renderer.SetActiveShader(graphics.m_TexturedMeshShader);

    graphics.m_Renderer.SetActiveFBufferTexture(shadowBuffer, "ShadowMap");
    graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_TexturedMeshShader, "LightSpaceMatrix", m_ShadowCamera.GetCamMatrix());

    graphics.SetCamera(m_Cameras[0]);

    graphics.SetDirectionalLight(m_DirLight);

    for (auto& it : m_Models)
    {
        graphics.Draw(*it.second);
    }

    for (auto& it : m_UntrackedModels)
    {
        graphics.Draw(*it);
    }
#endif
}

void Scene::EditorDraw(GraphicsModule& graphics, Framebuffer_ID buffer)
{
    Draw(graphics, buffer);
}

void Scene::SetDirectionalLight(DirectionalLight light)
{
    m_DirLight = light;
}

SceneRayCastHit Scene::RayCast(Ray ray, CollisionModule& collision, std::vector<Model*> IgnoredModels)
{
    SceneRayCastHit finalHit;

    for (auto& it : m_Models)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it.second) > 0)
        {
            continue;
        }
        CollisionMesh& colMesh = collision.GetCollisionMeshFromMesh(it.second->m_TexturedMeshes[0].m_Mesh);

        finalHit = Closer(finalHit, SceneRayCastHit{ collision.RayCast(ray, colMesh, it.second->GetTransform()), it.second });
    }

    for (auto& it : m_UntrackedModels)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it) > 0)
        {
            continue;
        }
        CollisionMesh& colMesh = collision.GetCollisionMeshFromMesh(it->m_TexturedMeshes[0].m_Mesh);
        
        finalHit = Closer(finalHit, SceneRayCastHit{ collision.RayCast(ray, colMesh, it->GetTransform()), it });
    }

    return finalHit;
}

Model* Scene::MenuListEntities(UIModule& ui, Font& font)
{
    Vec2f cursor = Vec2f(0.0f, 0.0f);

    Model* result = nullptr;
    for (int i = 0; i < m_UntrackedModels.size(); ++i)
    {
        Model* model = m_UntrackedModels[i];
        Vec3f pos = model->GetTransform().GetPosition();

        std::string modelDesc = model->m_Name.empty() ? "<unnamed>" : model->m_Name;

        Rect rect;
        rect.location = cursor;
        rect.size = Vec2f(160.0f, 20.0f);

        if (ui.TextButton(modelDesc, rect, 5.0f))
        {
            result = m_UntrackedModels[i];
        }
        cursor.y += 20.0f;
    }

    return result;
}

void Scene::Save(std::string FileName)
{   
    // Set of all textures used in the scene (TODO: replace with Materials)
    std::set<Material> Materials;
    // Set of all Static Meshes used in the scene
    std::set<StaticMesh> StaticMeshes;

    for (auto& it : m_UntrackedModels)
    {
        Texture tex = it->m_TexturedMeshes[0].m_Material.m_Albedo;
        StaticMesh mesh = it->m_TexturedMeshes[0].m_Mesh;
        
        Material mat = it->m_TexturedMeshes[0].m_Material;
        //if (tex.LoadedFromFile)
        //{
        //  Materials.insert(mat);
        //}
        Materials.insert(mat);
        if (mesh.LoadedFromFile)
        {
            StaticMeshes.insert(it->m_TexturedMeshes[0].m_Mesh);
        }
    }

    std::vector<Material> MaterialVec(Materials.begin(), Materials.end());
    std::vector<StaticMesh> StaticMeshVec(StaticMeshes.begin(), StaticMeshes.end());


    std::ofstream File(FileName);

    if (!File.is_open())
    {
        Engine::DEBUGPrint("Failed to save scene :(");
        return;
    }

    File << "Textures:" << std::endl;
    for (auto& Mat : MaterialVec)
    {
        std::string Path = Mat.m_Albedo.Path.GetFullPath();
        std::string NormPath = Mat.m_Normal.Path.GetFullPath();
        std::string RoughPath = Mat.m_Roughness.Path.GetFullPath();
        std::string MetalPath = Mat.m_Metallic.Path.GetFullPath();
        std::string AOPath = Mat.m_AO.Path.GetFullPath();

        File << Path << Separator << NormPath << Separator << RoughPath << Separator << MetalPath << Separator << AOPath << std::endl;
    }

    File << "StaticMeshes:" << std::endl;
    for (auto& Mesh : StaticMeshVec)
    {
        std::string Path = Mesh.Path.GetFullPath();
        File << Path << std::endl;
    }

    File << "Entities:" << std::endl;

for (auto& it : m_UntrackedModels)
{
    int64_t StaticMeshIndex = 0;
    int64_t TextureIndex = 0;

    bool GeneratedMesh = false;

    auto MeshIt = std::find(StaticMeshVec.begin(), StaticMeshVec.end(), it->m_TexturedMeshes[0].m_Mesh);
    if (MeshIt != StaticMeshVec.end())
    {
        StaticMeshIndex = MeshIt - StaticMeshVec.begin();
    }
    else
    {
        GeneratedMesh = true;
        //Engine::FatalError("Could not find mesh while saving scene (this should never happen)");
    }

    auto MaterialIt = std::find(MaterialVec.begin(), MaterialVec.end(), it->m_TexturedMeshes[0].m_Material);
    if (MaterialIt != MaterialVec.end())
    {
        TextureIndex = MaterialIt - MaterialVec.begin();
    }
    else
    {
        Engine::FatalError("Could not find texture while saving scene (this should never happen)");
    }

    File << TextureIndex << Separator;

    if (!GeneratedMesh)
    {
        File << StaticMeshIndex << Separator;
    }
    else
    {
        File << "B" << Separator;
        // For now, all generated meshes are boxes, so store the AABB
        AABB BoxAABB = m_Collisions->GetCollisionMeshFromMesh(it->m_TexturedMeshes[0].m_Mesh).boundingBox;

        File << BoxAABB.min.x << Separator << BoxAABB.min.y << Separator << BoxAABB.min.z << Separator;
        File << BoxAABB.max.x << Separator << BoxAABB.max.y << Separator << BoxAABB.max.z << Separator;
    }

    Mat4x4f TransMat = it->GetTransform().GetTransformMatrix();

    for (int i = 0; i < 4; ++i)
    {
        File << TransMat[i].x << Separator << TransMat[i].y << Separator << TransMat[i].z << Separator << TransMat[i].w << Separator;
    }

    std::vector<std::string> BehaviourNames = BehaviourRegistry::Get()->GetBehavioursAttachedToEntity(it);

    for (std::string BehaviourName : BehaviourNames)
    {
        File << BehaviourName << Separator;
    }

    File << std::endl;

}

File.close();

}

void Scene::Load(std::string FileName)
{
    std::ifstream File(FileName);

    if (!File.is_open())
    {
        return;
    }

    m_Models.clear();
    m_UntrackedModels.clear();

    BehaviourRegistry::Get()->ClearAllAttachedBehaviours();

    std::vector<Material> SceneMaterials;
    std::vector<StaticMesh> SceneStaticMeshes;

    FileReaderState ReaderState;

    std::string Line;
    while (std::getline(File, Line))
    {
        std::vector<std::string> LineTokens = StringUtils::Split(Line, Separator);

        if (GetReaderStateFromToken(LineTokens[0], ReaderState))
        {
            // State changed, go to next line
            continue;
        }
        switch (ReaderState)
        {
        case TEXTURES:
            if (LineTokens.size() == 5)
            {
                Texture DiffuseTex = m_Graphics->LoadTexture(LineTokens[0]);
                Texture NormalTex = m_Graphics->LoadTexture(LineTokens[1]);
                Texture RoughnessTex = m_Graphics->LoadTexture(LineTokens[2]);
                Texture MetallicTex = m_Graphics->LoadTexture(LineTokens[3]);
                Texture AOTex = m_Graphics->LoadTexture(LineTokens[4]);

                SceneMaterials.push_back(m_Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex, MetallicTex, AOTex));
            }
            else if (LineTokens.size() == 4)
            {
                Texture DiffuseTex = m_Graphics->LoadTexture(LineTokens[0]);
                Texture NormalTex = m_Graphics->LoadTexture(LineTokens[1]);
                Texture RoughnessTex = m_Graphics->LoadTexture(LineTokens[2]);
                Texture MetallicTex = m_Graphics->LoadTexture(LineTokens[3]);

                SceneMaterials.push_back(m_Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex, MetallicTex));
            }
            else if (LineTokens.size() == 3)
            {
                Texture DiffuseTex = m_Graphics->LoadTexture(LineTokens[0]);
                Texture NormalTex = m_Graphics->LoadTexture(LineTokens[1]);
                Texture RoughnessTex = m_Graphics->LoadTexture(LineTokens[2]);

                SceneMaterials.push_back(m_Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex));
            }
            else if (LineTokens.size() == 2)
            {
                Texture DiffuseTex = m_Graphics->LoadTexture(LineTokens[0]);
                Texture NormalTex = m_Graphics->LoadTexture(LineTokens[1]);

                SceneMaterials.push_back(m_Graphics->CreateMaterial(DiffuseTex, NormalTex));
            }
            else
            {
                Texture DiffuseTex = m_Graphics->LoadTexture(LineTokens[0]);
                SceneMaterials.push_back(m_Graphics->CreateMaterial(DiffuseTex));
            }
            break;
        case STATIC_MESHES:
            SceneStaticMeshes.push_back(m_Graphics->LoadMesh(LineTokens[0]));
            break;
        case ENTITIES:
        {
            int At = 0;
            
            int MaterialIndex = std::stoi(LineTokens[At++]);
            
            Model* NewModel;

            if (LineTokens[At++] == "B")
            {
                AABB BoxAABB;
                BoxAABB.min.x = std::stof(LineTokens[At++]); BoxAABB.min.y = std::stof(LineTokens[At++]); BoxAABB.min.z = std::stof(LineTokens[At++]);
                BoxAABB.max.x = std::stof(LineTokens[At++]); BoxAABB.max.y = std::stof(LineTokens[At++]); BoxAABB.max.z = std::stof(LineTokens[At++]);

                NewModel = new Model(m_Graphics->CreateBoxModel(BoxAABB, SceneMaterials[MaterialIndex]));
            }
            else
            {
                int StaticMeshIndex = std::stoi(LineTokens[1]);
                NewModel = new Model(m_Graphics->CreateModel(TexturedMesh(SceneStaticMeshes[StaticMeshIndex], SceneMaterials[MaterialIndex])));
            }

            Mat4x4f EntityTransform;
            EntityTransform[0].x = std::stof(LineTokens[At++]); EntityTransform[0].y = std::stof(LineTokens[At++]); EntityTransform[0].z = std::stof(LineTokens[At++]); EntityTransform[0].w = std::stof(LineTokens[At++]);
            EntityTransform[1].x = std::stof(LineTokens[At++]); EntityTransform[1].y = std::stof(LineTokens[At++]); EntityTransform[1].z = std::stof(LineTokens[At++]); EntityTransform[1].w = std::stof(LineTokens[At++]);
            EntityTransform[2].x = std::stof(LineTokens[At++]); EntityTransform[2].y = std::stof(LineTokens[At++]); EntityTransform[2].z = std::stof(LineTokens[At++]); EntityTransform[2].w = std::stof(LineTokens[At++]);
            EntityTransform[3].x = std::stof(LineTokens[At++]); EntityTransform[3].y = std::stof(LineTokens[At++]); EntityTransform[3].z = std::stof(LineTokens[At++]); EntityTransform[3].w = std::stof(LineTokens[At++]);

            NewModel->GetTransform().SetTransformMatrix(EntityTransform);

            size_t NumBehaviours = LineTokens.size() - (At);

            for (int i = 0; i < NumBehaviours; ++i)
            {
                std::string BehaviourName = LineTokens[At++];
                BehaviourRegistry::Get()->AttachNewBehaviour(BehaviourName, NewModel);
            }

            m_UntrackedModels.push_back(NewModel);

            break;
        }
        default:
            break;
        }
    }

}

bool Scene::IsIgnored(Model* model, std::vector<Model*> ignoredModels)
{
    for (auto it : ignoredModels)
    {
        if (model == it)
        {
            return true;
        }
    }
    return false;
}

bool Scene::GetReaderStateFromToken(std::string Token, FileReaderState& OutState)
{
    if (Token == "Textures:")
    {
        OutState = TEXTURES;
        return true;
    }
    else if (Token == "StaticMeshes:")
    {
        OutState = STATIC_MESHES;
        return true;
    }
    else if (Token == "Entities:")
    {
        OutState = ENTITIES;
        return true;
    }
    // Otherwise leave state the same
    return false;
}
