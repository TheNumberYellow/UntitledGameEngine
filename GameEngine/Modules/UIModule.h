#pragma once
#include "Platform\RendererPlatform.h"
#include "Interfaces\Resizeable_i.h"
#include "Modules\TextModule.h"
#include "Utils\Hash.h"

#include <unordered_map>
#include <functional>

typedef std::pair<std::vector<float>, std::vector<ElementIndex>> MeshData;

struct Click
{
    explicit operator bool()
    {
        return clicked;
    }
    bool hovering = false;
    bool clicking = false;
    bool clicked = false;
};

struct ButtonInfo
{
    Rect m_Rect;
    float m_BorderWidth;

    friend size_t Hash_Value(const ButtonInfo& ti)
    {
        size_t h = Hash::Hash_Value(ti.m_Rect);
        return Hash::Combine(h, Hash::Hash_Value(ti.m_BorderWidth));
    }

    friend bool operator==(const ButtonInfo& lhs, const ButtonInfo& rhs)
    {
        return (lhs.m_Rect == rhs.m_Rect) && (lhs.m_BorderWidth == rhs.m_BorderWidth);
    }
};

struct ButtonState
{
    bool hovering = false;
    bool clicking = false;
};

class UIModule
    : public IResizeable
{
public:

    UIModule(Renderer& renderer, TextModule& text);
    ~UIModule();

    void AlignLeft();
    void AlignRight();
    void AlignTop();
    void AlignBottom();

    void ImgPanel(Texture_ID texture, Rect rect);
    void BufferPanel(Framebuffer_ID fBuffer, Rect rect);
    Click ImgButton(Texture_ID texture, Rect rect, float borderWidth);
    Click BufferButton(Framebuffer_ID fBuffer, Rect rect, float borderWidth);

    void StartFrame(Rect rect, float borderWidth, std::string text = "");
    void EndFrame();

    void StartTab(Rect rect);
    void EndTab();

    void OnFrameStart();
    void OnFrameEnd();

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;
private:

    Click Button(unsigned int img, Rect rect, float borderWidth, bool isBuffer);

    MeshData GetVertexDataForRect(Rect rect);
    MeshData GetVertexDataForBorderMesh(Rect rect, float borderWidth);

    ButtonState* GetButtonState(Rect rect, float borderWidth);

    std::unordered_map<ButtonInfo, ButtonState, Hash::Hasher<ButtonInfo>> m_Buttons;

    size_t m_HashCount = 0;

    Mesh_ID m_RectMesh;
    Mesh_ID m_BorderMesh;
    Texture_ID m_DefaultBorderTexture;
    Texture_ID m_DefaultFrameTexture;
    Shader_ID m_UIShader;

    Renderer& m_Renderer;
    TextModule& m_Text;

    Font m_FrameFont;

    Rect m_SubFrame;
    Vec2i m_WindowSize;

    bool m_LeftAligned = true;
    bool m_TopAligned = true;

};