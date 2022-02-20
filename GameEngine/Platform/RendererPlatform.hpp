#pragma once

#include <string>
#include <vector>
#include <initializer_list>

#include "EnginePlatform.hpp"
#include "..\Math\Math.hpp"
#include "..\GUID.hpp"

typedef GUID Framebuffer_ID;
typedef GUID Texture_ID;
typedef GUID Cubemap_ID;
typedef GUID Shader_ID;
typedef GUID Mesh_ID;

enum class ColourFormat
{
    RGB,
    RGBA,
    Red,
    DEPTH
};

enum class FBufferFormat
{
    COLOUR,
    DEPTH
};

enum class TextureMode
{
    NEAREST,
    LINEAR
};

typedef unsigned int ElementIndex;

enum class VertAttribute
{
    //TODO(fraser) Add normalized fixed-point types
    Int,
    UInt,
    Float,
    Vec2f,
    Vec3f,
    Vec4f,
};

class VertexBufferFormat
{
public:
    VertexBufferFormat(std::initializer_list<VertAttribute> vertAttributes);

    void EnableVertexAttributes() const;

    const std::vector<VertAttribute>& GetAttributes() const;
    unsigned int GetVertexStride() const;

    unsigned int GetCount(VertAttribute vertAttribute) const;
    unsigned int GetSize(VertAttribute vertAttribute) const;

private:
    std::vector<VertAttribute> _attributes;
    unsigned int _vertexStride;
};

enum class VertType
{
    Pos,
    Normal,
    UV
};

enum class Cull
{
    Back,
    Front
};

struct PosVertex
{
    PosVertex() {}
    PosVertex(Vec3f position) : position(position) {}
    Vec3f position;

    static void ActivateVertexAttributes();
};

struct Vertex
{
    Vertex() {}
    Vertex(Vec3f position, Vec3f normal, Vec4f colour, Vec2f uv) : position(position), normal(normal), colour(colour), uv(uv) {}
    Vec3f position;
    Vec3f normal;
    Vec4f colour;
    Vec2f uv;
    
    static void ActivateVertexAttributes();
};

struct UVVertex
{
    UVVertex() {}
    UVVertex(Vec3f position, Vec2f uv) : position(position), uv(uv) {}
    Vec3f position;
    Vec2f uv;

    static void ActivateVertexAttributes();
};

enum class DrawType
{
    Triangle, Line
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    // These functions are defined in renderer-specific renderer code
    Framebuffer_ID CreateFrameBuffer(Vec2i size, FBufferFormat format = FBufferFormat::COLOUR);
    void AttachTextureToFramebuffer(Texture_ID textureID, Framebuffer_ID fBufferID);

    Texture_ID LoadTexture(Vec2i size, std::vector<unsigned char> textureData, ColourFormat format, TextureMode minTexMode = TextureMode::LINEAR, TextureMode magTexMode = TextureMode::LINEAR);
    Texture_ID LoadTexture(std::string filePath, TextureMode minTexMode = TextureMode::LINEAR, TextureMode magTexMode = TextureMode::LINEAR);
    
    Cubemap_ID LoadCubemap(std::string filepath);

    Shader_ID LoadShader(std::string vertShaderSource, std::string fragShaderSource);

    Mesh_ID LoadMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData);
    Mesh_ID LoadMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices);

    void DeleteFrameBuffer(Framebuffer_ID fBufferID);
    void DeleteTexture(Texture_ID textureID);
    void DeleteMesh(Mesh_ID mesh);

    Texture_ID CreateEmptyTexture(Vec2i size, ColourFormat format = ColourFormat::RGB);
    Mesh_ID CreateEmptyMesh(const VertexBufferFormat& vertBufFormat, bool useElementArray = true);

    void ClearMesh(Mesh_ID meshID);
    
    void UpdateTextureData(Texture_ID textureID, Recti region, std::vector<unsigned char> textureData, ColourFormat format);
    
    void UpdateMeshData(Mesh_ID meshID, const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData);
    void UpdateMeshData(Mesh_ID meshID, const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices);

    float* GetMeshVertexData(Mesh_ID meshID);

    void SetActiveFBuffer(Framebuffer_ID fBufferID);
    void ResizeFBuffer(Framebuffer_ID fBufferID, Vec2i newSize);
    void ResetToScreenBuffer();

    void SetActiveTexture(Texture_ID textureID, unsigned int textureSlot = 0);
    void SetActiveFBufferTexture(Framebuffer_ID frameBufferID, unsigned int textureSlot = 0);
    void SetActiveCubemap(Cubemap_ID cubemapID, unsigned int textureSlot = 0);
    void SetActiveShader(Shader_ID shaderID);
    void DrawMesh(Mesh_ID meshID);

    void SetShaderUniformVec2f(Shader_ID shaderID, std::string uniformName, Vec2f vec);
    void SetShaderUniformVec3f(Shader_ID shaderID, std::string uniformName, Vec3f vec);
    void SetShaderUniformMat4x4f(Shader_ID shaderID, std::string uniformName, Mat4x4f mat);
    void SetShaderUniformFloat(Shader_ID shaderID, std::string uniformName, float f);
    void SetShaderUniformInt(Shader_ID shaderID, std::string uniformName, int i);
    void SetShaderUniformBool(Shader_ID shaderID, std::string uniformName, bool b);

    void SetMeshDrawType(Mesh_ID meshID, DrawType type);
    void SetMeshColour(Mesh_ID meshID, Vec4f colour);

    std::vector<Vertex*> MapMeshVertices(Mesh_ID meshID);
    void UnmapMeshVertices(Mesh_ID meshID);

    std::vector<unsigned int*> MapMeshElements(Mesh_ID meshID);
    void UnmapMeshElements(Mesh_ID meshID);

    void ClearScreenAndDepthBuffer();
    void SwapBuffer();

    void ClearDepthBuffer();

    void EnableDepthTesting();
    void DisableDepthTesting();

    void SetCulling(Cull c);

    Vec2i GetViewportSize();

    //TODO(fraser): functions for deleting/freeing memory of fbuffers/textures/meshes/shaders/etc
};