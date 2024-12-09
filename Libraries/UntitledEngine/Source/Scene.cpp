#include "Scene.h"

#include "Behaviour/Behaviour.h"
#include "HalfEdge/HalfEdge.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numbers>
#include <set>
#include <filesystem>


const std::string Separator = " ";

Texture* Scene::LightBillboardTexture = nullptr;
StaticMesh* Scene::CameraMesh = nullptr;
Material* Scene::CameraMaterial = nullptr;

SceneRayCastHit Closer(const SceneRayCastHit& lhs, const SceneRayCastHit& rhs)
{
    return (lhs.rayCastHit.hitDistance <= rhs.rayCastHit.hitDistance ? lhs : rhs);
}

Scene::Scene()
{
    GraphicsModule* Graphics = GraphicsModule::Get();


    m_Cameras.resize(1);


    m_DirLight.colour = Vec3f(1.0, 1.0, 1.0);

    if (!Graphics)
    {
        return;
    }


    AssetRegistry* Registry = AssetRegistry::Get();

    if (!LightBillboardTexture)
    {
        LightBillboardTexture = Registry->LoadTexture("Assets/images/light.png");
    }
    if (!CameraMesh)
    {
        CameraMesh = Registry->LoadStaticMesh("Assets/models/CornyCamera.obj");
    }
    if (!CameraMaterial)
    {
        CameraMaterial = new Material(Graphics->CreateMaterial(*(Registry->LoadTexture("Assets/textures/Camera.png"))));
    }
}

Scene::Scene(Scene& other)
{
    CopyInternal(other);
}

Scene& Scene::operator=(const Scene& other)
{
    CopyInternal(other);

    return *this;
}

Scene::~Scene()
{
    for (auto it : m_Models)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(it.second);
        delete it.second;
    }
    m_Models.clear();
    m_PointLights.clear();
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

Model* Scene::AddModel(Model* model)
{
    GUID newModelID = m_ModelIDGenerator.Generate();

    m_Models[newModelID] = model;

#ifdef USE_EDITOR
    m_EditorClickables.push_back(model);
#endif

    return m_Models[newModelID];
}

Model* Scene::GetModelByTag(std::string tag)
{
    for (auto it : m_Models)
    {
        if (it.second->m_Name == tag)
        {
            return it.second;
        }
    }
    return nullptr;
}

std::vector<Model*> Scene::GetModelsByTag(std::string tag)
{
    std::vector<Model*> result;
    for (auto it : m_Models)
    {
        if (it.second->m_Name == tag)
        {
            result.push_back(it.second);
        }
    }
    return result;
}

PointLight* Scene::AddPointLight(PointLight newLight)
{
    PointLight* newLightPtr = new PointLight(newLight);
    m_PointLights.push_back(newLightPtr);

#ifdef USE_EDITOR
    m_EditorClickables.push_back(newLightPtr);
#endif
    
    return m_PointLights.back();
}

void Scene::DeletePointLight(PointLight* light)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_EditorClickables.begin(), m_EditorClickables.end(), light);
    if (clickableIt != m_EditorClickables.end())
    {
        m_EditorClickables.erase(clickableIt);
    }
#endif

    auto it = std::find(m_PointLights.begin(), m_PointLights.end(), light);
    if (it != m_PointLights.end())
    {
        m_PointLights.erase(it);

        delete light;
    }
}

Brush* Scene::AddBrush(Brush* newBrush)
{
    m_Brushes.push_back(newBrush);
    return m_Brushes.back();
}

void Scene::DeleteBrush(Brush* brush)
{
    auto it = std::find(m_Brushes.begin(), m_Brushes.end(), brush);
    if (it != m_Brushes.end())
    {
        m_Brushes.erase(it);

        BehaviourRegistry::Get()->ClearBehavioursOnEntity(brush->RepModel);

        delete brush->RepModel;
        delete brush;
    }
}

void Scene::AddHalfEdgeMesh(he::HalfEdgeMesh* newMesh)
{
#ifdef USE_EDITOR
    m_EditorClickables.push_back(newMesh);
#endif

    m_HEMeshes.push_back(newMesh);
}

void Scene::DeleteHalfEdgeMesh(he::HalfEdgeMesh* mesh)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_EditorClickables.begin(), m_EditorClickables.end(), mesh);
    if (clickableIt != m_EditorClickables.end())
    {
        m_EditorClickables.erase(clickableIt);
    }
#endif
    auto it = std::find(m_HEMeshes.begin(), m_HEMeshes.end(), mesh);
    if (it != m_HEMeshes.end())
    {
        m_HEMeshes.erase(it);

        delete mesh;
    }
}

std::vector<IEditorClickable*>& Scene::GetEditorClickables()
{
    return m_EditorClickables;
}

std::vector<PointLight*>& Scene::GetPointLights()
{
    return m_PointLights;
}

std::vector<Brush*>& Scene::GetBrushes()
{
    return m_Brushes;
}

void Scene::DeleteModel(Model* model)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_EditorClickables.begin(), m_EditorClickables.end(), model);
    if (clickableIt != m_EditorClickables.end())
    {
        m_EditorClickables.erase(clickableIt);
    }
#endif

    auto it = std::find_if(std::begin(m_Models), std::end(m_Models),
        [&model](auto&& p) { return p.second == model; });

    if (it != std::end(m_Models))
    {
        m_Models.erase(it);

        BehaviourRegistry::Get()->ClearBehavioursOnEntity(model);
        m_ModelIDGenerator.FreeID(it->first);

        delete model;
    }
}

void Scene::AddCamera(Camera* camera)
{
    m_Cameras.push_back(Camera(*camera));

    // If in editor

}

Camera* Scene::GetCamera(size_t index)
{
    if (index < m_Cameras.size())
    {
        return &m_Cameras[index];
    }

    return nullptr;
}

void Scene::SetCamera(Camera* camera)
{
    m_Cameras[0] = Camera(*camera);
}

void Scene::Initialize()
{
    InitializeBehaviours();
}

void Scene::InitializeBehaviours()
{
    BehaviourRegistry* Registry = BehaviourRegistry::Get();
    for (auto it : m_Models)
    {
        Registry->InitializeModelBehaviours(it.second, this);
    }
}

void Scene::Update(double DeltaTime)
{
    UpdateBehaviours(DeltaTime);
}

void Scene::UpdateBehaviours(double DeltaTime)
{
    BehaviourRegistry* Registry = BehaviourRegistry::Get();
    for (auto it : m_Models)
    {
        Registry->UpdateModelBehaviours(it.second, this, DeltaTime);
    }
}

void Scene::Draw(GraphicsModule& graphics, GBuffer gBuffer, size_t camIndex)
{
    PushSceneRenderCommandsInternal(graphics);

    assert(camIndex < m_Cameras.size());

    graphics.Render(gBuffer, m_Cameras[camIndex], m_DirLight);

}

void Scene::EditorDraw(GraphicsModule& graphics, GBuffer gBuffer, Camera* editorCam)
{
    PushSceneRenderCommandsInternal(graphics);

    assert(editorCam);
    for (PointLight* Light : m_PointLights)
    {
        BillboardRenderCommand BillboardRC;
        BillboardRC.m_Colour = Light->colour;
        BillboardRC.m_Position = Light->position;
        BillboardRC.m_Texture = LightBillboardTexture->Id;
        BillboardRC.m_Size = 0.5f;

        graphics.AddRenderCommand(BillboardRC);
    }
    for (Camera& Cam : m_Cameras)
    {
        StaticMeshRenderCommand CamRC;
        CamRC.m_Mesh = CameraMesh->Id;
        CamRC.m_Material = *CameraMaterial;
        
        Transform CamTrans;
        CamTrans.SetTransformMatrix(Cam.GetCamTransMatrix());

        // *vomits on the floor*
        Quaternion CamRot = Quaternion(Vec3f(0.0f, 0.0f, 1.0f), (float)M_PI);
        CamRot = CamRot * Quaternion(Vec3f(1.0f, 0.0f, 0.0f), (float)M_PI_2);

        CamTrans.Rotate(CamRot);
        CamTrans.SetScale(0.1f);

        CamRC.m_TransMat = CamTrans.GetTransformMatrix();

        graphics.AddRenderCommand(CamRC);
    }

    graphics.Render(gBuffer, *editorCam, m_DirLight);
}

void Scene::SetDirectionalLight(DirectionalLight light)
{
    m_DirLight = light;
}

SceneRayCastHit Scene::RayCast(Ray ray, std::vector<Model*> IgnoredModels)
{
    CollisionModule& Collision = *CollisionModule::Get();

    SceneRayCastHit finalHit;

    for (auto it : m_Models)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it.second) > 0)
        {
            continue;
        }
        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it.second->m_TexturedMeshes[0].m_Mesh);
        
        finalHit = Closer(finalHit, SceneRayCastHit{ Collision.RayCast(ray, colMesh, it.second->GetTransform()), it.second });
    }

    return finalHit;
}

Intersection Scene::SphereIntersect(Sphere sphere, std::vector<Model*> IgnoredModels)
{
    CollisionModule& Collision = *CollisionModule::Get();

    Intersection Result;

    for (auto it : m_Models)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it.second) > 0)
        {
            continue;
        }

        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it.second->m_TexturedMeshes[0].m_Mesh);

        Intersection ModelIntersection = Collision.SphereIntersection(sphere, colMesh, it.second->GetTransform());

        if (ModelIntersection.hit && ModelIntersection.penetrationDepth > Result.penetrationDepth)
        {
            Result = ModelIntersection;
        }
    }

    for (auto& it : m_Brushes)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it->RepModel) > 0)
        {
            continue;
        }

        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it->RepModel->m_TexturedMeshes[0].m_Mesh);

        Intersection ModelIntersection = Collision.SphereIntersection(sphere, colMesh, it->RepModel->GetTransform());

        if (ModelIntersection.hit && ModelIntersection.penetrationDepth > Result.penetrationDepth)
        {
            Result = ModelIntersection;
        }
    }

    return Result;
}

Model* Scene::MenuListEntities(UIModule& ui, Font& font)
{
    Vec2f cursor = Vec2f(0.0f, 0.0f);

    Model* result = nullptr;
    for (auto it : m_Models)
    {
        Model* model = it.second;
        Vec3f pos = model->GetTransform().GetPosition();

        std::string modelDesc = model->m_Name.empty() ? "<unnamed>" : model->m_Name;

        Rect rect;
        rect.location = cursor;
        rect.size = Vec2f(160.0f, 20.0f);

        if (ui.TextButton(modelDesc, rect.size, 5.0f))
        {
            result = model;
        }
        cursor.y += 20.0f;
    }

    return result;
}

void Scene::DrawSettingsPanel()
{
    UIModule* UI = UIModule::Get();

    // Directional light colour setting
    Colour dirLightColour = m_DirLight.colour;
    Colour invDirlightColour = Colour(1.0f - dirLightColour.r, 1.0f - dirLightColour.g, 1.0f - dirLightColour.b);
    UI->TextButton("Directional Light Colour", Vec2f(250.0f, 20.0f), 8.0f, dirLightColour, invDirlightColour);

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_DirLight.colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_DirLight.colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_DirLight.colour.b);


}

void Scene::Save(std::string FileName)
{
    std::ofstream File(FileName, std::ofstream::out | std::ofstream::trunc);

    if (!File.is_open())
    {
        Engine::DEBUGPrint("Failed to save scene :(");
        return;
    }

    // Set of all textures used in the scene
    std::set<Material> Materials;
    // Set of all Static Meshes used in the scene
    std::set<StaticMesh> StaticMeshes;

    for (auto it : m_Models)
    {
        Model* model = it.second;

        Texture tex = model->m_TexturedMeshes[0].m_Material.m_Albedo;
        StaticMesh mesh = model->m_TexturedMeshes[0].m_Mesh;
        
        Material mat = model->m_TexturedMeshes[0].m_Material;

        Materials.insert(mat);
        if (mesh.LoadedFromFile)
        {
            StaticMeshes.insert(model->m_TexturedMeshes[0].m_Mesh);
        }
    }
    for (auto& it : m_Brushes)
    {
        Material mat = it->RepModel->m_TexturedMeshes[0].m_Material;

        Materials.insert(mat);
    }

    std::vector<Material> MatVec(Materials.begin(), Materials.end());
    std::vector<StaticMesh> MeshVec(StaticMeshes.begin(), StaticMeshes.end());

    json SceneJson;

    json TextureList;
    json StaticMeshList;
    json PointLightList;
    json DirLightList;
    json ModelList;
    json BrushList;

    int Index = 0;
    for (Material Mat : MatVec)
    {
        SaveMaterial(TextureList[Index++], Mat);
    }
    Index = 0;
    for (StaticMesh Mesh : MeshVec)
    {
        if (!Mesh.LoadedFromFile)
            continue;

        SaveStaticMesh(StaticMeshList[Index++], Mesh);
    }
    Index = 0;
    for (PointLight* PLight : m_PointLights)
    {
        SavePointLight(PointLightList[Index++], *PLight);
    }
    
    // Saving only 1 directional light for now
    SaveDirectionalLight(DirLightList[0], m_DirLight);

    Index = 0;
    for (auto it : m_Models)
    {
        Model* model = it.second;

        int64_t StaticMeshIndex = 0;
        bool GeneratedMesh = false;

        auto MeshIt = std::find(MeshVec.begin(), MeshVec.end(), model->m_TexturedMeshes[0].m_Mesh);
        if (MeshIt != MeshVec.end())
        {
            StaticMeshIndex = MeshIt - MeshVec.begin();
        }
        else
        {
            GeneratedMesh = true;
        }
        
        int64_t MaterialIndex = 0;

        auto MatIt = std::find(MatVec.begin(), MatVec.end(), model->m_TexturedMeshes[0].m_Material);
        if (MatIt != MatVec.end())
        {
            MaterialIndex = MatIt - MatVec.begin();
        }
        else
        {
            Engine::FatalError("Could not find material, this should never happen");
        }

        if (GeneratedMesh)
        {
            SaveRawModel(ModelList[Index++], *model, MaterialIndex);
        }
        else
        {
            SaveModel(ModelList[Index++], *model, StaticMeshIndex, MaterialIndex);
        }
    }
    Index = 0;
    for (Brush* B : m_Brushes)
    {
        int64_t MaterialIndex = 0;

        auto MatIt = std::find(MatVec.begin(), MatVec.end(), B->RepModel->m_TexturedMeshes[0].m_Material);
        if (MatIt != MatVec.end())
        {
            MaterialIndex = MatIt - MatVec.begin();
        }
        else
        {
            Engine::FatalError("Could not find material while saving brush, this should never happen");
        }

        SaveBrush(BrushList[Index++], *B, MaterialIndex);
    }

    SceneJson["Textures"] = TextureList;
    SceneJson["StaticMeshes"] = StaticMeshList;
    SceneJson["PointLights"] = PointLightList;
    SceneJson["DirLights"] = DirLightList;
    SceneJson["Models"] = ModelList;
    SceneJson["Brushes"] = BrushList;

    // Uncomment to beautify json - makes it easier to debug but makes files much larger
    File << std::setw(4) << SceneJson;

    // Uncomment to normalness
    //File << SceneJson;

    File.close();
}

void Scene::Load(std::string FileName)
{
    std::ifstream File(FileName);

    if (!File.is_open())
    {
        Engine::Alert("Failed to open level " + FileName);
        return;
    }

    Clear();

    if (!json::accept(File))
    {
        LegacyLoad(FileName);
        return;
    }

    File.clear();
    File.seekg(0, std::ios::beg);

    json SceneJson = json::parse(File);

    File.close();

    std::vector<Material> MaterialVec;
    std::vector<StaticMesh> StaticMeshVec;

    json TexturesJson = SceneJson["Textures"];
    json StaticMeshesJson = SceneJson["StaticMeshes"];
    json PointLightsJson = SceneJson["PointLights"];
    json DirLightsJson = SceneJson["DirLights"];
    json ModelsJson = SceneJson["Models"];
    json BrushesJson = SceneJson["Brushes"];

    for (json& TextureJson : TexturesJson)
    {
        MaterialVec.push_back(LoadMaterial(TextureJson));
    }
    for (json& StaticMeshJson : StaticMeshesJson)
    {
        StaticMeshVec.push_back(LoadStaticMesh(StaticMeshJson));
    }
    for (json& PointLightJson : PointLightsJson)
    {
        AddPointLight(LoadPointLight(PointLightJson));
    }
    for (json& DirLightJson : DirLightsJson)
    {
        // Only 1 directional light for now
        SetDirectionalLight(LoadDirectionalLight(DirLightJson));
    }
    for (json& ModelJson : ModelsJson)
    {
        Model* AddedModel;
        if (ModelJson.contains("Buffer"))
        {
            AddedModel = AddModel(LoadRawModel(ModelJson, MaterialVec));
        }
        else
        {
            AddedModel = AddModel(LoadModel(ModelJson, MaterialVec, StaticMeshVec));
        }
    }
    
    for (json& BrushJson : BrushesJson)
    {
        // TODO: Brushes can't have behaviours (for now anyway)

        Brush* AddedBrush;
        AddedBrush = AddBrush(LoadBrush(BrushJson, MaterialVec));
    }

}

//void Scene::Save(std::string FileName)
//{   
//    // Set of all textures used in the scene (TODO: replace with Materials)
//    std::set<Material> Materials;
//    // Set of all Static Meshes used in the scene
//    std::set<StaticMesh> StaticMeshes;
//
//    for (auto& it : m_UntrackedModels)
//    {
//        Texture tex = it->m_TexturedMeshes[0].m_Material.m_Albedo;
//        StaticMesh mesh = it->m_TexturedMeshes[0].m_Mesh;
//        
//        Material mat = it->m_TexturedMeshes[0].m_Material;
//        //if (tex.LoadedFromFile)
//        //{
//        //  Materials.insert(mat);
//        //}
//        Materials.insert(mat);
//        if (mesh.LoadedFromFile)
//        {
//            StaticMeshes.insert(it->m_TexturedMeshes[0].m_Mesh);
//        }
//    }
//
//    std::vector<Material> MaterialVec(Materials.begin(), Materials.end());
//    std::vector<StaticMesh> StaticMeshVec(StaticMeshes.begin(), StaticMeshes.end());
//
//
//    std::ofstream File(FileName);
//
//    if (!File.is_open())
//    {
//        Engine::DEBUGPrint("Failed to save scene :(");
//        return;
//    }
//
//    File << "Textures:" << std::endl;
//    for (auto& Mat : MaterialVec)
//    {
//        std::string Path = Mat.m_Albedo.Path.GetFullPath();
//        std::string NormPath = Mat.m_Normal.Path.GetFullPath();
//        std::string RoughPath = Mat.m_Roughness.Path.GetFullPath();
//        std::string MetalPath = Mat.m_Metallic.Path.GetFullPath();
//        std::string AOPath = Mat.m_AO.Path.GetFullPath();
//
//        File << Path << Separator << NormPath << Separator << RoughPath << Separator << MetalPath << Separator << AOPath << std::endl;
//    }
//
//    File << "StaticMeshes:" << std::endl;
//    for (auto& Mesh : StaticMeshVec)
//    {
//        std::string Path = Mesh.Path.GetFullPath();
//        File << Path << std::endl;
//    }
//
//    File << "Entities:" << std::endl;
//
//    CollisionModule* Collisions = CollisionModule::Get();
//
//    for (auto& it : m_UntrackedModels)
//    {
//        int64_t StaticMeshIndex = 0;
//        int64_t TextureIndex = 0;
//
//        bool GeneratedMesh = false;
//
//        auto MeshIt = std::find(StaticMeshVec.begin(), StaticMeshVec.end(), it->m_TexturedMeshes[0].m_Mesh);
//        if (MeshIt != StaticMeshVec.end())
//        {
//            StaticMeshIndex = MeshIt - StaticMeshVec.begin();
//        }
//        else
//        {
//            GeneratedMesh = true;
//            //Engine::FatalError("Could not find mesh while saving scene (this should never happen)");
//        }
//
//        auto MaterialIt = std::find(MaterialVec.begin(), MaterialVec.end(), it->m_TexturedMeshes[0].m_Material);
//        if (MaterialIt != MaterialVec.end())
//        {
//            TextureIndex = MaterialIt - MaterialVec.begin();
//        }
//        else
//        {
//            Engine::FatalError("Could not find texture while saving scene (this should never happen)");
//        }
//
//        File << TextureIndex << Separator;
//
//        if (!GeneratedMesh)
//        {
//            File << StaticMeshIndex << Separator;
//        }
//        else
//        {
//            File << "B" << Separator;
//            // For now, all generated meshes are boxes, so store the AABB
//            AABB BoxAABB = Collisions->GetCollisionMeshFromMesh(it->m_TexturedMeshes[0].m_Mesh)->boundingBox;
//
//            File << BoxAABB.min.x << Separator << BoxAABB.min.y << Separator << BoxAABB.min.z << Separator;
//            File << BoxAABB.max.x << Separator << BoxAABB.max.y << Separator << BoxAABB.max.z << Separator;
//        }
//
//        Mat4x4f TransMat = it->GetTransform().GetTransformMatrix();
//
//        for (int i = 0; i < 4; ++i)
//        {
//            File << TransMat[i].x << Separator << TransMat[i].y << Separator << TransMat[i].z << Separator << TransMat[i].w << Separator;
//        }
//
//        std::vector<std::string> BehaviourNames = BehaviourRegistry::Get()->GetBehavioursAttachedToEntity(it);
//
//        for (std::string BehaviourName : BehaviourNames)
//        {
//            File << BehaviourName << Separator;
//        }
//
//        File << std::endl;
//
//    }
//
//    File.close();
//
//}

void Scene::LegacyLoad(std::string FileName)
{
    AssetRegistry* Registry = AssetRegistry::Get();

    std::ifstream File(FileName);

    if (!File.is_open())
    {
        return;
    }

    GraphicsModule* Graphics = GraphicsModule::Get();

    Clear();

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
                Texture DiffuseTex = *Registry->LoadTexture(LineTokens[0]);
                Texture NormalTex = *Registry->LoadTexture(LineTokens[1]);
                Texture RoughnessTex = *Registry->LoadTexture(LineTokens[2]);
                Texture MetallicTex = *Registry->LoadTexture(LineTokens[3]);
                Texture AOTex = *Registry->LoadTexture(LineTokens[4]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex, MetallicTex, AOTex));
            }
            else if (LineTokens.size() == 4)
            {
                Texture DiffuseTex = *Registry->LoadTexture(LineTokens[0]);
                Texture NormalTex = *Registry->LoadTexture(LineTokens[1]);
                Texture RoughnessTex = *Registry->LoadTexture(LineTokens[2]);
                Texture MetallicTex = *Registry->LoadTexture(LineTokens[3]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex, MetallicTex));
            }
            else if (LineTokens.size() == 3)
            {
                Texture DiffuseTex = *Registry->LoadTexture(LineTokens[0]);
                Texture NormalTex = *Registry->LoadTexture(LineTokens[1]);
                Texture RoughnessTex = *Registry->LoadTexture(LineTokens[2]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex));
            }
            else if (LineTokens.size() == 2)
            {
                Texture DiffuseTex = *Registry->LoadTexture(LineTokens[0]);
                Texture NormalTex = *Registry->LoadTexture(LineTokens[1]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex));
            }
            else
            {
                Texture DiffuseTex = *Registry->LoadTexture(LineTokens[0]);
                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex));
            }
            break;
        case STATIC_MESHES:
            SceneStaticMeshes.push_back(*Registry->LoadStaticMesh(LineTokens[0]));
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

                NewModel = new Model(Graphics->CreateBoxModel(BoxAABB, SceneMaterials[MaterialIndex]));
            }
            else
            {
                int StaticMeshIndex = std::stoi(LineTokens[1]);
                NewModel = new Model(Graphics->CreateModel(TexturedMesh(SceneStaticMeshes[StaticMeshIndex], SceneMaterials[MaterialIndex])));
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

            GUID newModelID = m_ModelIDGenerator.Generate();

            m_Models[newModelID] = NewModel;

            break;
        }
        default:
            break;
        }
    }

}

void Scene::Clear()
{
    m_ModelIDGenerator.Reset();

    for (auto it : m_Models)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(it.second);
        delete it.second;
    }
    for (auto& PointLight : m_PointLights)
    {
        delete PointLight;
    }
    for (auto& B : m_Brushes)
    {
        if (B->RepModel)
        {
            delete B->RepModel;
        }
        delete B;
    }

    m_Models.clear();
    m_PointLights.clear();
    m_Cameras.clear();
    m_Brushes.clear();

#ifdef USE_EDITOR
    m_EditorClickables.clear();
#endif

    // Set camera to default TODO: (want to load camera info from file)
    m_Cameras.push_back(Camera());
}

void Scene::CopyInternal(const Scene& other)
{
    Clear();
    m_Cameras.resize(1);

    m_DirLight = other.m_DirLight;
    m_Cameras = other.m_Cameras;

    for (auto it : other.m_Models)
    {
        Model* newModel = new Model(*it.second);
        AddModel(newModel);

        // TODO: Copy actual behaviour so parameter changes are picked up

        Behaviour* oldBehaviour = BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(it.second);
        if (oldBehaviour)
        {
            BehaviourRegistry::Get()->AttachNewBehaviour(oldBehaviour->BehaviourName, newModel);
        }
    }

    for (auto& brush : other.m_Brushes)
    {
        // Reminder to self: copied brush needs its own rep model ptr

        // (TODO): BIGGER REMINDER TO SELF: COPYING A 2D VECTOR OF VEC3FS IN THE FACE ARRAY NEEDS ANOTHER LOOK 
        // (THEY'RE STILL POINTING TO THE INITIAL VEC3F VECTOR ELEMENTS!!!!)

        Brush* newBrush = new Brush(*brush);

        newBrush->RepModel = nullptr;

        GraphicsModule::Get()->UpdateBrushModel(newBrush);

        AddBrush(newBrush);

        Behaviour* oldBehaviour = BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(brush->RepModel);
        if (oldBehaviour)
        {
            BehaviourRegistry::Get()->AttachNewBehaviour(oldBehaviour->BehaviourName, brush->RepModel);
        }
    }

    for (auto& pointLight : other.m_PointLights)
    {
        AddPointLight(*pointLight);
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

void Scene::PushSceneRenderCommandsInternal(GraphicsModule& graphics)
{
    for (auto& heMesh : m_HEMeshes)
    {
        heMesh->Draw();
    }

    for (auto it : m_Models)
    {
        StaticMeshRenderCommand command;
        command.m_Material = it.second->m_TexturedMeshes[0].m_Material;
        command.m_Mesh = it.second->m_TexturedMeshes[0].m_Mesh.Id;
        command.m_TransMat = it.second->GetTransform().GetTransformMatrix();

        graphics.AddRenderCommand(command);
    }

    for (auto& it : m_Brushes)
    {
        if (!it->RepModel)
        {
            graphics.UpdateBrushModel(it);
        }

        Model* repModel = it->RepModel;
        
        StaticMeshRenderCommand command;
        command.m_Material = repModel->m_TexturedMeshes[0].m_Material;
        command.m_Mesh = repModel->m_TexturedMeshes[0].m_Mesh.Id;
        command.m_TransMat = repModel->GetTransform().GetTransformMatrix();

        graphics.AddRenderCommand(command);
    }

    for (PointLight* Light : m_PointLights)
    {
        PointLightRenderCommand LightRC;
        LightRC.m_Colour = Light->colour;
        LightRC.m_Position = Light->position;
        LightRC.m_Intensity = Light->intensity;

        graphics.AddRenderCommand(LightRC);
    }
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

void Scene::SaveMaterial(json& JsonObject, Material& Mat)
{
    JsonObject[0] = Mat.m_Albedo.Path.GetFullPath();
    JsonObject[1] = Mat.m_Normal.Path.GetFullPath();
    JsonObject[2] = Mat.m_Roughness.Path.GetFullPath();
    JsonObject[3] = Mat.m_Metallic.Path.GetFullPath();
    JsonObject[4] = Mat.m_AO.Path.GetFullPath();
}

void Scene::SaveStaticMesh(json& JsonObject, StaticMesh& Mesh)
{
    JsonObject = { Mesh.Path.GetFullPath() };
}

void Scene::SavePointLight(json& JsonObject, PointLight& PointLight)
{
    JsonObject["Position"] = { PointLight.position.x, PointLight.position.y, PointLight.position.z };
    JsonObject["Colour"] = { PointLight.colour.x, PointLight.colour.y, PointLight.colour.z };
    JsonObject["Intensity"] = PointLight.intensity;
}

void Scene::SaveDirectionalLight(json& JsonObject, DirectionalLight& DirLight)
{
    JsonObject["Direction"] = { DirLight.direction.x, DirLight.direction.y, DirLight.direction.z };
    JsonObject["Colour"] = { DirLight.colour.x, DirLight.colour.y, DirLight.colour.z };
}

void Scene::SaveModel(json& JsonObject, Model& Mod, int64_t MeshIndex, int64_t MatIndex)
{
    Mat4x4f ModTrans = Mod.GetTransform().GetTransformMatrix();

    JsonObject["MeshID"] = MeshIndex;
    JsonObject["MatID"] = MatIndex;
    JsonObject["Transform"] = { 
        ModTrans[0].x, ModTrans[0].y, ModTrans[0].z, ModTrans[0].w,
        ModTrans[1].x, ModTrans[1].y, ModTrans[1].z, ModTrans[1].w,
        ModTrans[2].x, ModTrans[2].y, ModTrans[2].z, ModTrans[2].w,
        ModTrans[3].x, ModTrans[3].y, ModTrans[3].z, ModTrans[3].w,
    };

    std::vector<std::string> Behaviours = BehaviourRegistry::Get()->GetBehavioursAttachedToEntity(&Mod);

    JsonObject["Behaviours"] = Behaviours;
    JsonObject["Type"] = Mod.Type;

}

void Scene::SaveRawModel(json& JsonObject, Model& Mod, int64_t MatIndex)
{
    Mat4x4f ModTrans = Mod.GetTransform().GetTransformMatrix();
    JsonObject["MatID"] = MatIndex;
    JsonObject["Transform"] = {
        ModTrans[0].x, ModTrans[0].y, ModTrans[0].z, ModTrans[0].w,
        ModTrans[1].x, ModTrans[1].y, ModTrans[1].z, ModTrans[1].w,
        ModTrans[2].x, ModTrans[2].y, ModTrans[2].z, ModTrans[2].w,
        ModTrans[3].x, ModTrans[3].y, ModTrans[3].z, ModTrans[3].w,
    };

    GraphicsModule* Graphics = GraphicsModule::Get();

    std::vector<float> VertBuf = Graphics->GetModelVertexBuffer(Mod);
    std::vector<unsigned int> IndexBuf = Graphics->GetModelIndexBuffer(Mod);

    JsonObject["Buffer"][0] = VertBuf;
    JsonObject["Buffer"][1] = IndexBuf;

    std::vector<std::string> Behaviours = BehaviourRegistry::Get()->GetBehavioursAttachedToEntity(&Mod);

    JsonObject["Behaviours"] = Behaviours;

    JsonObject["Type"] = Mod.Type;
}

void Scene::SaveBrush(json& JsonObject, Brush& B, int64_t MatIndex)
{
    JsonObject["MatID"] = MatIndex;

    std::vector<float> VertVec;

    for (auto& Vert : B.Vertices)
    {
        VertVec.insert(VertVec.end(),
            {
                Vert.x, Vert.y, Vert.z
            }
        );
    }

    JsonObject["Verts"] = VertVec;
    
    for (int i = 0; i < B.Faces.size(); i++)
    {
        JsonObject["Faces"][i] = B.Faces[i];
    }
}

Material Scene::LoadMaterial(json& JsonObject)
{
    std::string albedoPath = JsonObject[0].get<std::string>();
    std::string normalPath = JsonObject[1].get<std::string>();
    std::string roughPath = JsonObject[2].get<std::string>();
    std::string metalPath = JsonObject[3].get<std::string>();
    std::string aoPath = JsonObject[4].get<std::string>();

    if (albedoPath.find("Assets") == std::string::npos) albedoPath = "Assets/" + albedoPath;
    if (normalPath.find("Assets") == std::string::npos) normalPath = "Assets/" + normalPath;
    if (roughPath.find("Assets") == std::string::npos) roughPath = "Assets/" + roughPath;
    if (metalPath.find("Assets") == std::string::npos) metalPath = "Assets/" + metalPath;
    if (aoPath.find("Assets") == std::string::npos) aoPath = "Assets/" + aoPath;

    Texture* Albedo = AssetRegistry::Get()->LoadTexture(albedoPath);
    Texture* Normal = AssetRegistry::Get()->LoadTexture(normalPath);
    Texture* Roughness = AssetRegistry::Get()->LoadTexture(roughPath);
    Texture* Metallic = AssetRegistry::Get()->LoadTexture(metalPath);
    Texture* AO = AssetRegistry::Get()->LoadTexture(aoPath);

    return Material(*Albedo, *Normal, *Roughness, *Metallic, *AO);
}

StaticMesh Scene::LoadStaticMesh(json& JsonObject)
{
    std::string path = JsonObject[0].get<std::string>();

    if (path.find("Assets") == std::string::npos) path = "Assets/" + path;

    return *AssetRegistry::Get()->LoadStaticMesh(path);
}

PointLight Scene::LoadPointLight(json& JsonObject)
{
    auto Pos = JsonObject["Position"];
    auto Colour = JsonObject["Colour"];

    Vec3f VecPos = Vec3f(Pos[0], Pos[1], Pos[2]);
    Vec3f VecColour = Vec3f(Colour[0], Colour[1], Colour[2]);
    
    float Intensity = JsonObject["Intensity"];

    PointLight PLight;
    PLight.position = VecPos;
    PLight.colour = VecColour;
    PLight.intensity = Intensity;

    return PLight;
}

DirectionalLight Scene::LoadDirectionalLight(json& JsonObject)
{
    auto Dir = JsonObject["Direction"];
    auto Clr = JsonObject["Colour"];

    Vec3f VecDir = Vec3f(Dir[0], Dir[1], Dir[2]);
    Vec3f VecColour = Colour(Clr[0], Clr[1], Clr[2]);

    DirectionalLight DLight;
    DLight.direction = VecDir;
    DLight.colour = VecColour;

    return DLight;
}

Model* Scene::LoadModel(json& JsonObject, std::vector<Material>& MaterialVector, std::vector<StaticMesh>& StaticMeshVector)
{
    Material ModelMat = MaterialVector[JsonObject["MatID"]];
    StaticMesh ModelMesh = StaticMeshVector[JsonObject["MeshID"]];

    auto Trans = JsonObject["Transform"];

    Mat4x4f TransMat;
    TransMat[0] = { Trans[0], Trans[1], Trans[2], Trans[3]};
    TransMat[1] = { Trans[4], Trans[5], Trans[6], Trans[7] };
    TransMat[2] = { Trans[8], Trans[9], Trans[10], Trans[11] };
    TransMat[3] = { Trans[12], Trans[13], Trans[14], Trans[15] };

    Model* NewModel = new Model(TexturedMesh(ModelMesh, ModelMat));
    NewModel->GetTransform().SetTransformMatrix(TransMat);

    for (std::string Behaviour : JsonObject["Behaviours"])
    {
        BehaviourRegistry::Get()->AttachNewBehaviour(Behaviour, NewModel);
    }

    if (JsonObject.contains("Type"))
    {
        NewModel->Type = JsonObject["Type"];
    }

    return NewModel;
}

Model* Scene::LoadRawModel(json& JsonObject, std::vector<Material>& MaterialVector)
{
    Material ModelMat = MaterialVector[JsonObject["MatID"]];
    std::vector<float> MeshBuffer = JsonObject["Buffer"][0];
    std::vector<unsigned int> IndexBuffer = JsonObject["Buffer"][1];

    auto Trans = JsonObject["Transform"];

    Mat4x4f TransMat;
    TransMat[0] = { Trans[0], Trans[1], Trans[2], Trans[3] };
    TransMat[1] = { Trans[4], Trans[5], Trans[6], Trans[7] };
    TransMat[2] = { Trans[8], Trans[9], Trans[10], Trans[11] };
    TransMat[3] = { Trans[12], Trans[13], Trans[14], Trans[15] };

    GraphicsModule* Graphics = GraphicsModule::Get();

    Model* NewModel = new Model(Graphics->LoadModel(MeshBuffer, IndexBuffer, ModelMat));

    NewModel->GetTransform().SetTransformMatrix(TransMat);

    for (std::string Behaviour : JsonObject["Behaviours"])
    {
        BehaviourRegistry::Get()->AttachNewBehaviour(Behaviour, NewModel);
    }

    if (JsonObject.contains("Type"))
    {
        NewModel->Type = JsonObject["Type"];
    }

    return NewModel;
}

Brush* Scene::LoadBrush(json& JsonObject, std::vector<Material>& MaterialVector)
{
    std::vector<float> ReadVerts = JsonObject["Verts"];

    // I'll see if this works...
    std::vector<std::vector<unsigned int>> ReadFaces = JsonObject["Faces"];


    std::vector<Vec3f> Vertices;

    for (int i = 0; i < ReadVerts.size(); i += 3)
    {
        Vertices.emplace_back(ReadVerts[i], ReadVerts[i + 1], ReadVerts[i + 2]);
    }

    GraphicsModule* Graphics = GraphicsModule::Get();
    
    Brush* NewBrush = new Brush(Vertices, ReadFaces);

    Graphics->UpdateBrushModel(NewBrush);

    return NewBrush;
}
