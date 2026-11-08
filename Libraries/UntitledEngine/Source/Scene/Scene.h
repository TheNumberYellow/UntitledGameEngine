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

    template<typename T, typename... Args>
    std::shared_ptr<T> CreateObject(Args&&... args)
    {
        std::shared_ptr<T> object = std::make_shared<T>(std::forward<Args>(args)...);

        uGUID newID = m_SceneObjectIDGenerator.Generate();

        m_SceneObjects[newID] = object;

#ifdef USE_EDITOR
        // Add to generic editor clickables if applicable
        if constexpr (std::is_base_of<IEditorClickable, T>::value)
        {
            GetGenericEditorClickables().push_back(object.get());
        }
#endif

        GetRegistry<T>().push_back(object);

        return object;
    }

    template<typename T>
    void DeleteObject(std::shared_ptr<T> object)
    {
        DeleteObject(object.get());
    }
    
    template<typename T>
    void DeleteObject(T* object)
    {
#ifdef USE_EDITOR
        // Remove from generic editor clickables if applicable
        if constexpr (std::is_base_of<IEditorClickable, T>::value)
        {
            auto& editorClickables = GetGenericEditorClickables();
            std::erase_if(editorClickables, [object](IEditorClickable* clickable) {
                return clickable == object;
                });
        }
#endif

        auto it = std::find_if(m_SceneObjects.begin(), m_SceneObjects.end(),
            [object](const auto& pair) { return pair.second.get() == object; });
        if (it != m_SceneObjects.end())
        {
            m_SceneObjectIDGenerator.FreeID(it->first);
            m_SceneObjects.erase(it);
        }
        auto& registry = GetRegistry<T>();
        registry.erase(std::remove_if(registry.begin(), registry.end(),
            [object](const std::shared_ptr<T>& ptr) { return ptr.get() == object; }),
            registry.end());
    }

    Model* GetModelByTag(std::string tag);
    std::vector<Model*> GetModelsByTag(std::string tag);


#ifdef USE_EDITOR
    std::vector<IEditorClickable*>& GetGenericEditorClickables();
#endif

    std::vector<std::shared_ptr<PointLight>>& GetPointLights();
    
    std::vector<std::shared_ptr<SpotLight>>& GetSpotLights();
    
    std::vector<std::shared_ptr<he::HalfEdgeMesh>>& GetHalfEdgeMeshes();

    void AddCamera(Camera* camera);

    Camera* GetCamera(size_t index = 0);
    void SetCamera(Camera* camera);

    void Initialize();
    void InitializeBehaviours();

    void Update(double DeltaTime);
    void UpdateBehaviours(double DeltaTime);

    void Draw(GraphicsModule& graphics, GBuffer gBuffer, size_t camIndex = 0);
#ifdef USE_EDITOR
    void EditorDraw(GraphicsModule& graphics, GBuffer gBuffer, Camera* editorCam, bool drawSceneCam = true);
#endif

    // TODO: Unify mesh/model collisions
    SceneRayCastHit RayCast(
        Ray ray, 
        std::vector<Model*> IgnoredModels = std::vector<Model*>(), 
        std::vector<he::HalfEdgeMesh*> IgnoredHalfEdgeMeshes = std::vector<he::HalfEdgeMesh*>());

    Intersection SphereIntersect(Sphere sphere, std::vector<Model*> IgnoredModels = std::vector<Model*>());

    Model* MenuListEntities(UIModule& ui, Font& font);

    void DrawSceneHierarchyPanel();
    void DrawSettingsPanel();

    void Save(std::string FileName);

    void Load(std::string FileName);
    //void LegacyLoad(std::string FileName);

    void Clear();

private:

    template<typename T>
    auto& GetRegistry();

    template<>
    auto& GetRegistry<Model>()
    {
        return m_Models;
    }

    template<>
    auto& GetRegistry<he::HalfEdgeMesh>()
    {
        return m_HEMeshes;
    }

    template<>
    auto& GetRegistry<PointLight>()
    {
        return m_PointLights;
    }

    template<>
    auto& GetRegistry<DirectionalLight>()
    {
        return m_DirectionalLights;
    }

    template<>
    auto& GetRegistry<SpotLight>()
    {
        return m_SpotLights;
    }

    template<>
    auto& GetRegistry<Decal>()
    {
        return m_Decals;
    }

    void CopyInternal(const Scene& other);

    bool IsIgnored(Model* model, std::vector<Model*>& ignoredModels);

    // Temp
public:
    std::vector<std::shared_ptr<Model>> m_Models;
private:

    std::unordered_map<uGUID, std::shared_ptr<SceneObject>> m_SceneObjects;

    std::vector<std::shared_ptr<PointLight>> m_PointLights;
    std::vector<std::shared_ptr<DirectionalLight>> m_DirectionalLights;
    std::vector<std::shared_ptr<SpotLight>> m_SpotLights;

    std::vector<std::shared_ptr<Decal>> m_Decals;

    // TODO: these might only be in editor builds (might all be converted to regular models in non-editor builds)
    std::vector< std::shared_ptr<he::HalfEdgeMesh>> m_HEMeshes;

    std::vector<Camera> m_Cameras;

    Vec3f m_AmbientLight = Vec3f(0.2f);

    bool m_UseProceduralSky = false;

    Colour m_ZenithColour = Colour(0.8f, 0.23f, 0.56f);
    Colour m_HorizonColour = Colour(0.23f, 0.34f, 0.95f);
    Colour m_GroundColour = Colour(0.4f, 0.4f, 0.4f);
    
    float m_ZenithExponent = 1.25f;
    float m_GroundExponent = 1.25f;

    float m_HorizonFalloff = 1.0f;

#ifdef USE_EDITOR
    std::vector<IEditorClickable*> m_GenericEditorClickables;
#endif

    bool m_Paused = false;

    void PushSceneRenderCommandsInternal(GraphicsModule& graphics);

    static bool GetReaderStateFromToken(std::string Token, FileReaderState& OutState);

    // Utility functions for scene serialization/deserialization
    void SaveMaterial(json& JsonObject, Material& Mat);
    void SaveStaticMesh(json& JsonObject, StaticMesh& Mesh);
    void SavePointLight(json& JsonObject, PointLight& PointLight);
    void SaveDirectionalLight(json& JsonObject, DirectionalLight& DirLight);
    void SaveSpotLight(json& JsonObject, SpotLight& SpotLight);
    void SaveModel(json& JsonObject, Model& Mod, int64_t MeshIndex, int64_t MatIndex);
    void SaveRawModel(json& JsonObject, Model& Mod, int64_t MatIndex);
    void SaveHEMesh(json& JsonObject, he::HalfEdgeMesh& HeMesh, std::vector<Material>& MatVec);

    Material LoadMaterial(json& JsonObject);
    StaticMesh LoadStaticMesh(json& JsonObject);
    std::shared_ptr<PointLight> LoadPointLight(json& JsonObject);
    std::shared_ptr<DirectionalLight> LoadDirectionalLight(json& JsonObject);
    std::shared_ptr<SpotLight> LoadSpotLight(json& JsonObject);
    std::shared_ptr<Model> LoadModel(json& JsonObject, std::vector<Material>& MaterialVector, std::vector<StaticMesh>& StaticMeshVector);
    std::shared_ptr<Model> LoadRawModel(json& JsonObject, std::vector<Material>& MaterialVector);
    std::shared_ptr<he::HalfEdgeMesh> LoadHEMesh(json& JsonObject, std::vector<Material>& MaterialVector);

#ifdef USE_EDITOR
    // Editor specific rendering stuff
    static Texture* LightBillboardTexture;
    static StaticMesh* CameraMesh;
    static Material* CameraMaterial;
    static StaticMesh* DirectionalLightMesh;
    static Material* DirectionalLightMaterial;
    static StaticMesh* SpotLightMesh;
    static Material* SpotLightMaterial;
    static Texture* DecalBillboardTexture;
#endif

    uGUIDGenerator m_ModelIDGenerator;
    uGUIDGenerator m_SceneObjectIDGenerator;

    // Special case just for saving scenes "as" entities in the editor - will likely come back to this
    friend class EditorState;
};