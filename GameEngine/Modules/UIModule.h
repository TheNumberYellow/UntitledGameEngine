#pragma once
#include "..\Platform\RendererPlatform.hpp"

#include "..\Interfaces\Resizeable_i.h"

#include <unordered_map>
#include <functional>

typedef std::pair<std::vector<float>, std::vector<ElementIndex>> MeshData;

struct ClickableState
{
    bool hovering = false;
    bool clicking = false;
};

class UIModule
    : public IResizeable
{
public:

    UIModule(Renderer& renderer);
    ~UIModule();

    void AlignLeft();
    void AlightRight();
    void AlignTop();
    void AlignBottom();

    void ImgPanel(Texture_ID texture, Rect rect);
    void BufferPanel(Framebuffer_ID fBuffer, Rect rect);
    bool ImgButton(Texture_ID texture, Rect rect, float borderWidth);
    void StartFrame(Rect rect, float borderWidth);
    
    void OnFrameStart();

    void OnFrameEnd();

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;
private:

    MeshData GetVertexDataForRect(Rect rect);
    MeshData GetVertexDataForBorderMesh(Rect rect, float borderWidth);

    std::unordered_map<size_t, ClickableState> m_Clickables;

    size_t m_HashCount = 0;
    ClickableState* GetClickable(std::string value);

    Mesh_ID m_RectMesh;
    Mesh_ID m_BorderMesh;
    Texture_ID m_DefaultBorderTexture;
    Texture_ID m_DefaultFrameTexture;
    Shader_ID m_UIShader;

    Renderer& m_Renderer;

    bool m_LeftAligned = true;
    bool m_TopAligned = true;

};