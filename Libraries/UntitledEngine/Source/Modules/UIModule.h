#pragma once
#include "Platform\RendererPlatform.h"
#include "Interfaces\Resizeable_i.h"
#include "Modules\GraphicsModule.h"
#include "Modules\TextModule.h"
#include "Modules\InputModule.h"
#include "Utils\Hash.h"

#include <unordered_map>
#include <functional>
#include <stack>

#define ElementID size_t

struct CursorInfo
{
    CursorInfo() : Top(Vec2f(0.0f, 0.0f)), Bottom(Vec2f(0.0f, 0.0f))
    {
    }

    CursorInfo(Vec2f InTop, Vec2f InBottom) : Top(InTop), Bottom(InBottom)
    {
    }

    Vec2f Top = Vec2f(0.0f, 0.0f);
    Vec2f Bottom = Vec2f(0.0f, 0.0f);
};

struct Click
{
    void Update(Rect bounds, Rect frameRect);

    explicit operator bool()
    {
        return clicked;
    }
    bool hovering = false;
    bool clicking = false;
    bool clicked = false;
};

struct ElementState
{
    bool m_Alive = true;

    Click m_Click;
};

struct FrameState : public ElementState
{
    // The tab is changed on a one frame delay 
    uint32_t activeTabNextTick = 0;

    uint32_t activeTab = 0;

    std::string name;

    float verticalOffset = 0.0f;
    bool scrollingNeeded = false;
    bool elementOutsideRectBoundsThisFrame = false;
    float maxElementOffset = 0.0f;
};

struct ButtonState : public ElementState
{
};

struct TextEntryState : public ElementState
{
    bool focused = false;
};

struct FloatSliderState : public ElementState
{
    float percentage = 0.0f;
    bool dragging = false;
};

enum class PlacementType
{
    SIZE,
    RECT,
};

struct PlacementSettings
{
    PlacementSettings(Vec2f inSize);
    PlacementSettings(Rect inRect);

    PlacementSettings(PlacementType inType, float f);
    PlacementSettings(PlacementType inType, Vec2f v);
    PlacementSettings(PlacementType inType, Rect r);

    PlacementType type;
    
    union
    {    
        Vec2f size;
        Rect rect;
    };
};

class UIModule
    : public IResizeable
{
public:

    UIModule(GraphicsModule& graphics, TextModule& text, InputModule& input, Renderer& renderer);
    ~UIModule();

    void AlignLeft();
    void AlignRight();
    void AlignTop();
    void AlignBottom();

    void ImgPanel(Texture texture, Rect rect);
    void ImgPanel(Texture_ID texture, Rect rect);

    void BufferPanel(Framebuffer_ID fBuffer, Rect rect);
    void BufferPanel(Framebuffer_ID fBuffer, Vec2f size);

    Click TextButton(std::string text, PlacementSettings placeSettings, float borderWidth, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f), Vec3f textColour = Vec3f(0.0f, 0.0f, 0.0f));
    Click ImgButton(std::string name, Texture texture, Vec2f size, float borderWidth, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    Click BufferButton(std::string name, Framebuffer_ID fBuffer, Vec2f size, float borderWidth, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

    void Text(std::string text, Vec2f position, Vec3f colour = Vec3f(0.1f, 0.1f, 0.4f));
    void Text(std::string text, Colour col = Colour(0.1f, 0.1f, 0.4f));

    void TextEntry(std::string name, std::string& stringRef, Vec2f size, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void FloatTextEntry(std::string name, float& floatRef, Vec2f size, Colour colour = Vec3f(1.0f, 1.0f, 1.0f));

    bool StartFrame(std::string name, Rect rect, float borderWidth, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void EndFrame();

    void StartFrame(std::string name, Vec2f size, float borderWidth, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

    bool StartTab(std::string text = "", Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));
    void EndTab();

    void FloatSlider(std::string name, Vec2f size, float& outNum, float min = 0.0f, float max = 1.0f, bool vertical = false, bool drawText = true, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

    void NewLine(float lineHeight = 10.0f);

    void OnFrameStart();
    void OnFrameEnd();

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;

    static UIModule* Get() { return s_Instance; }
private:

    Click ButtonInternal(std::string name, Vec2f size, float borderWidth, Vec3f colour);

    Click ButtonInternal(std::string name, Rect rect, float borderWidth, Vec3f colour);

    void FloatSliderInternal(std::string name, Rect rect, float& outNum, float min = 0.0f, float max = 1.0f, bool vertical = false, bool drawText = true, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));


    // Returns the bounds of an element given a placement setting, without advancing the cursor
    Rect SizeElement(PlacementSettings settings);

    // Returns bounds of an element given a placement setting, and advances the cursor
    Rect PlaceElement(PlacementSettings settings);

    MeshData GetVertexDataForRect(Rect rect);
    MeshData GetVertexDataForBorderMesh(Rect rect, float borderWidth);   

    ElementID GetElementID(std::string name);

    FrameState* GetFrameState(std::string name);
    ButtonState* GetButtonState(std::string name);
    TextEntryState* GetTextEntryState(std::string name);
    FloatSliderState* GetFloatSliderState(std::string name);

    Rect GetFrame();

    // Returns whether the ui element we're about to draw/update should be drawn or updated. 
    // As an example, if we're currently in a tab which hasn't been selected, that tab's contents are not active.
    bool IsActive();

    // Returns whether the ui element we're about to draw is visible.
    // If IsActive returns false, this will always return false as well.
    // However, this will additionally return false in some cases where the element is not visible but it still needs to be updated,
    // like when it's been scrolled away from in a scrolling frame but it still needs to update the cursor position.
    bool ShouldDraw(Rect rectToDraw);

    void ResetAllElementAliveFlags();
    void RemoveInactiveElements();

    VertexBufferFormat UIElementFormat;

    std::unordered_map<ElementID, FrameState> m_FrameStates;
    std::unordered_map<ElementID, ButtonState> m_ButtonStates;
    std::unordered_map<ElementID, TextEntryState> m_TextEntryStates;
    std::unordered_map<ElementID, FloatSliderState> m_FloatSliderStates;

    size_t m_HashCount = 0;

    StaticMesh_ID m_RectMesh;
    StaticMesh_ID m_BorderMesh;

    Texture_ID m_DefaultButtonTexture;
    Texture_ID m_DefaultFrameTexture;
    Texture_ID m_DefaultTabTexture;
    Texture_ID m_White;

    Shader_ID m_UIShader;

    GraphicsModule& m_Graphics;
    TextModule& m_Text;
    InputModule& m_Input;
    // TEMP(Fraser) Modules that aren't the Graphics module probably shouldn't be able to access the renderer...
    Renderer& m_Renderer;

    Font m_FrameFont;

    std::stack<Rect> m_SubRectStack;
    std::vector<FrameState*> m_FrameStateStack;
    std::vector<bool> m_InTabStack;
    std::vector<uint32_t> m_CurrentTabIndexStack;
    //uint32_t m_TabIndexOnCurrentFrame = 0;

    bool m_InInactiveTab = false;

    std::stack<CursorInfo> CursorStack;

    ElementState* m_ActiveElement = nullptr;

    Vec2i m_WindowSize;

    bool m_LeftAligned = true;
    bool m_TopAligned = true;

    const float c_TabButtonWidth = 75.0f;

    const Vec3f c_White = Vec3f(1.0f, 1.0f, 1.0f);
    const Vec3f c_NiceBlue = Vec3f(0.0f, 150.f / 255.f, 255.f / 255.f);
    static UIModule* s_Instance;
};