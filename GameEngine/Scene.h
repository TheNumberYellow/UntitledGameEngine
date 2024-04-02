#pragma once

#include "Modules/ModuleManager.h"
#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include <string>

#include <json.hpp>

using json = nlohmann::json;

typedef uint64_t Entity_ID;

// TODO(Fraser): Move this to some reader/asset manager file
enum FileReaderState
{
    TEXTURES,
    STATIC_MESHES,
    ENTITIES,
    NONE
};

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
    Scene(Scene& other);
    Scene& operator=(const Scene& other);
    ~Scene();

    void Pause();
    void UnPause();
    bool IsPaused();

    Model* AddModel(Model model, std::string name = "");
    void DeleteModel(Model* model);
    
    Model* GetModel(std::string name);
    Model* GetModelByTag(std::string tag);
    std::vector<Model*> GetModelsByTag(std::string tag);

    PointLight* AddPointLight(PointLight newLight);
    void DeletePointLight(PointLight* light);
    
    std::vector<PointLight*>& GetPointLights();

    void AddCamera(Camera* camera);

    Camera* GetCamera(size_t index = 0);
    void SetCamera(Camera* camera);

    void Initialize();
    void InitializeBehaviours();

    void Update(float DeltaTime);
    void UpdateBehaviours(float DeltaTime);

    void Draw(GraphicsModule& graphics, GBuffer gBuffer, size_t camIndex = 0);
    void EditorDraw(GraphicsModule& graphics, GBuffer gBuffer, Camera* editorCam);

    void SetDirectionalLight(DirectionalLight light);

    SceneRayCastHit RayCast(Ray ray, std::vector<Model*> IgnoredModels = std::vector<Model*>());

    Model* MenuListEntities(UIModule& ui, Font& font);

    void Save(std::string FileName);

    void Load(std::string FileName);

    void Clear();
private:
    bool IsIgnored(Model* model, std::vector<Model*> ignoredModels);

    std::unordered_map<std::string, Model*> m_Models;
    std::vector<Model*> m_UntrackedModels;
    
    std::vector<PointLight*> m_PointLights;
    
    DirectionalLight m_DirLight;

    std::vector<Camera> m_Cameras;

    bool m_Paused = false;

    void PushSceneRenderCommandsInternal(GraphicsModule& graphics);

    static bool GetReaderStateFromToken(std::string Token, FileReaderState& OutState);

    // Utility functions for scene serialization/deserialization
    void SaveMaterial(json& JsonObject, Material& Mat);
    void SaveStaticMesh(json& JsonObject, StaticMesh& Mesh);
    void SavePointLight(json& JsonObject, PointLight& PointLight);
    void SaveModel(json& JsonObject, Model& Mod, int64_t MeshIndex, int64_t MatIndex);
    void SaveRawModel(json& JsonObject, Model& Mod, int64_t MatIndex);

    Material LoadMaterial(json& JsonObject);
    StaticMesh LoadStaticMesh(json& JsonObject);
    PointLight LoadPointLight(json& JsonObject);
    Model LoadModel(json& JsonObject, std::vector<Material>& MaterialVector, std::vector<StaticMesh>& StaticMeshVector);
    Model LoadRawModel(json& JsonObject, std::vector<Material>& MaterialVector);

    // Editor specific rendering stuff
    static Texture* LightBillboardTexture;
    static StaticMesh* CameraMesh;
    static Material* CameraMaterial;
};
