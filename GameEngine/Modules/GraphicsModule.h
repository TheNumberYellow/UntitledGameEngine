#pragma once

#include "Asset/AssetRegistry.h"
#include "Camera.h"
#include "Interfaces\Resizeable_i.h"
#include "Math\Geometry.h"
#include "Platform\RendererPlatform.h"

#include <unordered_map>
#include <vector>

typedef std::pair<std::vector<float>, std::vector<ElementIndex>> MeshData;

class GraphicsModule;

struct GBuffer
{
    Framebuffer_ID Buffer;

    Texture_ID PositionTex;
    Texture_ID NormalTex;
    Texture_ID AlbedoTex;
    Texture_ID MetallicTex;
    Texture_ID RoughnessTex;
    Texture_ID AOTex;

    Framebuffer_ID SkyBuffer;
    Texture_ID SkyTex;

    Framebuffer_ID LightBuffer;
    Texture_ID LightTex;

    Framebuffer_ID DebugBuffer;
    Texture_ID DebugTex;

    StaticMesh_ID QuadMesh;

    Framebuffer_ID FinalOutput;
};

struct Material
{
    Material() {}
    Material(Texture Albedo, Texture Normal, Texture Metallic, Texture Roughness, Texture AO);

    Texture m_Albedo;
    Texture m_Normal;
    Texture m_Metallic;
    Texture m_Roughness;
    Texture m_AO;
    Texture m_Height;

    friend bool operator<(const Material& lhs, const Material& rhs)
    {
        return lhs.m_Albedo < rhs.m_Albedo
            || lhs.m_Normal < rhs.m_Normal;
    }

    friend bool operator==(const Material& lhs, const Material& rhs)
    {
        return lhs.m_Albedo == rhs.m_Albedo
            && lhs.m_Normal == rhs.m_Normal
            && lhs.m_Metallic == rhs.m_Metallic
            && lhs.m_Roughness == rhs.m_Roughness
            && lhs.m_AO == rhs.m_AO;
    }
};

enum class ModelType
{
    BLOCK,
    PLANE,
    MODEL
};

enum class Vis
{
    SHADOW_CAST = 1,
    SHADOW_RECV = 2,

};

enum class RenderMode
{
    FULLBRIGHT,
    DEFAULT
};

class Transform
{
public:
    void SetPosition(Vec3f newPos);
    void SetScale(Vec3f newScale);
    void SetScale(float newScale);
    void SetRotation(Quaternion newRotation);

    Vec3f GetPosition() { return m_Position; }
    Vec3f GetScale() { return m_Scale; }
    Quaternion GetRotation() { return m_Rotation; }

    void Move(Vec3f move);
    void Scale(Vec3f scale);
    void Rotate(Quaternion rotation);

    Mat4x4f GetTransformMatrix();
    void SetTransformMatrix(Mat4x4f mat);

    bool WasTransformMatrixUpdated();

private:

    void UpdateTransformMatrix();

    Vec3f m_Position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f m_Scale = Vec3f(1.0f, 1.0f, 1.0f);
    Quaternion m_Rotation;

    Mat4x4f m_Transform;
    bool m_TransformMatrixNeedsUpdate = false;
    bool m_WasTransformMatrixUpdated = false;
};

struct TexturedMesh
{
    TexturedMesh() {}

    TexturedMesh(StaticMesh mesh, Material material)
        : m_Mesh(mesh)
        , m_Material(material)
    {}

    StaticMesh m_Mesh;

    Material m_Material;
};

class Model
{
public:
    Model()
        : m_Transform()
    {}
    Model(TexturedMesh texturedMesh)
        : m_Transform()
    {
        m_TexturedMeshes.push_back(texturedMesh);
    }

    Transform& GetTransform()
    {
        return m_Transform;
    }

    void SetMaterial(Material material)
    {
        m_TexturedMeshes[0].m_Material = material;
    }

    std::vector<TexturedMesh> m_TexturedMeshes;

    std::string m_Name = "";

    ModelType Type = ModelType::MODEL;
private:
    Transform m_Transform;
};

struct DirectionalLight
{
    Vec3f direction;
    Vec3f colour;
};

struct PointLight
{
    Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f);
};

struct StaticMeshRenderCommand
{
    StaticMesh_ID m_Mesh;
    Material m_Material;
    Transform m_Transform;
};

struct BillboardRenderCommand
{
    Texture_ID m_Texture;
    Vec3f m_Position;
    Vec3f m_Colour = Vec3f(1.0f, 1.0f, 1.0f);
    float m_Size = 1.0f;
};

struct PointLightRenderCommand
{
    Vec3f m_Colour;
    Vec3f m_Position;
};

class GraphicsModule
    : public IResizeable
{
public:
    GraphicsModule(Renderer& renderer);
    ~GraphicsModule();

    GBuffer CreateGBuffer(Vec2i Size);

    void AddRenderCommand(StaticMeshRenderCommand Command);
    void AddRenderCommand(BillboardRenderCommand Command);
    void AddRenderCommand(PointLightRenderCommand Command);

    // Render all submitted render commands into the specified buffer
    //void Render(Framebuffer_ID OutBuffer, Camera Cam, DirectionalLight DirLight);

    void Render(GBuffer Buffer, Camera Cam, DirectionalLight DirLight);

    Shader_ID CreateShader(std::string vertShaderSource, std::string fragShaderSource);

    Framebuffer_ID CreateFBuffer(Vec2i size, FBufferFormat format = FBufferFormat::COLOUR);
    
    void AttachTextureToFBuffer(Texture texture, Framebuffer_ID fBufferID);
    
    Texture CreateTexture(Vec2i size);
    Texture LoadTexture(std::string filePath, TextureMode minFilter = TextureMode::LINEAR, TextureMode magFilter = TextureMode::LINEAR);
    StaticMesh LoadMesh(std::string filePath);

    void SetActiveFrameBuffer(Framebuffer_ID fBufferID);
    void ResizeFrameBuffer(Framebuffer_ID fBufferID, Vec2i size);
    void ResizeGBuffer(GBuffer Buffer, Vec2i Size);
    void ResetFrameBuffer();

    //Material CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap, Texture MetallicMap, Texture AOMap, Texture HeightMap);
    Material CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap, Texture MetallicMap, Texture AOMap);
    Material CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap, Texture MetallicMap);
    Material CreateMaterial(Texture AlbedoMap, Texture NormalMap, Texture RoughnessMap);
    Material CreateMaterial(Texture AlbedoMap, Texture NormalMap);
    Material CreateMaterial(Texture AlbedoMap);

    Model CreateModel(TexturedMesh texturedMesh);

    Model CloneModel(const Model& original);

    //TODO(fraser) Going to want something that's not a model for level geometry like this, something that can be edited easily (and which doesn't need use a transform matrix)
    Model CreateBoxModel(AABB box);
    Model CreateBoxModel(AABB box, Material texture);

    Model CreatePlaneModel(Vec2f min, Vec2f max, float elevation = 0.0f, int subsections = 1);
    Model CreatePlaneModel(Vec2f min, Vec2f max, Material material, float elevation = 0.0f, int subsections = 1);

    void RecalculateTerrainModelNormals(Model& model);

    void Draw(Model& model);

    void SetCamera(Camera* camera);

    void SetDirectionalLight(DirectionalLight dirLight);

    // todo(Fraser): these two should not be called from client code (only the engine)
    // look up a better way to do this
    void OnFrameStart();
    void OnFrameEnd();

    void InitializeDebugDraw();
    void InitializeDebugDraw(Framebuffer_ID fBuffer);
    void DebugDrawLine(Vec3f a, Vec3f b, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void DebugDrawLine(LineSegment line, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    
    // Extremely slow, basically never use this
    void DebugDrawModelMesh(Model model, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void DebugDrawAABB(AABB box, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f), Mat4x4f transform = Mat4x4f());
    void DebugDrawPoint(Vec3f p, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void DebugDrawSphere(Vec3f p, float radius = 1.0f, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

    Vec2i GetViewportSize();

    void SetRenderMode(RenderMode mode);

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;

    Texture_ID GetLightTexture();

    static GraphicsModule* Get() { return s_Instance; }

    //TEMP: public
    Renderer& m_Renderer;
    Camera* m_Camera;
private:

    bool m_CameraMatrixSetThisFrame;

public:
    Shader_ID m_UnlitShader;
    Shader_ID m_TexturedMeshShader;
    Shader_ID m_ShadowShader;
private:
    Shader_ID m_SkyboxShader;
    Shader_ID m_DebugLineShader;
    Shader_ID m_UIShader;

    Texture m_DefaultNormalMap;
    Texture m_DefaultMetallicMap;
    Texture m_DefaultRoughnessMap;
    Texture m_DefaultAOMap;
    Texture m_DefaultHeightMap;

    Material m_DebugMaterial;
    
    VertexBufferFormat m_TexturedMeshFormat;

    //TODO(Fraser): this should likely be moved to some sort of "Scene" and cubemaps should have a more generic interface in the graphics module
    Cubemap_ID m_SkyboxCubemap;
    StaticMesh_ID m_SkyboxMesh;
    
    bool m_IsSkyboxSet;

    RenderMode m_RenderMode;

    Framebuffer_ID m_ActiveFrameBuffer;

    bool m_IsDebugDrawInitialized;
    bool m_IsDebugDrawAttachedToFBuffer;
    Framebuffer_ID m_DebugFBuffer;

    void DrawDebugDrawMesh(Camera cam);

    MeshData GetVertexDataForQuad();
    MeshData GetVertexDataFor3DQuad();

    std::unordered_map<Vec3f, std::vector<float>, Vec3fHash> m_DebugLineMap;
    VertexBufferFormat m_DebugVertFormat;
    StaticMesh_ID m_DebugDrawMesh;

    StaticMesh_ID m_BillboardQuadMesh;
    Texture_ID m_LightTexture;

    Framebuffer_ID m_ShadowBuffer;
    Camera m_ShadowCamera;
    Shader_ID m_PosShader;
    Shader_ID m_NormalsShader;
    Shader_ID m_AlbedoShader;

    std::vector<StaticMeshRenderCommand> m_StaticMeshRenderCommands;
    std::vector<BillboardRenderCommand> m_BillboardRenderCommands;
    std::vector<PointLightRenderCommand> m_PointLightRenderCommands;

    // GBuffer stuff
    Shader_ID m_GBufferShader;
    Shader_ID m_GBufferOldLightingShader;
    Shader_ID m_GBufferSkyShader;
    Shader_ID m_GBufferDebugShader;

    Shader_ID m_GBufferDirectionalLightShader;
    Shader_ID m_GBufferPointLightShader;

    Shader_ID m_GBufferCombinerShader;

    // todo(Fraser): move this to some GUI module?
    Mat4x4f m_OrthoProjection;

    static GraphicsModule* s_Instance;
};