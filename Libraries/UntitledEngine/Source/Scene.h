#pragma once


#include "Modules/ModuleManager.h"
#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include "Interfaces/EditorClickable_i.h"

#include <string>

#include <json.hpp>

using json = nlohmann::json;

typedef uint64_t Entity_ID;

namespace he
{
    struct HalfEdgeMesh;
}

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

enum class EditorObjectType
{
    NONE,
    MODEL,
    POINTLIGHT,

};

struct EditorRayCastHit
{
    RayCastHit rayCastHit;
    EditorObjectType typeHit = EditorObjectType::NONE;

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

    Model* AddModel(Model* model);
    void DeleteModel(Model* model);
    
    Model* GetModelByTag(std::string tag);
    std::vector<Model*> GetModelsByTag(std::string tag);

    PointLight* AddPointLight(PointLight newLight);
    void DeletePointLight(PointLight* light);
    
    Brush* AddBrush(Brush* newBrush);
    void DeleteBrush(Brush* brush);

    void AddHalfEdgeMesh(he::HalfEdgeMesh* newMesh);
    void DeleteHalfEdgeMesh(he::HalfEdgeMesh* mesh);

    std::vector<IEditorClickable*>& GetEditorClickables();

    std::vector<PointLight*>& GetPointLights();

    std::vector<Brush*>& GetBrushes();

    void AddCamera(Camera* camera);

    Camera* GetCamera(size_t index = 0);
    void SetCamera(Camera* camera);

    void Initialize();
    void InitializeBehaviours();

    void Update(double DeltaTime);
    void UpdateBehaviours(double DeltaTime);

    void Draw(GraphicsModule& graphics, GBuffer gBuffer, size_t camIndex = 0);
    void EditorDraw(GraphicsModule& graphics, GBuffer gBuffer, Camera* editorCam);

    void SetDirectionalLight(DirectionalLight light);

    SceneRayCastHit RayCast(Ray ray, std::vector<Model*> IgnoredModels = std::vector<Model*>());

    Intersection SphereIntersect(Sphere sphere, std::vector<Model*> IgnoredModels = std::vector<Model*>());

    Model* MenuListEntities(UIModule& ui, Font& font);

    void DrawSettingsPanel();

    void Save(std::string FileName);

    void Load(std::string FileName);
    void LegacyLoad(std::string FileName);

    void Clear();

    DirectionalLight m_DirLight;

private:

    void CopyInternal(const Scene& other);

    bool IsIgnored(Model* model, std::vector<Model*> ignoredModels);

    // Temp
public:
    std::unordered_map<GUID, Model*> m_Models;
    //std::vector<Model*> m_UntrackedModels;
private:

    std::vector<PointLight*> m_PointLights;
    std::vector<Brush*> m_Brushes;
    std::vector<he::HalfEdgeMesh*> m_HEMeshes;

    std::unordered_map<Model*, Brush*> m_ModelBrushMap;

    std::vector<Camera> m_Cameras;

    std::vector<IEditorClickable*> m_EditorClickables;

    bool m_Paused = false;

    void PushSceneRenderCommandsInternal(GraphicsModule& graphics);

    static bool GetReaderStateFromToken(std::string Token, FileReaderState& OutState);

    // Utility functions for scene serialization/deserialization
    void SaveMaterial(json& JsonObject, Material& Mat);
    void SaveStaticMesh(json& JsonObject, StaticMesh& Mesh);
    void SavePointLight(json& JsonObject, PointLight& PointLight);
    void SaveDirectionalLight(json& JsonObject, DirectionalLight& DirLight);
    void SaveModel(json& JsonObject, Model& Mod, int64_t MeshIndex, int64_t MatIndex);
    void SaveRawModel(json& JsonObject, Model& Mod, int64_t MatIndex);
    void SaveBrush(json& JsonObject, Brush& B, int64_t MatIndex);

    Material LoadMaterial(json& JsonObject);
    StaticMesh LoadStaticMesh(json& JsonObject);
    PointLight LoadPointLight(json& JsonObject);
    DirectionalLight LoadDirectionalLight(json& JsonObject);
    Model* LoadModel(json& JsonObject, std::vector<Material>& MaterialVector, std::vector<StaticMesh>& StaticMeshVector);
    Model* LoadRawModel(json& JsonObject, std::vector<Material>& MaterialVector);
    Brush* LoadBrush(json& JsonObject, std::vector<Material>& MaterialVector);

    // Editor specific rendering stuff
    static Texture* LightBillboardTexture;
    static StaticMesh* CameraMesh;
    static Material* CameraMaterial;

    GUIDGenerator m_ModelIDGenerator;
};
