#include "UIModule.h"

UIModule* UIModule::s_Instance = nullptr;

void Click::Update(Rect bounds, Rect frameRect)
{
    Vec2i mousePos = Engine::GetMousePosition();

    if (!frameRect.IsZero() && !frameRect.Contains(mousePos))
    {
        hovering = false;
        clicking = false;
        clicked = false;
        return;
    }

    if (clicked)
    {
        clicked = false;
    }

    if (clicking)
    {
        if (!bounds.Contains(mousePos))
        {
            // Mouse moved out of bounds while clicking, element not clicked
            clicking = false;
            clicked = false;
        }
        if (!Engine::GetMouseDown())
        {
            // Mouse stopped clicking while on element, element was clicked
            clicking = false;
            clicked = true;
        }
    }
    else
    {
        if (Engine::GetMouseDown() && hovering)
        {
            clicking = true;
        }
        if (!Engine::GetMouseDown() && bounds.Contains(mousePos))
        {
            hovering = true;
        }
        if (!bounds.Contains(mousePos))
        {
            hovering = false;
        }
    }
}

UIModule::UIModule(GraphicsModule& graphics, TextModule& text, InputModule& input, Renderer& renderer)
    : m_Graphics(graphics)
    , m_Text(text)
    , m_Input(input)
    , m_Renderer(renderer)
    , UIElementFormat(VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f }))
{
    m_DefaultButtonTexture = m_Renderer.LoadTexture("images/nine_cut_bw.png");
    m_DefaultFrameTexture = m_Renderer.LoadTexture("images/nine_cut_bw.png");
    m_DefaultTabTexture = m_Renderer.LoadTexture("images/nine_cut_bw.png");
    m_White = m_Renderer.LoadTexture("images/white.png");

    // ~~~~~~~~~~~~~~~~~~~~~UI shader code~~~~~~~~~~~~~~~~~~~~~ //
    std::string vertShaderSource = R"(
    #version 400
    
    uniform mat4x4 Projection;

    in vec2 VertPosition;
    in vec2 VertUV;

    smooth out vec2 FragUV;

    void main()
    {
        gl_Position = Projection * vec4(VertPosition, 0.0, 1.0);
        FragUV = VertUV;    
    }
    )";

    std::string fragShaderSource = R"(
    #version 400

    uniform sampler2D Texture;
    uniform bool Hovering;
    uniform bool Clicking;
    uniform vec3 Colour;    

    smooth in vec2 FragUV;	

    out vec4 OutColour;

    void main()
    {
        vec4 textureAt = texture(Texture, FragUV);
        OutColour = textureAt * vec4(Colour, 1.0);

        if (Clicking)
        {
            OutColour.rgb *= 0.5;
        }
        else if (Hovering)
        {
            OutColour.rgb *= 1.4;
        }
    }
    
    )";

    m_UIShader = m_Graphics.CreateShader(vertShaderSource, fragShaderSource);

    Resize(m_Renderer.GetViewportSize());

    m_BorderMesh = m_Renderer.CreateEmptyMesh(UIElementFormat, true);
    m_RectMesh = m_Renderer.CreateEmptyMesh(UIElementFormat, true);

    m_FrameFont = m_Text.LoadFont("fonts/ARLRDBD.TTF", 12);

    s_Instance = this;
}

UIModule::~UIModule()
{
}

void UIModule::AlignLeft()
{
    m_LeftAligned = true;
}

void UIModule::AlignRight()
{
    m_LeftAligned = false;
}

void UIModule::AlignTop()
{
    m_TopAligned = true;
}

void UIModule::AlignBottom()
{
    m_TopAligned = false;
}

void UIModule::ImgPanel(Texture texture, Rect rect)
{
    if (!IsActive())
        return;

    rect.location += GetFrame().location;

    if (!ShouldDraw(rect))
    {
        return;
    }

    m_Renderer.DisableDepthTesting();

    MeshData vertexData = GetVertexDataForRect(rect);

    m_Renderer.UpdateMeshData(m_RectMesh, UIElementFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(texture.Id, "Texture");
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);
    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", c_White);

    m_Renderer.DrawMesh(m_RectMesh);

    m_Renderer.EnableDepthTesting();
}

void UIModule::ImgPanel(Texture_ID texture, Rect rect)
{
    if (!IsActive())
        return;

    rect.location += GetFrame().location;

    if (!ShouldDraw(rect))
    {
        return;
    }

    m_Renderer.DisableDepthTesting();

    MeshData vertexData = GetVertexDataForRect(rect);

    m_Renderer.UpdateMeshData(m_RectMesh, UIElementFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(texture, "Texture");
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);
    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", c_White);

    m_Renderer.DrawMesh(m_RectMesh);

    m_Renderer.EnableDepthTesting();
}

void UIModule::BufferPanel(Framebuffer_ID fBuffer, Rect rect)
{
    if (!IsActive())
        return;

    //rect.location += GetFrame().location;
    
    if (!ShouldDraw(rect))
    {
        return;
    }

    m_Renderer.DisableDepthTesting();

    MeshData vertexData = GetVertexDataForRect(rect);

    m_Renderer.UpdateMeshData(m_RectMesh, UIElementFormat, vertexData.first, vertexData.second);

    //auto testIndices = m_Renderer.MapMeshElements(m_RectMesh);
    //
    //m_Renderer.UnmapMeshElements(m_RectMesh);
    m_Renderer.SetActiveFBufferTexture(fBuffer, "Texture");

    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);
    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", c_White);

    m_Renderer.DrawMesh(m_RectMesh);

    m_Renderer.EnableDepthTesting();
}

void UIModule::BufferPanel(Framebuffer_ID fBuffer, Vec2f size)
{
    if (!IsActive())
        return;

    Rect BufferRect = PlaceElement(size);

    BufferPanel(fBuffer, BufferRect);
}

Click UIModule::TextButton(std::string text, Vec2f size, float borderWidth, Vec3f colour, Vec3f textColour)
{
    if (!IsActive())
    {
        return Click();
    }

    Rect BRect = SizeElement(size);

    Click Result = ButtonInternal(text, size, borderWidth, colour);

    if (!ShouldDraw(BRect))
    {
        return Result;
    }

    //TODO: hard coded text colour
    if (text != "")
    {
        //TODO: come up with better depth testing solution for rendering UI
        m_Renderer.DisableDepthTesting();
        m_Text.DrawText(text, &m_FrameFont, BRect.location + (BRect.size / 2), textColour, Anchor::CENTER);
        m_Renderer.EnableDepthTesting();
    }

    return Result;
}

Click UIModule::ImgButton(std::string name, Texture texture, Vec2f size, float borderWidth, Vec3f colour)
{
    if (!IsActive())
    {
        return Click();
    }

    Rect BRect = SizeElement(size);

    Click Result = ButtonInternal(name, size, borderWidth, colour);

    if (!ShouldDraw(BRect))
    {
        return Result;
    }

    Rect innerRect = BRect;
    innerRect.location.x += borderWidth;
    innerRect.location.y += borderWidth;
    innerRect.size.x -= borderWidth * 2;
    innerRect.size.y -= borderWidth * 2;

    MeshData VertexData = GetVertexDataForRect(innerRect);

    m_Renderer.UpdateMeshData(m_RectMesh, UIElementFormat, VertexData.first, VertexData.second);

    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", c_White);

    m_Renderer.SetActiveTexture(texture.Id, "Texture");

    //TODO: come up with better depth testing solution for rendering UI
    m_Renderer.DrawMesh(m_RectMesh);

    return Result;
}

Click UIModule::BufferButton(std::string name, Framebuffer_ID fBuffer, Vec2f size, float borderWidth, Vec3f colour)
{
    if (!IsActive())
    {
        return Click();
    }

    Rect BRect = SizeElement(size);

    Click Result = ButtonInternal(name, size, borderWidth, colour);

    if (!ShouldDraw(BRect))
    {
        return Result;
    }

    Rect innerRect = BRect;
    innerRect.location.x += borderWidth;
    innerRect.location.y += borderWidth;
    innerRect.size.x -= borderWidth * 2;
    innerRect.size.y -= borderWidth * 2;

    MeshData VertexData = GetVertexDataForRect(innerRect);

    m_Renderer.UpdateMeshData(m_RectMesh, UIElementFormat, VertexData.first, VertexData.second);

    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", c_White);

    m_Renderer.SetActiveFBufferTexture(fBuffer, "Texture");

    //TODO: come up with better depth testing solution for rendering UI
    m_Renderer.DrawMesh(m_RectMesh);

    return Result;
}

void UIModule::Text(std::string text, Vec2f position, Vec3f colour)
{
    position += GetFrame().location;
    m_Text.DrawText(text, &m_FrameFont, position, colour);
}

void UIModule::Text(std::string text, Colour col)
{

}

void UIModule::TextEntry(std::string name, std::string& stringRef, Vec2f size, Vec3f colour)
{
    if (!IsActive())
        return;
   
    TextEntryState* State = GetTextEntryState(name);
    
    Click click = TextButton(stringRef, size, m_ActiveElement == State ? 0.0f : 4.0f, colour);

    if (click)
    {
        m_ActiveElement = State;
    }

    if (m_ActiveElement == State)
    {
        State->focused = true;
    }
    else
    {
        State->focused = false;
    }

    if (State->focused)
    {
        char Character;
        while (m_Input.ConsumeCharacter(Character))
        {
            // Backspace
            if (Character == '\b')
            {
                if (!stringRef.empty())
                {
                    stringRef.pop_back();
                }
            }
            // Escape/Enter
            else if (Character == '\x1b' || Character == '\r')
            {
                State->focused = false;
                if (m_ActiveElement == State)
                {
                    m_ActiveElement = nullptr;
                }
                break;
            }
            else
            {
                stringRef += Character;
            }
        }
    }
}

void UIModule::FloatTextEntry(std::string name, float& floatRef, Vec2f size, Colour colour)
{
}

void UIModule::StartFrame(std::string name, Rect rect, float borderWidth, Vec3f colour)
{
    m_InTabStack.push_back(false);

    if (!IsActive())
        return;

    //rect.location += GetFrame().location;

    m_CurrentTabIndexStack.push_back(0);

    //Engine::DEBUGPrint("Starting frame named " + name);

    FrameState* State = GetFrameState(name);

    State->activeTab = State->activeTabNextTick;

    State->elementOutsideRectBoundsThisFrame = false;
    State->maxElementOffset = 0.0f;

    m_FrameStateStack.push_back(State);

    m_SubRectStack.push(Rect(rect.location + Vec2f(borderWidth, borderWidth), rect.size - Vec2f(borderWidth * 2, borderWidth * 2)));

    CursorStack.push(CursorInfo(rect.location + Vec2f(borderWidth, borderWidth), rect.location + Vec2f(borderWidth, borderWidth)));

    

    // Render
    m_Renderer.DisableDepthTesting();

    MeshData verts = GetVertexDataForBorderMesh(rect, borderWidth);
    
    m_Renderer.SetActiveShader(m_UIShader);
    
    m_Renderer.UpdateMeshData(m_BorderMesh, UIElementFormat, verts.first, verts.second);
    m_Renderer.SetActiveTexture(m_DefaultFrameTexture, "Texture");

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);
    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", colour);

    m_Renderer.DrawMesh(m_BorderMesh);

    verts = GetVertexDataForRect(Rect(rect.location + Vec2f(borderWidth, borderWidth), rect.size - Vec2f(borderWidth * 2, borderWidth * 2)));
    
    m_Renderer.UpdateMeshData(m_RectMesh, UIElementFormat, verts.first, verts.second);

    m_Renderer.SetActiveTexture(m_White, "Texture");

    //m_Renderer.ClearStencilBuffer();
    m_Renderer.StartStencilDrawing(StencilCompareFunc::EQUAL, StencilOperationFunc::INCREMENT, (int)m_FrameStateStack.size() - 1);
    m_Renderer.DrawMesh(m_RectMesh);
    m_Renderer.EndStencilDrawing();

    if (name != "")
    {
        Rect r = rect;
        //TODO: hard coded text colour
        m_Text.DrawText(name, &m_FrameFont, r.location, Vec3f(0.0f, 0.0f, 0.0f));
    }

    m_Renderer.EnableDepthTesting();

    m_Renderer.StartStencilTesting(StencilCompareFunc::EQUAL, (int)m_FrameStateStack.size());
} 

void UIModule::EndFrame()
{
    m_InTabStack.pop_back();

    if (!IsActive())
    {
        return;
    }
    m_Renderer.EndStencilTesting();
    //m_Renderer.ClearStencilBuffer();

    FrameState* frameState = m_FrameStateStack.back();
    //Engine::DEBUGPrint("Ending frame named " + frameState->name);
    Rect FrameRect = m_SubRectStack.top();

    m_CurrentTabIndexStack.pop_back();
    m_SubRectStack.pop();
    m_FrameStateStack.pop_back();
    CursorStack.pop();
    

    if (frameState->elementOutsideRectBoundsThisFrame)
    {
        if (!frameState->scrollingNeeded)
        {
            frameState->verticalOffset = 0;
        }
        frameState->scrollingNeeded = true;

        Rect SliderRect;
        SliderRect.location = FrameRect.location + Vec2f(FrameRect.size.x, 0.0f);
        SliderRect.size = Vec2f(15.0f, FrameRect.size.y);

        FloatSliderInternal(frameState->name + "_Slider", SliderRect, frameState->verticalOffset, 0.0f, frameState->maxElementOffset, true, false);
    }
    else
    {
        frameState->scrollingNeeded = false;
    }
}

void UIModule::StartFrame(std::string name, Vec2f size, float borderWidth, Vec3f colour)
{
    if (!IsActive())
    {
        return;
    }

    Rect FrameRect = PlaceElement(size);

    StartFrame(name, FrameRect, borderWidth, colour);
}

void UIModule::StartTab(std::string text, Vec3f colour)
{
    if (!IsActive())
    {
        return;
    }

    //Engine::DEBUGPrint("Start tab " + text);

    if (m_FrameStateStack.empty())
    {
        return;
    }
    else
    {
        float borderWidth = 20.0f;

        Vec2f buttonSize = Vec2f(c_TabButtonWidth, borderWidth);
        
        FrameState* frameState = m_FrameStateStack.back();
        bool IsActiveTab = frameState->activeTab == m_CurrentTabIndexStack.back();

        if (TextButton(text, buttonSize, 5.0f, IsActiveTab ? colour * 0.75f : colour))
        {
            frameState->activeTabNextTick = m_CurrentTabIndexStack.back();
        }

        Rect CurrentFrame = GetFrame();
        Vec2f NewCursorPos = CurrentFrame.location + Vec2f(0.0f, borderWidth);

        CursorStack.push(CursorInfo(NewCursorPos, NewCursorPos));

        m_CurrentTabIndexStack.back()++;
        m_InTabStack.back() = true;
        //m_InTabStack.top() = true;
    }
    //m_InTab = true;
}

void UIModule::EndTab()
{

    //if (CursorStack.size() == m_InTabStack)

    if (!m_InTabStack.back() && !IsActive())
    {
        return;
    }
    //if (!IsActive())
    //{
    //    return;
    //}
    if (m_FrameStateStack.empty())
    {
        return;
    }

    //Engine::DEBUGPrint("Ending tab");

    m_InTabStack.back() = false;
    CursorStack.pop();
}

void UIModule::FloatSlider(std::string name, Vec2f size, float& outNum, float min, float max, bool vertical, bool drawText, Vec3f colour)
{
    if (!IsActive())
    {
        return;
    }

    Rect TotalSize = PlaceElement(size);

    FloatSliderInternal(name, TotalSize, outNum, min, max, vertical, drawText, colour);
}

void UIModule::OnFrameStart()
{
    ResetAllElementAliveFlags();

    if (m_Input.IsKeyDown(Key::Escape))
    {
        m_ActiveElement = nullptr;
    }

    CursorStack.push(CursorInfo());
    //m_InTabStack.push_back(false);

    m_HashCount = 0;

    m_Renderer.ClearStencilBuffer();
}

void UIModule::OnFrameEnd()
{
    RemoveInactiveElements();

    if (!m_FrameStateStack.empty() || !m_SubRectStack.empty())
    {
        std::string errorString = "Not all UI frames have been closed!";

        //Engine::FatalError(errorString);
    }

    while (!CursorStack.empty())
    {
        CursorStack.pop();
    }
    while (!m_InTabStack.empty())
    {
        m_InTabStack.pop_back();
    }
    while (!m_CurrentTabIndexStack.empty())
    {
        m_CurrentTabIndexStack.pop_back();
    }

    //Engine::DEBUGPrint("~~~~ENDING FRAME~~~~");
}

Click UIModule::ButtonInternal(std::string name, Vec2f size, float borderWidth, Vec3f colour)
{
    if (!IsActive())
    {
        return Click();
    }

    Rect BRect = PlaceElement(size);

    return ButtonInternal(name, BRect, borderWidth, colour);
}

Click UIModule::ButtonInternal(std::string name, Rect rect, float borderWidth, Vec3f colour)
{
    if (!IsActive())
    {
        return Click();
    }

    ButtonState* BState = GetButtonState(name);

    if (!m_SubRectStack.empty())
    {
        BState->m_Click.Update(rect, m_SubRectStack.top());
    }
    else
    {
        BState->m_Click.Update(rect, Rect());
    }

    if (!ShouldDraw(rect))
    {
        return BState->m_Click;
    }

    // Render
    m_Renderer.DisableDepthTesting();
    MeshData vertexData = GetVertexDataForBorderMesh(rect, borderWidth);

    m_Renderer.UpdateMeshData(m_BorderMesh, UIElementFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetActiveTexture(m_DefaultButtonTexture, "Texture");

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", BState->m_Click.hovering);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", BState->m_Click.clicking);
    m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", colour);

    m_Renderer.DrawMesh(m_BorderMesh);

    m_Renderer.EnableDepthTesting();
    // End render

    return BState->m_Click;
}

void UIModule::FloatSliderInternal(std::string name, Rect rect, float& outNum, float min, float max, bool vertical, bool drawText, Vec3f colour)
{
    if (!IsActive())
    {
        return;
    }

    FloatSliderState* SliderState = GetFloatSliderState(name);

    if (SliderState->dragging)
    {
        if (!Engine::GetMouseDown())
        {
            SliderState->dragging = false;
        }
    }

    Rect TotalSize = rect;

    if (ShouldDraw(TotalSize))
    {
        // Render
        m_Renderer.DisableDepthTesting();

        MeshData verts = GetVertexDataForBorderMesh(TotalSize, 2.f);

        m_Renderer.SetActiveShader(m_UIShader);

        m_Renderer.UpdateMeshData(m_BorderMesh, UIElementFormat, verts.first, verts.second);
        m_Renderer.SetActiveTexture(m_DefaultFrameTexture, "Texture");

        m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
        m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);
        m_Renderer.SetShaderUniformVec3f(m_UIShader, "Colour", colour);

        m_Renderer.DrawMesh(m_BorderMesh);

        if (name != "" && drawText)
        {
            //TODO: hard coded text colour
            m_Text.DrawText(name + " = " + std::to_string(outNum), &m_FrameFont, TotalSize.Center(), Vec3f(0.5f, 0.5f, 0.5f), Anchor::CENTER);
            m_Text.DrawText(std::to_string(min), &m_FrameFont, TotalSize.location, Vec3f(0.5f, 0.5f, 0.5f), Anchor::TOP_LEFT);
            m_Text.DrawText(std::to_string(max), &m_FrameFont, TotalSize.location + Vec2f(TotalSize.size.x, 0.0f), Vec3f(0.5f, 0.5f, 0.5f), Anchor::TOP_RIGHT);
        }

        m_Renderer.EnableDepthTesting();
        // End render
    }

    float XSize;

    if (vertical)
    {
        XSize = TotalSize.size.y;
    }
    else
    {
        XSize = TotalSize.size.x;
    }

    float Num = Math::Clamp(outNum, min, max);

    float Val = (Num - min) / (max - min);
    Val = Val * XSize;

    Rect SliderRect;

    if (vertical)
    {
        SliderRect.location = TotalSize.location + Vec2f(0.0f, Val - 5.0f);
        SliderRect.size = Vec2f(TotalSize.size.x, 10.0f);
    }
    else
    {
        SliderRect.location = TotalSize.location + Vec2f(Val - 5.0f, 0.0f);
        SliderRect.size = Vec2f(10.0f, TotalSize.size.y);
    }

    Click SliderClick = ButtonInternal(name + "_Slider", SliderRect, 2.0f, c_NiceBlue);

    bool SliderContainsMouse = InputModule::Get()->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed && TotalSize.Contains(Engine::GetMousePosition());

    if ((SliderClick.clicking || SliderContainsMouse) && !SliderState->dragging)
    {
        // Initial grab of slider
        SliderState->dragging = true;
    }

    if (SliderState->dragging)
    {
        Vec2f MousePos = Engine::GetMousePosition();

        MousePos = MousePos - TotalSize.location;

        float MouseVal;

        if (vertical)
        {
            MouseVal = Math::Remap(
                0.0f, TotalSize.size.y,
                0.0f, 1.0f, MousePos.y
            );
        }
        else
        {
            MouseVal = Math::Remap(
                0.0f, TotalSize.size.x,
                0.0f, 1.0f, MousePos.x
            );
        }

        MouseVal = Math::Clamp(MouseVal, 0.0f, 1.0f);

        outNum = Math::Lerp(min, max, MouseVal);
    }

}

Rect UIModule::SizeElement(Vec2f size)
{
    Rect ElementBounds;

    Rect CurrentFrame = GetFrame();
    CursorInfo& CurrentCursor = CursorStack.top();

    CursorInfo NewCursor = CurrentCursor;

    ElementBounds.size = size;

    if (CurrentCursor.Top.x + ElementBounds.size.x > CurrentFrame.location.x + CurrentFrame.size.x)
    {
        // New horizontal line
        NewCursor.Top.x = CurrentFrame.location.x;
        NewCursor.Top.y = NewCursor.Bottom.y;

        NewCursor.Bottom = NewCursor.Top + Vec2f(0.0f, ElementBounds.size.y);
    }

    if (NewCursor.Top.y + ElementBounds.size.y > NewCursor.Bottom.y)
    {
        NewCursor.Bottom.y = NewCursor.Top.y + ElementBounds.size.y;
    }

    ElementBounds.location = NewCursor.Top;
    NewCursor.Top.x += ElementBounds.size.x;

    if (!m_FrameStateStack.empty())
    {
        FrameState* frameState = m_FrameStateStack.back();

        if (!CurrentFrame.Contains(ElementBounds))
        {
            frameState->elementOutsideRectBoundsThisFrame = true;
            float elementOffset = (ElementBounds.location.y + ElementBounds.size.y) - (CurrentFrame.location.y + CurrentFrame.size.y);
            if (frameState->maxElementOffset < elementOffset) frameState->maxElementOffset = elementOffset;
        }

        if (frameState->scrollingNeeded)
        {
            ElementBounds.location.y -= frameState->verticalOffset;
        }
    }

    return ElementBounds;
}

Rect UIModule::PlaceElement(Vec2f size)
{
    Rect ElementBounds;

    Rect CurrentFrame = GetFrame();
    CursorInfo& CurrentCursor = CursorStack.top();

    ElementBounds.size = size;

    if (CurrentCursor.Top.x + ElementBounds.size.x > CurrentFrame.location.x + CurrentFrame.size.x)
    {
        // New horizontal line
        CurrentCursor.Top.x = CurrentFrame.location.x;
        CurrentCursor.Top.y = CurrentCursor.Bottom.y;

        CurrentCursor.Bottom = CurrentCursor.Top + Vec2f(0.0f, ElementBounds.size.y);
    }

    if (CurrentCursor.Top.y + ElementBounds.size.y > CurrentCursor.Bottom.y)
    {
        CurrentCursor.Bottom.y = CurrentCursor.Top.y + ElementBounds.size.y;
    }

    ElementBounds.location = CurrentCursor.Top;
    CurrentCursor.Top.x += ElementBounds.size.x;

    if (!m_FrameStateStack.empty())
    {
        FrameState* frameState = m_FrameStateStack.back();

        if (!CurrentFrame.Contains(ElementBounds))
        {
            frameState->elementOutsideRectBoundsThisFrame = true;
        }

        if (frameState->scrollingNeeded)
        {
            ElementBounds.location.y -= frameState->verticalOffset;
        }
    }

    return ElementBounds;
}

MeshData UIModule::GetVertexDataForRect(Rect rect)
{
    Vec2i screenSize = Engine::GetClientAreaSize();

    rect.location.y = (screenSize.y - rect.location.y) - rect.size.y;

    Vec2f min = rect.location;
    Vec2f max = rect.location + rect.size;

    std::vector<float> vertices =
    {
        // Position                 // UV
        min.x, min.y,               0.0f, 0.0f,
        min.x, max.y,               0.0f, 1.0f,
        max.x, max.y,               1.0f, 1.0f,
        max.x, min.y,               1.0f, 0.0f,
    };

    std::vector<ElementIndex> indices =
    {
        0, 1, 2, 
        0, 2, 3
    };

    return MeshData(vertices, indices);
}

std::pair<std::vector<float>, std::vector<ElementIndex>> UIModule::GetVertexDataForBorderMesh(Rect rect, float borderWidth)
{
    Vec2i screenSize = Engine::GetClientAreaSize();

    rect.location.y = (screenSize.y - rect.location.y) - rect.size.y;

    Vec2f min = rect.location;
    Vec2f max = rect.location + rect.size;

    float b = borderWidth;

    float t0 = 0.0f;
    float t1 = 1.0f / 3.0f;
    float t2 = 2.0f / 3.0f;

    //  12__13____14__15
    //  |   |     |   |
    //  8___9_____10__11
    //  |   |     |   |
    //  |-b-|     |   |
    //  |   |     |   |
    //  4___5_____6___7
    //  |   |     |   |
    //  0___1_____2___3

    std::vector<float> vertices =
    {
        // Position                 // UV
        min.x, min.y,               0.0f, 0.0f,
        min.x + b, min.y,           t1, 0.0f,
        max.x - b, min.y,           t2, 0.0f,
        max.x, min.y,               1.0f, 0.0f,

        min.x, min.y + b,           0.0f, t1,
        min.x + b, min.y + b,       t1, t1,
        max.x - b, min.y + b,       t2, t1,
        max.x, min.y + b,           1.0f, t1,

        min.x, max.y - b,           0.0f, t2,
        min.x + b, max.y - b,       t1, t2,
        max.x - b, max.y - b,       t2, t2,
        max.x, max.y - b,           1.0f, t2,

        min.x, max.y,               0.0f, 1.0f,
        min.x + b, max.y,           t1, 1.0f,
        max.x - b, max.y,           t2, 1.0f,
        max.x, max.y,               1.0f, 1.0f

    };
    std::vector<ElementIndex> indices =
    {
        0, 4, 5, 0, 5, 1,
        1, 5, 6, 1, 6, 2,
        2, 6, 7, 2, 7, 3,
        4, 8, 9, 4, 9, 5,
        5, 9, 10, 5, 10, 6,
        6, 10, 11, 6, 11, 7,
        8, 12, 13, 8, 13, 9,
        9, 13, 14, 9, 14, 10,
        10, 14, 15, 10, 15, 11
    };

    return MeshData(vertices, indices);
}

ElementID UIModule::GetElementID(std::string name)
{
    // Combine name with per-frame unique number
    return Hash::Combine(Hash::Hash_Value(name), Hash::Hash_Value(m_HashCount++));
}

FrameState* UIModule::GetFrameState(std::string name)
{
    ElementID ID = GetElementID(name);

    auto got = m_FrameStates.find(ID);
    if (got == m_FrameStates.end())
    {
        FrameState newFrameState;
        newFrameState.name = name;
        m_FrameStates.emplace(ID, newFrameState);
    }

    m_FrameStates[ID].m_Alive = true;

    return &m_FrameStates[ID];
}

ButtonState* UIModule::GetButtonState(std::string name)
{
    ElementID ID = GetElementID(name);

    auto got = m_ButtonStates.find(ID);
    if (got == m_ButtonStates.end())
    {
        m_ButtonStates.emplace(ID, ButtonState());
    }

    m_ButtonStates[ID].m_Alive = true;

    return &m_ButtonStates[ID];
}

TextEntryState* UIModule::GetTextEntryState(std::string name)
{
    ElementID ID = GetElementID(name);

    auto got = m_TextEntryStates.find(ID);
    if (got == m_TextEntryStates.end())
    {
        m_TextEntryStates.emplace(ID, TextEntryState());
    }

    m_TextEntryStates[ID].m_Alive = true;

    return &m_TextEntryStates[ID];
}

FloatSliderState* UIModule::GetFloatSliderState(std::string name)
{
    ElementID ID = GetElementID(name);

    auto got = m_FloatSliderStates.find(ID);
    if (got == m_FloatSliderStates.end())
    {
        m_FloatSliderStates.emplace(ID, FloatSliderState());
    }

    m_FloatSliderStates[ID].m_Alive = true;

    return &m_FloatSliderStates[ID];
}

Rect UIModule::GetFrame()
{
    if (m_SubRectStack.empty())
    {
        return Rect(Vec2f(0.0f, 0.0f), m_WindowSize);
    }
    else
    {
        return m_SubRectStack.top();
    }
}

bool UIModule::IsActive()
{
    if (m_FrameStateStack.empty())
    {
        // We're not inside a frame element, so we're always active
        return true;
    }
    //else if (m_InTabStack.size() == 1 && !m_InTabStack.back())
    //{
    //    // We're not in a tab, so always display (add more elements maybe later)
    //    return true;
    //}
    else
    {
        for (int i = 0; i < (m_InTabStack.size() < m_FrameStateStack.size() ? m_InTabStack.size() : m_FrameStateStack.size()); i++)
        {
            if (m_InTabStack[i] && m_FrameStateStack[i]->activeTab != m_CurrentTabIndexStack[i] - 1)
            {
                return false;
            }
        }

        return true;

        //// Get the state of the frame element we're in now
        //FrameState* frameState = m_FrameStateStack.back();

        //if (frameState->activeTab == m_TabIndexStack.back() - 1)
        //{
        //    return true;
        //}
        //else
        //{
        //    return false;
        //}
    }
}

bool UIModule::ShouldDraw(Rect rectToDraw)
{
    if (!IsActive())
    {
        return false;
    }

    if (m_SubRectStack.empty())
    {
        return true;
    }

    if (m_SubRectStack.top().Overlaps(rectToDraw))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void UIModule::ResetAllElementAliveFlags()
{
    for (auto& Frame : m_FrameStates)
    {
        Frame.second.m_Alive = false;
    }
    for (auto& Button : m_ButtonStates)
    {
        Button.second.m_Alive = false;
    }
    for (auto& TextEntry : m_TextEntryStates)
    {
        TextEntry.second.m_Alive = false;
    }
}

void UIModule::RemoveInactiveElements()
{
    for (auto it = m_FrameStates.begin(); it != m_FrameStates.end();)
    {
        if (!it->second.m_Alive)
            it = m_FrameStates.erase(it);
        else
            ++it;
    }
    for (auto it = m_ButtonStates.begin(); it != m_ButtonStates.end();)
    {
        if (!it->second.m_Alive)
            it = m_ButtonStates.erase(it);
        else
            ++it;
    }
    for (auto it = m_TextEntryStates.begin(); it != m_TextEntryStates.end();)
    {
        if (!it->second.m_Alive)
            it = m_TextEntryStates.erase(it);
        else
            ++it;
    }
}

void UIModule::Resize(Vec2i newSize)
{
    m_WindowSize = newSize;

    Mat4x4f orthoMatrix = Math::GenerateOrthoMatrix(0.0f, (float)newSize.x, 0.0f, (float)newSize.y, 0.0f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_UIShader, "Projection", orthoMatrix);
}