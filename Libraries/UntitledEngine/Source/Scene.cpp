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

#ifdef USE_EDITOR
Texture* Scene::LightBillboardTexture = nullptr;
StaticMesh* Scene::CameraMesh = nullptr;
Material* Scene::CameraMaterial = nullptr;
StaticMesh* Scene::DirectionalLightMesh = nullptr;
Material* Scene::DirectionalLightMaterial = nullptr;
StaticMesh* Scene::SpotLightMesh = nullptr;
Material* Scene::SpotLightMaterial = nullptr;

#endif

SceneRayCastHit Closer(const SceneRayCastHit& lhs, const SceneRayCastHit& rhs)
{
    return (lhs.rayCastHit.hitDistance <= rhs.rayCastHit.hitDistance ? lhs : rhs);
}

Scene::Scene()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    m_Cameras.resize(1);

    if (!Graphics)
    {
        return;
    }

#ifdef USE_EDITOR
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
        CameraMaterial = new Material(Graphics->CreateMaterial((Registry->LoadTexture("Assets/textures/Camera.png"))));
    }
    if (!DirectionalLightMesh)
    {
        DirectionalLightMesh = Registry->LoadStaticMesh("Assets/models/DirLightBall.obj");
    }
    if (!DirectionalLightMaterial)
    {
        DirectionalLightMaterial = new Material(Graphics->CreateMaterial((Registry->LoadTexture("Assets/textures/whiteTexture.png"))));
    }
    if (!SpotLightMesh)
    {
        SpotLightMesh = Registry->LoadStaticMesh("Assets/models/SpotLight.obj");
    }
    if (!SpotLightMaterial)
    {
        SpotLightMaterial = new Material(Graphics->CreateMaterial((Registry->LoadTexture("Assets/textures/whiteTexture.png"))));
    }
#endif

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
        m_ModelIDGenerator.FreeID(it.first);
        
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(it.second);
        delete it.second;
    }
    m_Models.clear();
    m_PointLights.clear();
    m_DirectionalLights.clear();
    m_SpotLights.clear();
    m_HEMeshes.clear();
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
    uGUID newModelID = m_ModelIDGenerator.Generate();

    m_Models[newModelID] = model;

#ifdef USE_EDITOR
    m_GenericEditorClickables.push_back(model);
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
    m_GenericEditorClickables.push_back(newLightPtr);
#endif
    
    return m_PointLights.back();
}

void Scene::DeletePointLight(PointLight* light)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_GenericEditorClickables.begin(), m_GenericEditorClickables.end(), light);
    if (clickableIt != m_GenericEditorClickables.end())
    {
        m_GenericEditorClickables.erase(clickableIt);
    }
#endif

    auto it = std::find(m_PointLights.begin(), m_PointLights.end(), light);
    if (it != m_PointLights.end())
    {
        m_PointLights.erase(it);

        delete light;
    }
}

SpotLight* Scene::AddSpotLight(SpotLight newLight)
{
    SpotLight* newLightPtr = new SpotLight(newLight);
    m_SpotLights.push_back(newLightPtr);

#ifdef USE_EDITOR
    m_GenericEditorClickables.push_back(newLightPtr);
#endif

    return m_SpotLights.back();
}

void Scene::DeleteSpotLight(SpotLight* light)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_GenericEditorClickables.begin(), m_GenericEditorClickables.end(), light);
    if (clickableIt != m_GenericEditorClickables.end())
    {
        m_GenericEditorClickables.erase(clickableIt);
    }
#endif

    auto it = std::find(m_SpotLights.begin(), m_SpotLights.end(), light);
    if (it != m_SpotLights.end())
    {
        m_SpotLights.erase(it);

        delete light;
    }
}

DirectionalLight* Scene::AddDirectionalLight(DirectionalLight newLight)
{
    DirectionalLight* newLightPtr = new DirectionalLight(newLight);
    m_DirectionalLights.push_back(newLightPtr);

#ifdef USE_EDITOR
    m_GenericEditorClickables.push_back(newLightPtr);
#endif

    return m_DirectionalLights.back();
}

void Scene::DeleteDirectionalLight(DirectionalLight* light)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_GenericEditorClickables.begin(), m_GenericEditorClickables.end(), light);
    if (clickableIt != m_GenericEditorClickables.end())
    {
        m_GenericEditorClickables.erase(clickableIt);
    }
#endif

    auto it = std::find(m_DirectionalLights.begin(), m_DirectionalLights.end(), light);
    if (it != m_DirectionalLights.end())
    {
        m_DirectionalLights.erase(it);

        delete light;
    }
}

void Scene::AddHalfEdgeMesh(he::HalfEdgeMesh* newMesh)
{
#ifdef USE_EDITOR
    m_GenericEditorClickables.push_back(newMesh);
#endif
    m_HEMeshes.push_back(newMesh);
}

void Scene::DeleteHalfEdgeMesh(he::HalfEdgeMesh* mesh)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_GenericEditorClickables.begin(), m_GenericEditorClickables.end(), mesh);
    if (clickableIt != m_GenericEditorClickables.end())
    {
        m_GenericEditorClickables.erase(clickableIt);
    }
#endif

    auto it = std::find(m_HEMeshes.begin(), m_HEMeshes.end(), mesh);
    if (it != m_HEMeshes.end())
    {
        m_HEMeshes.erase(it);

        delete mesh;
    }
}

#ifdef USE_EDITOR
std::vector<IEditorClickable*>& Scene::GetGenericEditorClickables()
{
    return m_GenericEditorClickables;
}
#endif

std::vector<PointLight*>& Scene::GetPointLights()
{
    return m_PointLights;
}

std::vector<SpotLight*>& Scene::GetSpotLights()
{
    return m_SpotLights;
}

std::vector<he::HalfEdgeMesh*>& Scene::GetHalfEdgeMeshes()
{
    return m_HEMeshes;
}

void Scene::DeleteModel(Model* model)
{
#ifdef USE_EDITOR
    auto clickableIt = std::find(m_GenericEditorClickables.begin(), m_GenericEditorClickables.end(), model);
    if (clickableIt != m_GenericEditorClickables.end())
    {
        m_GenericEditorClickables.erase(clickableIt);
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

    graphics.Render(gBuffer, m_Cameras[camIndex]);
}

#ifdef USE_EDITOR
void Scene::EditorDraw(GraphicsModule& graphics, GBuffer gBuffer, Camera* editorCam, bool drawSceneCam, bool debugDrawHEMeshes)
{
    if (InputModule::Get()->GetKeyState(Key::C).justPressed)
    {
        debugDrawHEMeshes = !debugDrawHEMeshes;
    }

    PushSceneRenderCommandsInternal(graphics);
    if (debugDrawHEMeshes)
    {
        for (auto& heMesh : m_HEMeshes)
        {
            heMesh->EditorDraw();
        }
    }
    
    assert(editorCam);
    for (PointLight* Light : m_PointLights)
    {
        BillboardRenderCommand BillboardRC;
        BillboardRC.m_Colour = Light->colour;
        BillboardRC.m_Position = Light->position;
        BillboardRC.m_Texture = LightBillboardTexture->GetID();
        BillboardRC.m_Size = 0.5f;

        graphics.AddRenderCommand(BillboardRC);
    }
    for (DirectionalLight* Light : m_DirectionalLights)
    {

        StaticMeshRenderCommand RenderCommand;
        RenderCommand.m_Mesh = DirectionalLightMesh->Id;
        RenderCommand.m_Material = *DirectionalLightMaterial;
        RenderCommand.m_CastShadows = false;

        Transform DirLightTrans;
        DirLightTrans.SetPosition(Light->position);
        Quaternion q = Math::VecDiffToQuat(Light->direction, Vec3f::Up());

        DirLightTrans.SetRotation(q);
        
        RenderCommand.m_TransMat = DirLightTrans.GetTransformMatrix();

        graphics.AddRenderCommand(RenderCommand);
    }
    for (SpotLight* SLight : m_SpotLights)
    {
        StaticMeshRenderCommand RenderCommand;
        RenderCommand.m_Mesh = SpotLightMesh->Id;
        RenderCommand.m_Material = *SpotLightMaterial;
        RenderCommand.m_CastShadows = false;

        Transform SpotLightTrans;
        SpotLightTrans.SetPosition(SLight->position);
        Quaternion q = Math::VecDiffToQuat(SLight->direction, Vec3f::Up());

        SpotLightTrans.SetRotation(q);

        RenderCommand.m_TransMat = SpotLightTrans.GetTransformMatrix();

        graphics.AddRenderCommand(RenderCommand);
    }

    if (drawSceneCam)
    {
        for (Camera& Cam : m_Cameras)
        {
            StaticMeshRenderCommand CamRC;
            CamRC.m_Mesh = CameraMesh->Id;
            CamRC.m_Material = *CameraMaterial;
            CamRC.m_CastShadows = false;

            Transform CamTrans;
            CamTrans.SetTransformMatrix(Cam.GetCamTransMatrix());
            CamTrans.SetScale(0.1f);

            CamRC.m_TransMat = CamTrans.GetTransformMatrix();

            graphics.AddRenderCommand(CamRC);
        }
    }

    graphics.Render(gBuffer, *editorCam);
}
#endif

SceneRayCastHit Scene::RayCast(
    Ray ray, 
    std::vector<Model*> IgnoredModels, 
    std::vector<he::HalfEdgeMesh*> IgnoredHalfEdgeMeshes)
{
    CollisionModule& Collision = *CollisionModule::Get();

    SceneRayCastHit finalHit;

    for (auto it : m_Models)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it.second) > 0)
        {
            continue;
        }
        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it.second->m_StaticMesh);
        
        finalHit = Closer(finalHit, SceneRayCastHit{ Collision.RayCast(ray, colMesh, it.second->GetTransform()), it.second });
    }

    // TODO: Some sort of typed union (or maybe polymorphism if this gets more complex) for returning multiple hit object types
    for (auto& it : m_HEMeshes)
    {
        if (std::count(IgnoredHalfEdgeMeshes.begin(), IgnoredHalfEdgeMeshes.end(), it) > 0)
        {
            continue;
        }
        finalHit = Closer(finalHit, SceneRayCastHit{ Collision.RayCast(ray, *it), nullptr });
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

        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it.second->m_StaticMesh);

        Intersection ModelIntersection = Collision.SphereIntersection(sphere, colMesh, it.second->GetTransform());

        if (ModelIntersection.hit && ModelIntersection.penetrationDepth > Result.penetrationDepth)
        {
            Result = ModelIntersection;
        }
    }
    for (auto& it : m_HEMeshes)
    {
        Intersection HEMeshIntersection = Collision.SphereIntersection(sphere, *it);

        if (HEMeshIntersection.hit && HEMeshIntersection.penetrationDepth > Result.penetrationDepth)
        {
            Result = HEMeshIntersection;
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
    //Colour dirLightColour = m_DirLight.colour;
    //Colour invDirlightColour = Colour(1.0f - dirLightColour.r, 1.0f - dirLightColour.g, 1.0f - dirLightColour.b);
    //UI->TextButton("Directional Light Colour", Vec2f(250.0f, 20.0f), 8.0f, dirLightColour, invDirlightColour);

    //UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_DirLight.colour.r);
    //UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_DirLight.colour.g);
    //UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_DirLight.colour.b);

    // Ambient light colour setting
    UI->Text("Ambient Light:");
    UI->NewLine();
    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_AmbientLight.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_AmbientLight.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_AmbientLight.b);

}

void Scene::Save(std::string FileName)
{
    // Set of all textures used in the scene
    std::set<Material> Materials;
    // Set of all Static Meshes used in the scene
    std::set<StaticMesh> StaticMeshes;

    for (auto& it : m_Models)
    {
        Model* model = it.second;

        //Texture tex = model->m_Material.m_Albedo;
        StaticMesh mesh = model->m_StaticMesh;
        
        Material mat = model->m_Material;

        Materials.insert(mat);
        if (mesh.LoadedFromFile)
        {
            StaticMeshes.insert(model->m_StaticMesh);
        }
    }

    for (auto& it : m_HEMeshes)
    {
        he::HalfEdgeMesh* heMesh = it;

        for (auto& face : heMesh->m_Faces)
        {
            Material mat = face->material;
            Materials.insert(mat);
        }
    }

    std::vector<Material> MatVec(Materials.begin(), Materials.end());
    std::vector<StaticMesh> MeshVec(StaticMeshes.begin(), StaticMeshes.end());

    json SceneJson;

    json TextureList;
    json StaticMeshList;
    json PointLightList;
    json DirLightList;
    json SpotLightList;
    json ModelList;
    json HEMeshList;

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
    Index = 0;
    for (DirectionalLight* DLight : m_DirectionalLights)
    {
        SaveDirectionalLight(DirLightList[Index++], *DLight);
    }
    Index = 0;
    for (SpotLight* SLight : m_SpotLights)
    {
        SaveSpotLight(SpotLightList[Index++], *SLight);
    }
    Index = 0;
    for (auto it : m_Models)
    {
        Model* model = it.second;

        int64_t StaticMeshIndex = 0;
        bool GeneratedMesh = false;

        auto MeshIt = std::find(MeshVec.begin(), MeshVec.end(), model->m_StaticMesh);
        if (MeshIt != MeshVec.end())
        {
            StaticMeshIndex = MeshIt - MeshVec.begin();
        }
        else
        {
            GeneratedMesh = true;
        }
        
        int64_t MaterialIndex = 0;

        auto MatIt = std::find(MatVec.begin(), MatVec.end(), model->m_Material);
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
    for (he::HalfEdgeMesh* heMesh : m_HEMeshes)
    {
        SaveHEMesh(HEMeshList[Index++], heMesh, MatVec);
    }


    SceneJson["Textures"] = TextureList;
    SceneJson["StaticMeshes"] = StaticMeshList;
    SceneJson["PointLights"] = PointLightList;
    SceneJson["DirLights"] = DirLightList;
    SceneJson["SpotLights"] = SpotLightList;
    SceneJson["Models"] = ModelList;
    SceneJson["HEMeshes"] = HEMeshList;
    SceneJson["AmbientLight"] = { m_AmbientLight.x, m_AmbientLight.y, m_AmbientLight.z };


    std::ofstream File(FileName, std::ofstream::out | std::ofstream::trunc);

    if (!File.is_open())
    {
        Engine::DEBUGPrint("Failed to save scene :(");
        return;
    }
    // Uncomment to beautify json - makes it easier to debug but makes files much larger
    //File << std::setw(4) << SceneJson;

    // Uncomment to normalness
    File << SceneJson;

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
    json SpotLightsJson = SceneJson["SpotLights"];
    json ModelsJson = SceneJson["Models"];
    json HEMeshesJson = SceneJson["HEMeshes"];

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
        AddDirectionalLight(LoadDirectionalLight(DirLightJson));
    }
    for (json& SpotLightJson : SpotLightsJson)
    {
        AddSpotLight(LoadSpotLight(SpotLightJson));
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
    for (json& HEMeshJson : HEMeshesJson)
    {
        AddHalfEdgeMesh(LoadHEMesh(HEMeshJson, MaterialVec));
    }

    if (SceneJson.contains("AmbientLight"))
    {
        auto Ambient = SceneJson["AmbientLight"];
        m_AmbientLight = Vec3f(Ambient[0], Ambient[1], Ambient[2]);
    }
}

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
                Texture* DiffuseTex = Registry->LoadTexture(LineTokens[0]);
                Texture* NormalTex = Registry->LoadTexture(LineTokens[1]);
                Texture* RoughnessTex = Registry->LoadTexture(LineTokens[2]);
                Texture* MetallicTex = Registry->LoadTexture(LineTokens[3]);
                Texture* AOTex = Registry->LoadTexture(LineTokens[4]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex, MetallicTex, AOTex));
            }
            else if (LineTokens.size() == 4)
            {
                Texture* DiffuseTex = Registry->LoadTexture(LineTokens[0]);
                Texture* NormalTex = Registry->LoadTexture(LineTokens[1]);
                Texture* RoughnessTex = Registry->LoadTexture(LineTokens[2]);
                Texture* MetallicTex = Registry->LoadTexture(LineTokens[3]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex, MetallicTex));
            }
            else if (LineTokens.size() == 3)
            {
                Texture* DiffuseTex = Registry->LoadTexture(LineTokens[0]);
                Texture* NormalTex = Registry->LoadTexture(LineTokens[1]);
                Texture* RoughnessTex = Registry->LoadTexture(LineTokens[2]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex, RoughnessTex));
            }
            else if (LineTokens.size() == 2)
            {
                Texture* DiffuseTex = Registry->LoadTexture(LineTokens[0]);
                Texture* NormalTex = Registry->LoadTexture(LineTokens[1]);

                SceneMaterials.push_back(Graphics->CreateMaterial(DiffuseTex, NormalTex));
            }
            else
            {
                Texture* DiffuseTex = Registry->LoadTexture(LineTokens[0]);
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
                NewModel = new Model(Graphics->CreateModel(SceneStaticMeshes[StaticMeshIndex], SceneMaterials[MaterialIndex]));
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

            uGUID newModelID = m_ModelIDGenerator.Generate();

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

    for (auto& it : m_Models)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(it.second);
        delete it.second;
    }
    for (auto& PointLight : m_PointLights)
    {
        delete PointLight;
    }
    for (auto& DirLight : m_DirectionalLights)
    {
        delete DirLight;
    }
    for (auto& SLight : m_SpotLights)
    {
        delete SLight;
    }

    m_Models.clear();
    m_PointLights.clear();
    m_SpotLights.clear();
    m_DirectionalLights.clear();
    m_Cameras.clear();
    m_HEMeshes.clear();
    m_AmbientLight = Vec3f(0.2f);

#ifdef USE_EDITOR
    m_GenericEditorClickables.clear();
#endif

    // Set camera to default TODO: (want to load camera info from file)
    m_Cameras.push_back(Camera());
}

void Scene::CopyInternal(const Scene& other)
{
    Clear();
    m_Cameras.resize(1);

    m_Cameras = other.m_Cameras;

    for (auto it : other.m_Models)
    {
        Model* newModel = new Model(*it.second);
        AddModel(newModel);


        Behaviour* oldBehaviour = BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(it.second);
        if (oldBehaviour)
        {
            // Copy actual behaviour so parameter changes are picked up
            // TODO: Behaviour parameters are not serialized
            BehaviourRegistry::Get()->CopyAndAttachNewBehaviour(oldBehaviour, newModel);
        }
    }

    for (auto& heMesh : other.m_HEMeshes)
    {
        AddHalfEdgeMesh(heMesh);
    }

    for (auto& pointLight : other.m_PointLights)
    {
        AddPointLight(*pointLight);
    }
    for (auto& spotLight : other.m_SpotLights)
    {
        AddSpotLight(*spotLight);
    }
    for (auto& directionalLight : other.m_DirectionalLights)
    {
        AddDirectionalLight(*directionalLight);
    }

    m_AmbientLight = other.m_AmbientLight;
}

bool Scene::IsIgnored(Model* model, std::vector<Model*>& ignoredModels)
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
        if (heMesh->m_RepModelsNeedUpdate)
        {
            graphics.UpdateHEMeshModel(heMesh);
            heMesh->m_RepModelsNeedUpdate = false;
        }

        for (auto& repModel : heMesh->m_RepModels)
        {
            StaticMeshRenderCommand command;
            command.m_Material = repModel.m_Material;
            command.m_Mesh = repModel.m_StaticMesh.Id;
            command.m_TransMat = repModel.GetTransform().GetTransformMatrix();

            graphics.AddRenderCommand(command);
        }
    }
    for (auto& it : m_Models)
    {
        StaticMeshRenderCommand command;
        command.m_Material = it.second->m_Material;
        command.m_Mesh = it.second->m_StaticMesh.Id;
        command.m_TransMat = it.second->GetTransform().GetTransformMatrix();

        graphics.AddRenderCommand(command);
    }

    for (PointLight* Light : m_PointLights)
    {
        PointLightRenderCommand LightRC;
        LightRC.m_Colour = Light->colour;
        LightRC.m_Position = Light->position;
        LightRC.m_Intensity = Light->intensity;

        LightRC.m_ConstantAttenuation = Light->constantAttenuation;
        LightRC.m_LinearAttenuation = Light->linearAttenuation;
        LightRC.m_QuadraticAttenuation = Light->quadraticAttenuation;

        LightRC.m_CastShadows = Light->castShadows;

        graphics.AddRenderCommand(LightRC);
    }
    for (SpotLight* SLight : m_SpotLights)
    {
        SpotLightRenderCommand SLightRC;
        SLightRC.m_Colour = SLight->colour;
        SLightRC.m_Position = SLight->position;
        SLightRC.m_Direction = SLight->direction;
        
        SLightRC.m_Intensity = SLight->intensity;
        
        SLightRC.m_ConstantAttenuation = SLight->constantAttenuation;
        SLightRC.m_LinearAttenuation = SLight->linearAttenuation;
        SLightRC.m_QuadraticAttenuation = SLight->quadraticAttenuation;

        SLightRC.m_InnerAngle = SLight->innerAngle;
        SLightRC.m_OuterAngle = SLight->outerAngle;
        
        graphics.AddRenderCommand(SLightRC);
    }
    for (DirectionalLight* Light : m_DirectionalLights)
    {
        DirectionalLightRenderCommand RC;
        RC.m_Colour = Light->colour;
        RC.m_Direction = Light->direction;
        RC.m_ShadowBlurMult = Light->shadowBlurMult;

        graphics.AddRenderCommand(RC);
    }
    AmbientLightRenderCommand ARC;
    ARC.m_Light = m_AmbientLight;
    graphics.AddRenderCommand(ARC);
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
    JsonObject[0] = "textures/" + Mat.m_Albedo->Path.GetFileName();
    JsonObject[1] = "textures/" + Mat.m_Normal->Path.GetFileName();
    JsonObject[2] = "textures/" + Mat.m_Roughness->Path.GetFileName();
    JsonObject[3] = "textures/" + Mat.m_Metallic->Path.GetFileName();
    JsonObject[4] = "textures/" + Mat.m_AO->Path.GetFileName();
}

void Scene::SaveStaticMesh(json& JsonObject, StaticMesh& Mesh)
{
    JsonObject = { "models/" + Mesh.Path.GetFileName() };
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
    JsonObject["Position"] = { DirLight.position.x, DirLight.position.y, DirLight.position.z };
}

void Scene::SaveSpotLight(json& JsonObject, SpotLight& SpotLight)
{
    JsonObject["Position"] = { SpotLight.position.x, SpotLight.position.y, SpotLight.position.z };
    JsonObject["Direction"] = { SpotLight.direction.x, SpotLight.direction.y, SpotLight.direction.z };
    JsonObject["Colour"] = { SpotLight.colour.x, SpotLight.colour.y, SpotLight.colour.z };
    JsonObject["Intensity"] = SpotLight.intensity;
    JsonObject["ConstantAttenuation"] = SpotLight.constantAttenuation;
    JsonObject["LinearAttenuation"] = SpotLight.linearAttenuation;
    JsonObject["QuadraticAttenuation"] = SpotLight.quadraticAttenuation;
    JsonObject["InnerAngle"] = SpotLight.innerAngle;
    JsonObject["OuterAngle"] = SpotLight.outerAngle;
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

void Scene::SaveHEMesh(json& JsonObject, he::HalfEdgeMesh* HeMesh, std::vector<Material>& MatVec)
{
    json FaceListJson;
    json HalfEdgeListJson;
    json VertexListJson;

    std::vector<he::Face*>& Faces = HeMesh->m_Faces;
    std::vector<he::HalfEdge*>& HalfEdges = HeMesh->m_HalfEdges;
    std::vector<he::Vertex*>& Vertices = HeMesh->m_Verts;

    // Save faces
    for (auto& face : Faces)
    {
        json FaceJson;

        int64_t MaterialIndex = 0;

        auto MatIt = std::find(MatVec.begin(), MatVec.end(), face->material);
        if (MatIt != MatVec.end())
        {
            MaterialIndex = MatIt - MatVec.begin();
        }
        else
        {
            Engine::FatalError("Could not find material, this should never happen");
        }

        FaceJson["MatID"] = MaterialIndex;

        FaceJson["TNU"] = face->textureNudgeU;
        FaceJson["TNV"] = face->textureNudgeV;
        
        FaceJson["TSU"] = face->textureScaleU;
        FaceJson["TSV"] = face->textureScaleV;

        FaceJson["Flip"] = face->flipFace;

        FaceJson["R"] = face->textureRot;

        FaceJson["OverrideUV"] = face->useUVOverride;
        if (face->useUVOverride)
        {
            for (Vec2f& UV : face->uvOverrides)
            {
                FaceJson["UVOverride"].push_back({ UV.x, UV.y });
            }
        }


        int64_t HEIndex = 0;

        auto HalfEdgeIt = std::find(HalfEdges.begin(), HalfEdges.end(), face->halfEdge);
        if (HalfEdgeIt != HalfEdges.end())
        {
            HEIndex = HalfEdgeIt - HalfEdges.begin();
        }
        else
        {
            Engine::FatalError("Could not find Half Edge Index, this should never happen");
        }
        FaceJson["HE"] = HEIndex;

        FaceListJson.push_back(FaceJson);
    }

    // Save halfedges
    for (auto& halfEdge : HalfEdges)
    {
        json HalfEdgeJson;

        int64_t NextIndex = 0;
        auto NextIt = std::find(HalfEdges.begin(), HalfEdges.end(), halfEdge->next);
        if (NextIt != HalfEdges.end())
        {
            NextIndex = NextIt - HalfEdges.begin();
        }
        else
        {
            Engine::FatalError("Could not find Next Half Edge Index, this should never happen");
        }

        int64_t TwinIndex = 0;
        auto TwinIt = std::find(HalfEdges.begin(), HalfEdges.end(), halfEdge->twin);
        if (TwinIt != HalfEdges.end())
        {
            TwinIndex = TwinIt - HalfEdges.begin();
        }
        else
        {
            TwinIndex = -1;
            //Engine::FatalError("Could not find Twin Half Edge Index, this should never happen");
        }

        int64_t VertIndex = 0;
        auto VertIt = std::find(Vertices.begin(), Vertices.end(), halfEdge->vert);
        if (VertIt != Vertices.end())
        {
            VertIndex = VertIt - Vertices.begin();
        }
        else
        {
            Engine::FatalError("Could not find Vert Index, this should never happen");
        }

        int64_t FaceIndex = 0;
        auto FaceIt = std::find(Faces.begin(), Faces.end(), halfEdge->face);
        if (FaceIt != Faces.end())
        {
            FaceIndex = FaceIt - Faces.begin();
        }
        else
        {
            Engine::FatalError("Could not find Face Index, this should never happen");
        }


        HalfEdgeJson["Next"] = NextIndex;
        HalfEdgeJson["Twin"] = TwinIndex;
        HalfEdgeJson["Vert"] = VertIndex;
        HalfEdgeJson["Face"] = FaceIndex;

        HalfEdgeListJson.push_back(HalfEdgeJson);
    }

    // Save vertices
    for (auto& vertex : Vertices)
    {
        json VertexJson;

        int64_t HEIndex = 0;

        auto HalfEdgeIt = std::find(HalfEdges.begin(), HalfEdges.end(), vertex->halfEdge);
        if (HalfEdgeIt != HalfEdges.end())
        {
            HEIndex = HalfEdgeIt - HalfEdges.begin();
        }
        else
        {
            Engine::FatalError("Could not find Half Edge Index, this should never happen");
        }

        VertexJson["HE"] = HEIndex;

        VertexJson["Pos"] = { vertex->vec.x, vertex->vec.y, vertex->vec.z };

        VertexListJson.push_back(VertexJson);
    }

    JsonObject["Faces"] = FaceListJson;
    JsonObject["HalfEdges"] = HalfEdgeListJson;
    JsonObject["Vertices"] = VertexListJson;
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

    return Material(Albedo, Normal, Roughness, Metallic, AO);
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

    Vec3f VecPos;

    if (JsonObject.contains("Position"))
    {
        auto Pos = JsonObject["Position"];
        VecPos = Vec3f(Pos[0], Pos[1], Pos[2]);
    }

    DirectionalLight DLight;
    DLight.direction = VecDir;
    DLight.colour = VecColour;
    DLight.position = VecPos;

    return DLight;
}

SpotLight Scene::LoadSpotLight(json& JsonObject)
{
    auto Pos = JsonObject["Position"];
    auto Dir = JsonObject["Direction"];
    auto Clr = JsonObject["Colour"];
    Vec3f VecPos = Vec3f(Pos[0], Pos[1], Pos[2]);
    Vec3f VecDir = Vec3f(Dir[0], Dir[1], Dir[2]);
    Vec3f VecColour = Vec3f(Clr[0], Clr[1], Clr[2]);
    float Intensity = JsonObject["Intensity"];
    float ConstantAttenuation = JsonObject["ConstantAttenuation"];
    float LinearAttenuation = JsonObject["LinearAttenuation"];
    float QuadraticAttenuation = JsonObject["QuadraticAttenuation"];
    float InnerAngle = JsonObject["InnerAngle"];
    float OuterAngle = JsonObject["OuterAngle"];
    SpotLight SLight;
    SLight.position = VecPos;
    SLight.direction = VecDir;
    SLight.colour = VecColour;
    SLight.intensity = Intensity;
    SLight.constantAttenuation = ConstantAttenuation;
    SLight.linearAttenuation = LinearAttenuation;
    SLight.quadraticAttenuation = QuadraticAttenuation;
    SLight.innerAngle = InnerAngle;
    SLight.outerAngle = OuterAngle;
    return SLight;
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

    Model* NewModel = new Model(ModelMesh, ModelMat);
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

he::HalfEdgeMesh* Scene::LoadHEMesh(json& JsonObject, std::vector<Material>& MaterialVector)
{

    he::HalfEdgeMesh* NewHEMesh = new he::HalfEdgeMesh();

    for (json& FaceJson : JsonObject["Faces"])
    {
        he::Face* newFace = new he::Face();
        newFace->material = Material(MaterialVector[FaceJson["MatID"]]);
        newFace->textureNudgeU = FaceJson["TNU"];
        newFace->textureNudgeV = FaceJson["TNV"];
        
        newFace->textureScaleU = FaceJson["TSU"];
        newFace->textureScaleV = FaceJson["TSV"];

        newFace->textureRot = FaceJson["R"];
        if (FaceJson.contains("Flip"))
        {
            newFace->flipFace = FaceJson["Flip"];
        }

        if (FaceJson.contains("OverrideUV") && FaceJson["OverrideUV"])
        {
            for (json& UVJson : FaceJson["UVOverride"])
            {
                newFace->uvOverrides.push_back(Vec2f(UVJson[0], UVJson[1]));
            }
            newFace->useUVOverride = true;
        }

        NewHEMesh->m_Faces.push_back(newFace);
    }

    for (json& HalfEdgeJson : JsonObject["HalfEdges"])
    {
        he::HalfEdge* newHalfEdge = new he::HalfEdge();

        NewHEMesh->m_HalfEdges.push_back(newHalfEdge);
    }

    for (json& VertexJson : JsonObject["Vertices"])
    {
        he::Vertex* newVertex = new he::Vertex(Vec3f(VertexJson["Pos"][0], VertexJson["Pos"][1], VertexJson["Pos"][2]));

        NewHEMesh->m_Verts.push_back(newVertex);
    }

    // Connect all half edge mesh constructs
    int index = 0;
    for (json& FaceJson : JsonObject["Faces"])
    {
        NewHEMesh->m_Faces[index]->halfEdge = NewHEMesh->m_HalfEdges[FaceJson["HE"]];
        
        index++;
    }
    index = 0;
    for (json& HalfEdgeJson : JsonObject["HalfEdges"])
    {
        NewHEMesh->m_HalfEdges[index]->next = NewHEMesh->m_HalfEdges[HalfEdgeJson["Next"]];
        if (HalfEdgeJson["Twin"] == -1)
        {
            NewHEMesh->m_HalfEdges[index]->twin = nullptr;
        }
        else
        {
            NewHEMesh->m_HalfEdges[index]->twin = NewHEMesh->m_HalfEdges[HalfEdgeJson["Twin"]];
        }
        NewHEMesh->m_HalfEdges[index]->vert = NewHEMesh->m_Verts[HalfEdgeJson["Vert"]];
        NewHEMesh->m_HalfEdges[index]->face = NewHEMesh->m_Faces[HalfEdgeJson["Face"]];

        index++;
    }
    index = 0;
    for (json& VertexJson : JsonObject["Vertices"])
    {
        NewHEMesh->m_Verts[index]->halfEdge = NewHEMesh->m_HalfEdges[VertexJson["HE"]];

        index++;
    }

    return NewHEMesh;
}
