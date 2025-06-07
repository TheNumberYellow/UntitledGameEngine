#pragma once

#include <string>
#include <vector>
#include <initializer_list>

#include "EnginePlatform.h"
#include "Math/Math.h"
#include "GUID.hpp"

// Opaque handles for platform-specifically defined types
typedef GUID Framebuffer_ID;
typedef GUID Texture_ID;
typedef GUID Cubemap_ID;
typedef GUID Shader_ID;
typedef GUID StaticMesh_ID;
typedef GUID GBuffer_ID;

typedef unsigned int ElementIndex;

enum class ColourFormat
{
    RGB,
    RGBA,
    Red,
    DEPTH
};

enum class DataFormat
{
    FLOAT,
    UNSIGNED_BYTE
};

enum class FBufferFormat
{
    COLOUR,
    DEPTH,
    EMPTY
};

enum class TextureMode
{
    NEAREST,
    LINEAR
};

enum class VertAttribute
{
    //TODO(Fraser) Add normalized fixed-point types
    Int,
    UInt,
    Float,
    Vec2f,
    Vec3f,
    Vec4f,
};

struct TextureCreateInfo
{
    TextureCreateInfo(Vec2i Size, ColourFormat InternalFormat, ColourFormat ExternalFormat, DataFormat DataFormat) 
        : Size(Size), InternalFormat(InternalFormat), ExternalFormat(ExternalFormat), DataFormat(DataFormat)
    {}

    TextureCreateInfo() {}

    Vec2i Size;
    ColourFormat InternalFormat = ColourFormat::RGBA;
    ColourFormat ExternalFormat = ColourFormat::RGBA;
    DataFormat DataFormat = DataFormat::FLOAT;
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
    Triangle,
    TriangleFan,
    Line
};

enum class DepthFunc
{
    LESS,
    GREATER
};

enum class BlendFunc
{
    TRANS,
    ADDITIVE
};

enum class StencilCompareFunc
{
    ALWAYS,
    EQUAL,
    GREATER,
    LESS
};

enum class StencilOperationFunc
{
    KEEP,
    REPLACE,
    INCREMENT,
    DECREMENT
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    // These functions are defined in renderer-specific renderer code
    Framebuffer_ID CreateFrameBuffer(Vec2i size, FBufferFormat format = FBufferFormat::COLOUR);
    
    // Create a new FBuffer with the same depth test buffer as an existing FBuffer
    Framebuffer_ID CreateFBufferWithExistingDepthBuffer(Framebuffer_ID existingFBuffer, Vec2i size, FBufferFormat format = FBufferFormat::COLOUR);
    void AttachTextureToFramebuffer(Texture_ID textureID, Framebuffer_ID fBufferID);
    
    Texture_ID AttachColourAttachmentToFrameBuffer(Framebuffer_ID buffer, TextureCreateInfo createInfo, int attachmentIndex);

    Texture_ID LoadTexture(Vec2i size, std::vector<unsigned char> textureData, ColourFormat format, TextureMode minTexMode = TextureMode::LINEAR, TextureMode magTexMode = TextureMode::LINEAR);
    Texture_ID LoadTexture(std::string filePath, TextureMode minTexMode = TextureMode::LINEAR, TextureMode magTexMode = TextureMode::LINEAR);
    
    bool IsTextureValid(Texture_ID texID);
    bool IsFBufferTextureValid(Framebuffer_ID bufID);

    Cubemap_ID LoadCubemap(std::string filepath);

    Shader_ID LoadShader(std::string vertShaderSource, std::string fragShaderSource);

    StaticMesh_ID LoadMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData);
    StaticMesh_ID LoadMesh(const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices);

    void DeleteFrameBuffer(Framebuffer_ID fBufferID);
    void DeleteTexture(Texture_ID textureID);
    void DeleteMesh(StaticMesh_ID mesh);

    Texture_ID CreateEmptyTexture(Vec2i size, ColourFormat format = ColourFormat::RGB);
    StaticMesh_ID CreateEmptyMesh(const VertexBufferFormat& vertBufFormat, bool useElementArray = true);

    void ClearMesh(StaticMesh_ID meshID);
    
    void UpdateTextureData(Texture_ID textureID, Recti region, std::vector<unsigned char> textureData, ColourFormat format);
    
    void UpdateMeshData(StaticMesh_ID meshID, const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData);
    void UpdateMeshData(StaticMesh_ID meshID, const VertexBufferFormat& vertBufFormat, std::vector<float> vertexData, std::vector<ElementIndex> indices);

    std::vector<float> GetMeshVertexData(StaticMesh_ID meshID);
    std::vector<unsigned int> GetMeshIndexData(StaticMesh_ID meshID);

    void SetActiveFBuffer(Framebuffer_ID fBufferID);
    void ResizeFBuffer(Framebuffer_ID fBufferID, Vec2i newSize);
    void ResetToScreenBuffer();

    void SetActiveTexture(Texture_ID textureID, unsigned int textureSlot = 0);
    void SetActiveTexture(Texture_ID textureID, std::string textureName);
    void ResizeTexture(Texture_ID textureID, Vec2i newSize);

    void SetActiveFBufferTexture(Framebuffer_ID frameBufferID, unsigned int textureSlot = 0);
    void SetActiveFBufferTexture(Framebuffer_ID frameBufferID, std::string textureName);
    
    void SetActiveCubemap(Cubemap_ID cubemapID, unsigned int textureSlot = 0);
    void SetActiveCubemap(Cubemap_ID cubemapID, std::string textureName);
    
    void SetActiveShader(Shader_ID shaderID);
    
    void DrawMesh(StaticMesh_ID meshID);

    void SetShaderUniformVec2f(Shader_ID shaderID, std::string uniformName, Vec2f vec);
    void SetShaderUniformVec3f(Shader_ID shaderID, std::string uniformName, Vec3f vec);
    void SetShaderUniformMat4x4f(Shader_ID shaderID, std::string uniformName, Mat4x4f mat);
    void SetShaderUniformFloat(Shader_ID shaderID, std::string uniformName, float f);
    void SetShaderUniformInt(Shader_ID shaderID, std::string uniformName, int i);
    void SetShaderUniformBool(Shader_ID shaderID, std::string uniformName, bool b);

    void SetMeshDrawType(StaticMesh_ID meshID, DrawType type);
    void SetMeshColour(StaticMesh_ID meshID, Vec4f colour);

    std::vector<Vertex*> MapMeshVertices(StaticMesh_ID meshID);
    void UnmapMeshVertices(StaticMesh_ID meshID);

    std::vector<unsigned int*> MapMeshElements(StaticMesh_ID meshID);
    void UnmapMeshElements(StaticMesh_ID meshID);

    void ClearScreenAndDepthBuffer();
    void SwapBuffer();

    void ClearColourBuffer();
    void ClearDepthBuffer();
    void ClearStencilBuffer();

    void EnableDepthTesting();
    void DisableDepthTesting();

    void SetDepthFunction(DepthFunc func);
    void SetBlendFunction(BlendFunc func);

    void EnableStencilTesting();
    void DisableStencilTesting();

    void StartStencilDrawing(StencilCompareFunc compareFunc = StencilCompareFunc::ALWAYS, StencilOperationFunc opFunc = StencilOperationFunc::REPLACE, int value = 1);
    void EndStencilDrawing();

    void StartStencilTesting(StencilCompareFunc func = StencilCompareFunc::EQUAL, int value = 1);
    void EndStencilTesting();

    void SetCulling(Cull c);

    Vec2i GetViewportSize();

private:

    GUIDGenerator textureIDGenerator;
    GUIDGenerator cubemapIDGenerator;
    GUIDGenerator frameBufferIDGenerator;
    GUIDGenerator shaderIDGenerator;
    GUIDGenerator staticMeshIDGenerator;
    //TODO(fraser): functions for deleting/freeing memory of fbuffers/textures/meshes/shaders/etc
};