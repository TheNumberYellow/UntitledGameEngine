#pragma once

#include "Camera.h"
#include "Interfaces\Resizeable_i.h"
#include "Math\Geometry.h"
#include "Platform\RendererPlatform.h"

#include <unordered_map>
#include <vector>

class GraphicsModule;

struct RenderCommand
{
    Mesh_ID mesh;
    bool depthTest;
    uint32_t order;
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

    TexturedMesh(Mesh_ID mesh, Texture_ID texture)
        : m_Mesh(mesh)
        , m_Texture(texture)
    {}

    Mesh_ID m_Mesh;

    //TODO(fraser): soon we'll want multiple textures per mesh (for normal/bump/specular maps etc. etc.)
    Texture_ID m_Texture;
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

    void SetTexture(Texture_ID texture)
    {
        m_TexturedMeshes[0].m_Texture = texture;
    }

    std::vector<TexturedMesh> m_TexturedMeshes;
private:
    Transform m_Transform;
};

class Brush
{
public:
    Brush(Mesh_ID mesh)
        : m_Mesh(mesh)
    {}

    void SetTexture(Texture_ID texture) { m_Texture = texture; }

private:
    Mesh_ID m_Mesh;
    Texture_ID m_Texture;
    Transform m_Transform;
};

struct DirectionalLight
{
    Vec3f direction;
    Vec3f colour;
};

class GraphicsModule
    : public IResizeable
{
public:
    GraphicsModule(Renderer& renderer);
    ~GraphicsModule();

    Framebuffer_ID CreateFBuffer(Vec2i size, FBufferFormat format = FBufferFormat::COLOUR);
    Texture_ID CreateTexture(Vec2i size);
    Texture_ID LoadTexture(std::string filePath, TextureMode minFilter = TextureMode::LINEAR, TextureMode magFilter = TextureMode::LINEAR);
    Mesh_ID LoadMesh(std::string filePath);

    Mesh_ID CreatePlane(float width);

    void AttachTextureToFBuffer(Texture_ID textureID, Framebuffer_ID fBufferID);

    void SetActiveFrameBuffer(Framebuffer_ID fBufferID);
    void ResizeFrameBuffer(Framebuffer_ID fBufferID, Vec2i size);
    void ResetFrameBuffer();

    Model CreateModel(TexturedMesh texturedMesh);

    Model CloneModel(const Model& original);

    //TODO(fraser) Going to want something that's not a model for level geometry like this, something that can be edited easily (and which doesn't need use a transform matrix)
    Model CreateBoxModel(AABB box, Texture_ID texture = 0);
    Brush CreateBrush(AABB box, Texture_ID texture = 0);

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
    void DebugDrawModelMesh(Model model, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void DebugDrawAABB(AABB box, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f), Mat4x4f transform = Mat4x4f());
    void DebugDrawPoint(Vec3f p, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

    Vec2i GetViewportSize();

    void SetRenderMode(RenderMode mode);

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;

    //TEMP: public
    Renderer& m_Renderer;
    Camera* m_Camera;
private:

    Mesh_ID CreateBoxMesh(AABB box);

    bool m_CameraMatrixSetThisFrame;

public:
    Shader_ID m_UnlitShader;
    Shader_ID m_TexturedMeshShader;
    Shader_ID m_ShadowShader;
private:
    Shader_ID m_SkyboxShader;
    Shader_ID m_DebugLineShader;
    Shader_ID m_UIShader;

    Texture_ID m_DebugTexture;
    VertexBufferFormat m_TexturedMeshFormat;

    //TODO(fraser): this should likely be moved to some sort of "Scene" and cubemaps should have a more generic interface in the graphics module
    Cubemap_ID m_SkyboxCubemap;
    Mesh_ID m_SkyboxMesh;
    
    bool m_IsSkyboxSet;

    RenderMode m_RenderMode;

    Framebuffer_ID m_ActiveFrameBuffer;

    bool m_IsDebugDrawInitialized;
    bool m_IsDebugDrawAttachedToFBuffer;
    Framebuffer_ID m_DebugFBuffer;

    void DrawDebugDrawMesh();

    std::unordered_map<Vec3f, std::vector<float>, Vec3fHash> m_DebugLineMap;
    VertexBufferFormat m_DebugVertFormat;
    Mesh_ID m_DebugDrawMesh;

    // todo(Fraser): move this to some GUI module?
    Mat4x4f m_OrthoProjection;
};