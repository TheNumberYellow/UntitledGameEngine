#pragma once

#include "Modules/ModuleManager.h"
#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/UIModule.h"

#include <string>

typedef uint64_t Entity_ID;

// TODO(Fraser): Move this to some reader/asset manager file
enum FileReaderState
{
    TEXTURES,
    STATIC_MESHES,
    ENTITIES,
    NONE
};

struct SceneObject
{
    AABB m_Bounds;
    Transform m_Transform;
};

struct ModelObject : public SceneObject
{

};

struct PointLightObject : public SceneObject
{

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

    void Init(GraphicsModule& graphics, CollisionModule& collisions);

    void Pause();
    void UnPause();
    bool IsPaused();

    Model* AddModel(Model model, std::string name = "");
    void DeleteModel(Model* model);
    
    Model* GetModel(std::string name);
    Model* GetModelByTag(std::string tag);
    std::vector<Model*> GetModelsByTag(std::string tag);

    void AddPointLight(PointLight newLight);

    void AddCamera(Camera* camera);

    Camera* GetCamera();
    void SetCamera(Camera* camera);

    void UpdateBehaviours(ModuleManager& Modules, float DeltaTime);

    void Draw(GraphicsModule& graphics, Framebuffer_ID buffer, GBuffer gBuffer);
    void EditorDraw(GraphicsModule& graphics, Framebuffer_ID buffer);

    void SetDirectionalLight(DirectionalLight light);

    SceneRayCastHit RayCast(Ray ray, CollisionModule& collision, std::vector<Model*> IgnoredModels = std::vector<Model*>());

    Model* MenuListEntities(UIModule& ui, Font& font);

    void Save(std::string FileName);

    void Load(std::string FileName);
private:
    bool IsIgnored(Model* model, std::vector<Model*> ignoredModels);

    std::unordered_map<std::string, Model*> m_Models;
    std::vector<Model*> m_UntrackedModels;
    DirectionalLight m_DirLight;

    std::vector<PointLight> m_PointLights;

    std::vector<Camera*> m_Cameras;

    bool m_Paused = false;

    // TEMP member variables start
    //Camera m_ShadowCamera;
    //Framebuffer_ID m_ShadowBuffer;

    //GBuffer GBuf;

    //Shader_ID PosShader;

    // TEMP member variables end

    GraphicsModule* m_Graphics;
    CollisionModule* m_Collisions;

    static bool GetReaderStateFromToken(std::string Token, FileReaderState& OutState);

};
