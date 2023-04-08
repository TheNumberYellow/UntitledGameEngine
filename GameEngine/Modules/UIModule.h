#pragma once
#include "Platform\RendererPlatform.h"
#include "Interfaces\Resizeable_i.h"
#include "Modules\TextModule.h"
#include "Modules\InputModule.h"
#include "Utils\Hash.h"

#include <unordered_map>
#include <functional>
#include <stack>

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

struct FrameInfo
{
    Rect m_Rect;
    float m_BorderWidth;
    std::string m_Name;

    FrameInfo(Rect rect, float borderWidth, std::string name) : m_Rect(rect), m_BorderWidth(borderWidth), m_Name(name)
    {
    }

    friend size_t Hash_Value(const FrameInfo& fi)
    {
        size_t h = Hash::Hash_Value(fi.m_Rect);
        h = Hash::Combine(h, Hash::Hash_Value(fi.m_BorderWidth));
        return Hash::Combine(h, Hash::Hash_Value(fi.m_Name));
    }

    friend bool operator==(const FrameInfo& lhs, const FrameInfo& rhs)
    {
        return (lhs.m_BorderWidth == rhs.m_BorderWidth) && (lhs.m_Name == rhs.m_Name) && (lhs.m_Rect == rhs.m_Rect);
    }
}; 

struct FrameState
{
    uint32_t activeTab = 0;
};

struct ButtonInfo
{
    Rect m_Rect;
    float m_BorderWidth;

    ButtonInfo(Rect rect, float borderWidth) : m_Rect(rect), m_BorderWidth(borderWidth)
    {
    }

    friend size_t Hash_Value(const ButtonInfo& bi)
    {
        size_t h = Hash::Hash_Value(bi.m_Rect);
        return Hash::Combine(h, Hash::Hash_Value(bi.m_BorderWidth));
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

struct TextEntryInfo
{
    Rect m_Rect;

    TextEntryInfo(Rect rect) : m_Rect(rect)
    {
    }

    friend size_t Hash_Value(const TextEntryInfo& ti)
    {
        size_t h = Hash::Hash_Value(ti.m_Rect);
        return h;
    }

    friend bool operator==(const TextEntryInfo& lhs, const TextEntryInfo& rhs)
    {
        return (lhs.m_Rect == rhs.m_Rect);
    }
};

struct TextEntryState
{
    bool focused = false;
};

class UIModule
    : public IResizeable
{
public:

    UIModule(Renderer& renderer, TextModule& text, InputModule& input);
    ~UIModule();

    void AlignLeft();
    void AlignRight();
    void AlignTop();
    void AlignBottom();

    void ImgPanel(Texture_ID texture, Rect rect);
    void BufferPanel(Framebuffer_ID fBuffer, Rect rect);

    Click TextButton(std::string text, Rect rect, float borderWidth);
    Click ImgButton(Texture_ID texture, Rect rect, float borderWidth);
    Click BufferButton(Framebuffer_ID fBuffer, Rect rect, float borderWidth);

    void Text(std::string text, Vec2f position);

    void TextEntry(std::string& stringRef, Rect rect);

    void StartFrame(Rect rect, float borderWidth, std::string text = "");
    void EndFrame();

    void StartTab(std::string text = "");
    void EndTab();

    void OnFrameStart();
    void OnFrameEnd();

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;
private:

    Click Button(unsigned int img, Rect rect, float borderWidth, bool hasImage, bool isBuffer, bool hasText, std::string text = "");

    MeshData GetVertexDataForRect(Rect rect);
    MeshData GetVertexDataForBorderMesh(Rect rect, float borderWidth);

    ButtonState* GetButtonState(Rect rect, float borderWidth);
    
    TextEntryState* GetTextEntryState(Rect rect);

    FrameState* GetFrameState(FrameInfo& fInfo);
    FrameState* GetFrameState(Rect rect, float borderWidth, std::string name = "");

    Rect GetFrame();

    // Returns whether the ui element we're about to draw/update should be drawn or updated. 
    // As an example, if we're currently in a tab which hasn't been selected, that tab's contents should not be drawn.
    bool ShouldDisplay();

    std::unordered_map<ButtonInfo, ButtonState, Hash::Hasher<ButtonInfo>> m_Buttons;
    std::unordered_map<FrameInfo, FrameState, Hash::Hasher<FrameInfo>> m_Frames;
    std::unordered_map<TextEntryInfo, TextEntryState, Hash::Hasher<TextEntryInfo>> m_TextEntries;

    bool m_InTab = false;

    size_t m_HashCount = 0;

    Mesh_ID m_RectMesh;
    Mesh_ID m_BorderMesh;
    Texture_ID m_DefaultBorderTexture;
    Texture_ID m_DefaultFrameTexture;
    Shader_ID m_UIShader;

    Renderer& m_Renderer;
    TextModule& m_Text;
    InputModule& m_Input;

    Font m_FrameFont;

    std::stack<FrameInfo> m_SubFrameStack;

    uint32_t m_TabIndexOnCurrentFrame = 0;

    Vec2i m_WindowSize;

    bool m_LeftAligned = true;
    bool m_TopAligned = true;

    const float c_TabButtonWidth = 75.0f;
};