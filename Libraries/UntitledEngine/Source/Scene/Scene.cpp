#include "Scene.h"

#include "Behaviour/Behaviour.h"
#include "Graphics/HotspotTexture.h"
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
Texture* Scene::DecalBillboardTexture = nullptr;

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
    if (!DecalBillboardTexture)
    {
        DecalBillboardTexture = Registry->LoadTexture("Assets/images/decalTool.png");
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
    for (auto& it : m_Models)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(it.get());
    }


    m_Models.clear();
    m_PointLights.clear();
    m_DirectionalLights.clear();
    m_SpotLights.clear();
    m_HEMeshes.clear();
    m_Decals.clear();

    for (auto& it : m_SceneObjects)
    {
        m_SceneObjectIDGenerator.FreeID(it.first);
    }
    m_SceneObjects.clear();
#ifdef USE_EDITOR
    m_GenericEditorClickables.clear();
#endif

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

Model* Scene::GetModelByTag(std::string tag)
{
    for (auto it : m_Models)
    {
        if (it->m_Name == tag)
        {
            return it.get();
        }
    }
    return nullptr;
}

std::vector<Model*> Scene::GetModelsByTag(std::string tag)
{
    std::vector<Model*> result;
    for (auto it : m_Models)
    {
        if (it->m_Name == tag)
        {
            result.push_back(it.get());
        }
    }
    return result;
}

#ifdef USE_EDITOR
std::vector<IEditorClickable*>& Scene::GetGenericEditorClickables()
{
    return m_GenericEditorClickables;
}
#endif

std::vector<std::shared_ptr<PointLight>>& Scene::GetPointLights()
{
    return m_PointLights;
}

std::vector<std::shared_ptr<SpotLight>>& Scene::GetSpotLights()
{
    return m_SpotLights;
}

std::vector<std::shared_ptr<he::HalfEdgeMesh>>& Scene::GetHalfEdgeMeshes()
{
    return m_HEMeshes;
}

void Scene::AddCamera(Camera* camera)
{
    m_Cameras.push_back(Camera(*camera));
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
        Registry->InitializeModelBehaviours(it.get(), this);
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
        Registry->UpdateModelBehaviours(it.get(), this, DeltaTime);
    }
}

void Scene::Draw(GraphicsModule& graphics, GBuffer gBuffer, size_t camIndex)
{
    PushSceneRenderCommandsInternal(graphics);

    assert(camIndex < m_Cameras.size());

    graphics.Render(gBuffer, m_Cameras[camIndex]);
}

#ifdef USE_EDITOR
void Scene::EditorDraw(GraphicsModule& graphics, GBuffer gBuffer, Camera* editorCam, bool drawSceneCam)
{
    PushSceneRenderCommandsInternal(graphics);
    
    assert(editorCam);
    for (auto& Light : m_PointLights)
    {
        BillboardRenderCommand BillboardRC;
        BillboardRC.m_Colour = Light->colour;
        BillboardRC.m_Position = Light->position;
        BillboardRC.m_Texture = LightBillboardTexture->GetID();
        BillboardRC.m_Size = 0.5f;

        graphics.AddRenderCommand(BillboardRC);
    }
    for (auto& Light : m_DirectionalLights)
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
    for (auto& SLight : m_SpotLights)
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
    for (auto& decal : m_Decals)
    {
        BillboardRenderCommand BillboardRC;
        BillboardRC.m_Colour = Vec3f(1.0f, 1.0f, 1.0f);
        BillboardRC.m_Position = Vec3f(decal->orientation.m_Rows[3].x, decal->orientation.m_Rows[3].y, decal->orientation.m_Rows[3].z);
        BillboardRC.m_Texture = DecalBillboardTexture->GetID();
        BillboardRC.m_Size = 0.5f;

        graphics.AddRenderCommand(BillboardRC);
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

    for (auto& it : m_Models)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it.get()) > 0)
        {
            continue;
        }
        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it->m_StaticMesh);
        
        finalHit = Closer(finalHit, SceneRayCastHit{ Collision.RayCast(ray, colMesh, it->GetTransform()), it.get() });
    }

    // TODO: Some sort of typed union (or maybe polymorphism if this gets more complex) for returning multiple hit object types
    for (auto& it : m_HEMeshes)
    {
        if (std::count(IgnoredHalfEdgeMeshes.begin(), IgnoredHalfEdgeMeshes.end(), it.get()) > 0)
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

    for (auto& it : m_Models)
    {
        if (std::count(IgnoredModels.begin(), IgnoredModels.end(), it.get()) > 0)
        {
            continue;
        }

        CollisionMesh& colMesh = *Collision.GetCollisionMeshFromMesh(it->m_StaticMesh);

        Intersection ModelIntersection = Collision.SphereIntersection(sphere, colMesh, it->GetTransform());

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
        Model* model = it.get();
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

void Scene::DrawSceneHierarchyPanel()
{

}

void Scene::DrawSettingsPanel()
{
    UIModule* UI = UIModule::Get();

    // Ambient light colour setting
    UI->Text("Ambient Light:");
    UI->NewLine();
    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_AmbientLight.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_AmbientLight.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_AmbientLight.b);

    UI->Text("Sky Settings:");
    UI->NewLine();
    UI->CheckBox("Use Procedural Sky", m_UseProceduralSky);
    if (m_UseProceduralSky)
    {
        UI->NewLine();

        UI->Text("Zenith Colour:");
        UI->NewLine();
        UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_ZenithColour.r);
        UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_ZenithColour.g);
        UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_ZenithColour.b);
        UI->NewLine();

        UI->Text("Horizon Colour:");
        UI->NewLine();
        UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_HorizonColour.r);
        UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_HorizonColour.g);
        UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_HorizonColour.b);
        UI->NewLine();
        
        UI->Text("Ground Colour:");
        UI->NewLine();
        UI->FloatSlider("R", Vec2f(400.0f, 20.0f), m_GroundColour.r);
        UI->FloatSlider("G", Vec2f(400.0f, 20.0f), m_GroundColour.g);
        UI->FloatSlider("B", Vec2f(400.0f, 20.0f), m_GroundColour.b);
        UI->NewLine();

        static float testSliderFloat = 0.0f;
        UI->FloatSlider("TestSlider", Vec2f(20.0f, 200.0f), testSliderFloat, 0.0f, 100.0f, true);
        //UI->FloatSlider("Zenith Exponent", Vec2f(400.0f, 20.0f), m_ZenithExponent, 0.0f, 2.0f);
        //UI->FloatSlider("Ground Exponent", Vec2f(400.0f, 20.0f), m_GroundExponent, 0.0f, 2.0f);
        //UI->FloatSlider("Horizon Falloff", Vec2f(400.0f, 20.0f), m_HorizonFalloff, -2.0f, 2.0f);
    }
}

void Scene::Save(std::string FileName)
{
    // Set of all textures used in the scene
    std::set<Material> Materials;
    // Set of all Static Meshes used in the scene
    std::set<StaticMesh> StaticMeshes;

    for (auto& it : m_Models)
    {
        std::shared_ptr<Model> model = it;

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
        std::shared_ptr<he::HalfEdgeMesh> heMesh = it;

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
    for (std::shared_ptr<PointLight> PLight : m_PointLights)
    {
        SavePointLight(PointLightList[Index++], *PLight);
    }
    Index = 0;
    for (std::shared_ptr<DirectionalLight> DLight : m_DirectionalLights)
    {
        SaveDirectionalLight(DirLightList[Index++], *DLight);
    }
    Index = 0;
    for (std::shared_ptr<SpotLight> SLight : m_SpotLights)
    {
        SaveSpotLight(SpotLightList[Index++], *SLight);
    }
    Index = 0;
    for (auto it : m_Models)
    {
        std::shared_ptr<Model> model = it;

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
    for (std::shared_ptr<he::HalfEdgeMesh> heMesh : m_HEMeshes)
    {
        SaveHEMesh(HEMeshList[Index++], *heMesh, MatVec);
    }


    SceneJson["Textures"] = TextureList;
    SceneJson["StaticMeshes"] = StaticMeshList;
    SceneJson["PointLights"] = PointLightList;
    SceneJson["DirLights"] = DirLightList;
    SceneJson["SpotLights"] = SpotLightList;
    SceneJson["Models"] = ModelList;
    SceneJson["HEMeshes"] = HEMeshList;
    SceneJson["AmbientLight"] = { m_AmbientLight.x, m_AmbientLight.y, m_AmbientLight.z };
    SceneJson["UseProceduralSky"] = m_UseProceduralSky;
    SceneJson["ZenithColour"] = { m_ZenithColour.x, m_ZenithColour.y, m_ZenithColour.z };
    SceneJson["HorizonColour"] = { m_HorizonColour.x, m_HorizonColour.y, m_HorizonColour.z };
    SceneJson["GroundColour"] = { m_GroundColour.x, m_GroundColour.y, m_GroundColour.z };
    SceneJson["ZenithExponent"] = m_ZenithExponent;
    SceneJson["GroundExponent"] = m_GroundExponent;

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
        Engine::Alert("Failed to open level " + FileName + ". Error code: " + strerror(errno));
        return;
    }

    Clear();

    if (!json::accept(File))
    {
        Engine::Alert("Failed to parse level " + FileName + ". Error code: " + strerror(errno));
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
        LoadPointLight(PointLightJson);
    }
    for (json& DirLightJson : DirLightsJson)
    {
        LoadDirectionalLight(DirLightJson);
    }
    for (json& SpotLightJson : SpotLightsJson)
    {
        LoadSpotLight(SpotLightJson);
    }
    for (json& ModelJson : ModelsJson)
    {
        if (ModelJson.contains("Buffer"))
        {
            LoadRawModel(ModelJson, MaterialVec);
        }
        else
        {
            LoadModel(ModelJson, MaterialVec, StaticMeshVec);
        }
    }
    for (json& HEMeshJson : HEMeshesJson)
    {
        LoadHEMesh(HEMeshJson, MaterialVec);
    }

    if (SceneJson.contains("AmbientLight"))
    {
        auto Ambient = SceneJson["AmbientLight"];
        m_AmbientLight = Vec3f(Ambient[0], Ambient[1], Ambient[2]);
    }
    if (SceneJson.contains("UseProceduralSky"))
    {
        m_UseProceduralSky = SceneJson["UseProceduralSky"];
    }
    if (SceneJson.contains("ZenithColour"))
    {
        auto ZColour = SceneJson["ZenithColour"];
        m_ZenithColour = Vec3f(ZColour[0], ZColour[1], ZColour[2]); 
    }
    if (SceneJson.contains("HorizonColour"))
    {
        auto HColour = SceneJson["HorizonColour"];
        m_HorizonColour = Vec3f(HColour[0], HColour[1], HColour[2]);
    }
    if (SceneJson.contains("GroundColour"))
    {
        auto GColour = SceneJson["GroundColour"];
        m_GroundColour = Vec3f(GColour[0], GColour[1], GColour[2]);
    }
    if (SceneJson.contains("ZenithExponent"))
    {
        m_ZenithExponent = SceneJson["ZenithExponent"];
    }
    if (SceneJson.contains("GroundExponent"))
    {
        m_GroundExponent = SceneJson["GroundExponent"];
    }
}

void Scene::Clear()
{
    m_SceneObjectIDGenerator.Reset();

    for (auto& it : m_Models)
    {
        BehaviourRegistry::Get()->ClearBehavioursOnEntity(it.get());
    }

    m_Models.clear();
    m_PointLights.clear();
    m_SpotLights.clear();
    m_DirectionalLights.clear();
    m_Cameras.clear();
    m_HEMeshes.clear();
    m_AmbientLight = Vec3f(0.2f);
    
    m_SceneObjects.clear();
    
    m_UseProceduralSky = false;
    
    m_ZenithColour = Colour(0.8f, 0.23f, 0.56f);
    m_HorizonColour = Colour(0.23f, 0.34f, 0.95f);
    m_GroundColour = Colour(0.4f, 0.4f, 0.4f);

    m_ZenithExponent = 1.25f;
    m_GroundExponent = 1.25f;


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
        
        std::shared_ptr<Model> newModel = CreateObject<Model>(it->m_StaticMesh, it->m_Material);

        newModel->m_Name = it->m_Name;
        newModel->SetTransform(it->GetTransform());

        Behaviour* oldBehaviour = BehaviourRegistry::Get()->GetBehaviourAttachedToEntity(it.get());
        if (oldBehaviour)
        {
            // Copy actual behaviour so parameter changes are picked up
            // TODO: Behaviour parameters are not serialized
            BehaviourRegistry::Get()->CopyAndAttachNewBehaviour(oldBehaviour, newModel.get());
        }
    }

    for (auto& heMesh : other.m_HEMeshes)
    {
        CreateObject<he::HalfEdgeMesh>(*heMesh);
    }

    for (auto& pointLight : other.m_PointLights)
    {
        CreateObject<PointLight>(*pointLight);
    }
    for (auto& spotLight : other.m_SpotLights)
    {
        CreateObject<SpotLight>(*spotLight);
    }
    for (auto& directionalLight : other.m_DirectionalLights)
    {
        CreateObject<DirectionalLight>(*directionalLight);
    }
    for (auto& decal : other.m_Decals)
    {
        CreateObject<Decal>(*decal);
    }

    m_AmbientLight = other.m_AmbientLight;
    
    m_UseProceduralSky = other.m_UseProceduralSky;
    m_ZenithColour = other.m_ZenithColour;
    m_HorizonColour = other.m_HorizonColour;
    m_GroundColour = other.m_GroundColour;
    m_ZenithExponent = other.m_ZenithExponent;
    m_GroundExponent = other.m_GroundExponent;
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
            graphics.UpdateHEMeshModel(this, heMesh.get());
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
    for (auto& Model : m_Models)
    {
        StaticMeshRenderCommand command;
        command.m_Material = Model->m_Material;
        command.m_Mesh = Model->m_StaticMesh.Id;
        command.m_TransMat = Model->GetTransform().GetTransformMatrix();

        graphics.AddRenderCommand(command);
    }

    for (auto& Light : m_PointLights)
    {
        PointLightRenderCommand LightRC;
        LightRC.m_Colour = Light->colour;
        LightRC.m_Position = Light->position;
        LightRC.m_Intensity = Light->intensity;

        LightRC.m_Radius = Light->radius;

        LightRC.m_CastShadows = Light->castShadows;

        graphics.AddRenderCommand(LightRC);
    }
    for (auto& SLight : m_SpotLights)
    {
        SpotLightRenderCommand SLightRC;
        SLightRC.m_Colour = SLight->colour;
        SLightRC.m_Position = SLight->position;
        SLightRC.m_Direction = SLight->direction;
        
        SLightRC.m_Intensity = SLight->intensity;
        
        SLightRC.m_Range = SLight->range;

        SLightRC.m_InnerAngle = SLight->innerAngle;
        SLightRC.m_OuterAngle = SLight->outerAngle;
        
        graphics.AddRenderCommand(SLightRC);
    }
    for (auto& Light : m_DirectionalLights)
    {
        DirectionalLightRenderCommand RC;
        RC.m_Colour = Light->colour;
        RC.m_Direction = Light->direction;
        RC.m_ShadowBlurMult = Light->shadowBlurMult;

        graphics.AddRenderCommand(RC);
    }
    for (auto& decal : m_Decals)
    {
        if (decal->m_RepModel)
        {
            StaticMeshRenderCommand command;
            command.m_Material = decal->m_RepModel->m_Material;
            command.m_Mesh = decal->m_RepModel->m_StaticMesh.Id;
            command.m_TransMat = decal->m_RepModel->GetTransform().GetTransformMatrix();
            command.m_CastShadows = false;
            graphics.AddRenderCommand(command);
        }
    }

    AmbientLightRenderCommand ARC;
    ARC.m_Light = m_AmbientLight;
    graphics.AddRenderCommand(ARC);

    if (m_UseProceduralSky)
    {
        ProceduralSkyRenderCommand pSRC;
        pSRC.m_ZenithColour = m_ZenithColour;
        pSRC.m_HorizonColour = m_HorizonColour;
        pSRC.m_GroundColour = m_GroundColour;

        pSRC.m_ZenithExponent = m_ZenithExponent;
        pSRC.m_GroundExponent = m_GroundExponent;

        pSRC.m_HorizonFalloff = m_HorizonFalloff;

        graphics.AddRenderCommand(pSRC);
    }
    else
    {
        SkyboxRenderCommand sRC;

        sRC.m_Cubemap = 0;

        graphics.AddRenderCommand(sRC);
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
    JsonObject["Radius"] = PointLight.radius;
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
    JsonObject["Range"] = SpotLight.range;
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

void Scene::SaveHEMesh(json& JsonObject, he::HalfEdgeMesh& HeMesh, std::vector<Material>& MatVec)
{
    json FaceListJson;
    json HalfEdgeListJson;
    json VertexListJson;

    std::vector<he::Face*>& Faces = HeMesh.m_Faces;
    std::vector<he::HalfEdge*>& HalfEdges = HeMesh.m_HalfEdges;
    std::vector<he::Vertex*>& Vertices = HeMesh.m_Verts;

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


        if (face->appliedHotspotTexture)
        {
            FaceJson["HotspotTex"] = face->appliedHotspotTexture->m_Path.GetRelativePath();
        }
        else
        {        
            // Soon to be deprecated
            FaceJson["OverrideUV"] = face->useUVOverride;
            if (face->useUVOverride)
            {
                for (Vec2f& UV : face->uvOverrides)
                {
                    FaceJson["UVOverride"].push_back({ UV.x, UV.y });
                }
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

std::shared_ptr<PointLight> Scene::LoadPointLight(json& JsonObject)
{
    std::shared_ptr<PointLight> NewPointLight = CreateObject<PointLight>();

    auto Pos = JsonObject["Position"];
    auto Colour = JsonObject["Colour"];

    Vec3f VecPos = Vec3f(Pos[0], Pos[1], Pos[2]);
    Vec3f VecColour = Vec3f(Colour[0], Colour[1], Colour[2]);
    
    float Intensity = JsonObject["Intensity"];

    float Radius = JsonObject.contains("Radius") ? JsonObject["Radius"].get<float>() : 5.0f;

    NewPointLight->position = VecPos;
    NewPointLight->colour = VecColour;
    NewPointLight->intensity = Intensity;
    NewPointLight->radius = Radius;

    return NewPointLight;
}

std::shared_ptr<DirectionalLight> Scene::LoadDirectionalLight(json& JsonObject)
{
    std::shared_ptr<DirectionalLight> NewDirectionalLight = CreateObject<DirectionalLight>();

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

    NewDirectionalLight->direction = VecDir;
    NewDirectionalLight->colour = VecColour;
    NewDirectionalLight->position = VecPos;

    return NewDirectionalLight;
}

std::shared_ptr<SpotLight> Scene::LoadSpotLight(json& JsonObject)
{
    std::shared_ptr<SpotLight> NewSpotLight = CreateObject<SpotLight>();

    auto Pos = JsonObject["Position"];
    auto Dir = JsonObject["Direction"];
    auto Clr = JsonObject["Colour"];
    Vec3f VecPos = Vec3f(Pos[0], Pos[1], Pos[2]);
    Vec3f VecDir = Vec3f(Dir[0], Dir[1], Dir[2]);
    Vec3f VecColour = Vec3f(Clr[0], Clr[1], Clr[2]);
    float Intensity = JsonObject["Intensity"];
    float InnerAngle = JsonObject["InnerAngle"];
    float OuterAngle = JsonObject["OuterAngle"];

    NewSpotLight->position = VecPos;
    NewSpotLight->direction = VecDir;
    NewSpotLight->colour = VecColour;
    NewSpotLight->intensity = Intensity;
    NewSpotLight->range = JsonObject.contains("Range") ? JsonObject["Range"].get<float>() : 10.0f;
    NewSpotLight->innerAngle = InnerAngle;
    NewSpotLight->outerAngle = OuterAngle;
    
    return NewSpotLight;
}

std::shared_ptr<Model> Scene::LoadModel(json& JsonObject, std::vector<Material>& MaterialVector, std::vector<StaticMesh>& StaticMeshVector)
{
    Material ModelMat = MaterialVector[JsonObject["MatID"]];
    StaticMesh ModelMesh = StaticMeshVector[JsonObject["MeshID"]];
    
    std::shared_ptr<Model> NewModel = CreateObject<Model>();
    NewModel->m_Material = ModelMat;
    NewModel->m_StaticMesh = ModelMesh;

    auto Trans = JsonObject["Transform"];

    Mat4x4f TransMat;
    TransMat[0] = { Trans[0], Trans[1], Trans[2], Trans[3]};
    TransMat[1] = { Trans[4], Trans[5], Trans[6], Trans[7] };
    TransMat[2] = { Trans[8], Trans[9], Trans[10], Trans[11] };
    TransMat[3] = { Trans[12], Trans[13], Trans[14], Trans[15] };

    NewModel->GetTransform().SetTransformMatrix(TransMat);

    for (std::string Behaviour : JsonObject["Behaviours"])
    {
        BehaviourRegistry::Get()->AttachNewBehaviour(Behaviour, NewModel.get());
    }

    if (JsonObject.contains("Type"))
    {
        NewModel->Type = JsonObject["Type"];
    }

    return NewModel;
}

std::shared_ptr<Model> Scene::LoadRawModel(json& JsonObject, std::vector<Material>& MaterialVector)
{
    Material ModelMat = MaterialVector[JsonObject["MatID"]];
    
    std::vector<float> MeshBuffer = JsonObject["Buffer"][0];
    std::vector<unsigned int> IndexBuffer = JsonObject["Buffer"][1];

    std::shared_ptr<Model> NewModel = CreateObject<Model>(GraphicsModule::Get()->CreateStaticMesh(MeshBuffer, IndexBuffer), ModelMat);

    auto Trans = JsonObject["Transform"];

    Mat4x4f TransMat;
    TransMat[0] = { Trans[0], Trans[1], Trans[2], Trans[3] };
    TransMat[1] = { Trans[4], Trans[5], Trans[6], Trans[7] };
    TransMat[2] = { Trans[8], Trans[9], Trans[10], Trans[11] };
    TransMat[3] = { Trans[12], Trans[13], Trans[14], Trans[15] };
   
    NewModel->GetTransform().SetTransformMatrix(TransMat);

    for (std::string Behaviour : JsonObject["Behaviours"])
    {
        BehaviourRegistry::Get()->AttachNewBehaviour(Behaviour, NewModel.get());
    }

    if (JsonObject.contains("Type"))
    {
        NewModel->Type = JsonObject["Type"];
    }

    return NewModel;
}

std::shared_ptr<he::HalfEdgeMesh> Scene::LoadHEMesh(json& JsonObject, std::vector<Material>& MaterialVector)
{

    std::shared_ptr<he::HalfEdgeMesh> NewHEMesh = CreateObject<he::HalfEdgeMesh>();

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

        if (FaceJson.contains("HotspotTex") && FaceJson["HotspotTex"] != "")
        {
            std::string HotspotPath = FaceJson["HotspotTex"];
            if (HotspotPath.find("Assets") == std::string::npos) HotspotPath = "Assets/" + HotspotPath;
            newFace->appliedHotspotTexture = AssetRegistry::Get()->LoadHotspotTexture(HotspotPath);
        
            //newFace->ApplyHotspotTexture(*newFace->appliedHotspotTexture);
        }
        else
        {
            // Soon to be deprecated
            if (FaceJson.contains("OverrideUV") && FaceJson["OverrideUV"])
            {
                for (json& UVJson : FaceJson["UVOverride"])
                {
                    newFace->uvOverrides.push_back(Vec2f(UVJson[0], UVJson[1]));
                }
                newFace->useUVOverride = true;
            }
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

    for (he::Face* face : NewHEMesh->m_Faces)
    {
        if (face->appliedHotspotTexture != nullptr)
        {
            face->ApplyHotspotTexture(*face->appliedHotspotTexture);
        }
    }

    return NewHEMesh;
}
