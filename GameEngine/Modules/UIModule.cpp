#include "UIModule.h"

void Click::Update(Rect bounds)
{
    Vec2i mousePos = Engine::GetMousePosition();

    if (clicked)
    {
        clicked = false;
    }

    if (clicking)
    {
        if (!bounds.contains(mousePos))
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
        if (!Engine::GetMouseDown() && bounds.contains(mousePos))
        {
            hovering = true;
        }
        if (!bounds.contains(mousePos))
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
{
    m_DefaultBorderTexture = m_Renderer.LoadTexture("images/button_border.png");
    m_DefaultFrameTexture = m_Renderer.LoadTexture("images/frame_border.png");

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
    
    smooth in vec2 FragUV;	

    out vec4 OutColour;

    void main()
    {
        vec4 textureAt = texture(Texture, FragUV);
        OutColour = textureAt;


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

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });
    m_BorderMesh = m_Renderer.CreateEmptyMesh(vertFormat, true);
    m_RectMesh = m_Renderer.CreateEmptyMesh(vertFormat, true);

    m_FrameFont = m_Text.LoadFont("fonts/ARLRDBD.TTF", 12);
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
    if (!ShouldDisplay())
        return;

    rect.location += GetFrame().location;

    m_Renderer.DisableDepthTesting();

    MeshData vertexData = GetVertexDataForRect(rect);

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });

    m_Renderer.UpdateMeshData(m_RectMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(texture.Id, "Texture");
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);

    m_Renderer.DrawMesh(m_RectMesh);

    m_Renderer.EnableDepthTesting();
}

void UIModule::BufferPanel(Framebuffer_ID fBuffer, Rect rect)
{
    if (!ShouldDisplay())
        return;

    rect.location += GetFrame().location;
    
    m_Renderer.DisableDepthTesting();

    MeshData vertexData = GetVertexDataForRect(rect);

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });

    m_Renderer.UpdateMeshData(m_RectMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveFBufferTexture(fBuffer, "Texture");

    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);

    m_Renderer.DrawMesh(m_RectMesh);

    m_Renderer.EnableDepthTesting();
}

Click UIModule::TextButton(std::string text, Rect rect, float borderWidth)
{
    return Button(0, rect, borderWidth, false, false, true, text);
}

Click UIModule::ImgButton(Texture texture, Rect rect, float borderWidth)
{
    return Button(texture.Id, rect, borderWidth, true, false, false);
}

Click UIModule::BufferButton(Framebuffer_ID fBuffer, Rect rect, float borderWidth)
{
    return Button(fBuffer, rect, borderWidth, true, true, false);
}

void UIModule::Text(std::string text, Vec2f position, Vec3f colour)
{
    position += GetFrame().location;
    m_Text.DrawText(text, &m_FrameFont, position, colour);
}

void UIModule::TextEntry(std::string& stringRef, Rect rect)
{
    if (!ShouldDisplay())
        return;

    TextEntryState* State = GetTextEntryState(rect);
    
    Click click = TextButton(stringRef, rect, 0.0f);

    //if (click.clicked)
    //{
    //    State->focused = !State->focused;
    //}

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
            // Escape
            else if (Character == '\x1b')
            {
                State->focused = false;
                break;
            }
            else
            {
                stringRef += Character;
            }
        }
    }
}

void UIModule::StartFrame(Rect rect, float borderWidth, std::string text)
{
    if (!ShouldDisplay())
        return;

    m_Renderer.DisableDepthTesting();

    m_SubFrameStack.push(FrameInfo(Rect(rect.location + Vec2f(borderWidth, borderWidth), rect.size - Vec2f(borderWidth, borderWidth)), borderWidth, text));

    GetFrameState(m_SubFrameStack.top());

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });
    MeshData verts = GetVertexDataForBorderMesh(rect, borderWidth);
    
    m_Renderer.UpdateMeshData(m_BorderMesh, vertFormat, verts.first, verts.second);
    m_Renderer.SetActiveTexture(m_DefaultFrameTexture, "Texture");
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);

    m_Renderer.DrawMesh(m_BorderMesh);

    if (text != "")
    {
        Rect r = rect;
        //TODO: hard coded text colour
        m_Text.DrawText(text, &m_FrameFont, r.location, Vec3f(1.0f, 1.0f, 1.0f));
    }

    m_Renderer.EnableDepthTesting();
} 

void UIModule::EndFrame()
{
    m_SubFrameStack.pop();
    m_TabIndexOnCurrentFrame = 0;
    //m_SubFrame = Rect(Vec2f(0.0f, 0.0f), m_WindowSize);
}

void UIModule::StartTab(std::string text)
{
    if (m_SubFrameStack.empty())
    {
        Engine::FatalError("Tabs must be inside frames.");
        return;
    }
    else
    {
        float borderWidth = m_SubFrameStack.top().m_BorderWidth;

        Rect buttonRect = Rect(Vec2f(c_TabButtonWidth * m_TabIndexOnCurrentFrame, -borderWidth), Vec2f(c_TabButtonWidth, borderWidth));
        
        if (TextButton(text, buttonRect, 5.0f))
        {
            FrameState* frameState = GetFrameState(m_SubFrameStack.top());
            frameState->activeTab = m_TabIndexOnCurrentFrame;
        }
    
        m_TabIndexOnCurrentFrame++;
    }
    m_InTab = true;
}

void UIModule::EndTab()
{
    m_InTab = false;
}

void UIModule::OnFrameStart()
{
    ResetAllElementAliveFlags();

    m_HashCount = 0;
}

void UIModule::OnFrameEnd()
{
    RemoveInactiveElements();

    if (!m_SubFrameStack.empty())
    {
        std::vector<std::string> frameNames;

        std::string errorString = "Not all UI frames have been closed!\nOpen frames:\n";

        // Note: Destroys frame stack. Since it's a fatal error that shouldn't matter.
        while (!m_SubFrameStack.empty())
        {
            std::string frameName = m_SubFrameStack.top().m_Name;
            if (frameName == "")
            {
                errorString += "<Unnamed Frame>\n";
            }
            else
            {
                errorString += frameName + "\n";
            }
            m_SubFrameStack.pop();
        }

        Engine::FatalError(errorString);
    }
}

Click UIModule::Button(unsigned int img, Rect rect, float borderWidth, bool hasImage, bool isBuffer, bool hasText, std::string text)
{
    if (!ShouldDisplay())
        return Click();
    
    rect.location += GetFrame().location;
    ButtonState* buttonState = GetButtonState(rect, borderWidth);

    Click result;

    Vec2i mousePos = Engine::GetMousePosition();

    if (buttonState->clicking)
    {
        if (!rect.contains(mousePos))
        {
            // Mouse moved out of button while clicking, button not clicked
            buttonState->clicking = false;
            result.clicked = false;
        }
        if (!Engine::GetMouseDown())
        {
            // Mouse stopped clicking while on button, button was clicked
            buttonState->clicking = false;
            result.clicked = true;
        }
    }
    else
    {
        if (Engine::GetMouseDown() && buttonState->hovering)
        {
            buttonState->clicking = true;
        }
        if (!Engine::GetMouseDown() && rect.contains(mousePos))
        {
            buttonState->hovering = true;
        }
        if (!rect.contains(mousePos))
        {
            buttonState->hovering = false;
        }
    }

    m_Renderer.DisableDepthTesting();
    MeshData vertexData = GetVertexDataForBorderMesh(rect, borderWidth);

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });

    m_Renderer.UpdateMeshData(m_BorderMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(m_DefaultBorderTexture, "Texture");
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", buttonState->hovering);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", buttonState->clicking);

    m_Renderer.DrawMesh(m_BorderMesh);

    Rect innerRect = rect;
    innerRect.location.x += borderWidth;
    innerRect.location.y += borderWidth;
    innerRect.size.x -= borderWidth * 2;
    innerRect.size.y -= borderWidth * 2;

    vertexData = GetVertexDataForRect(innerRect);

    m_Renderer.UpdateMeshData(m_RectMesh, vertFormat, vertexData.first, vertexData.second);

    if (hasImage)
    {
        if (isBuffer)
        {
            m_Renderer.SetActiveFBufferTexture(img, "Texture");
        }
        else
        {
            m_Renderer.SetActiveTexture(img, "Texture");
        }
        m_Renderer.DrawMesh(m_RectMesh);
    }

    if (hasText)
    {
        //TODO: hard coded text colour
        m_Text.DrawText(text, &m_FrameFont, rect.location + (rect.size / 2), Vec3f(1.0f, 1.0f, 1.0f), Anchor::CENTER);
    }

    

    m_Renderer.EnableDepthTesting();

    result.clicking = buttonState->clicking;
    result.hovering = buttonState->hovering;

    return result;
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

ButtonState* UIModule::GetButtonState(Rect rect, float borderWidth)
{
    ButtonInfo bi = ButtonInfo{ rect, borderWidth };
    
    auto got = m_Buttons.find(bi);

    if (got == m_Buttons.end())
    {
        ButtonState newState;
        m_Buttons[bi] = newState;
    }

    m_Buttons[bi].m_Alive = true;

    rect.location += GetFrame().location;
    m_Buttons[bi].m_Click.Update(rect);

    if (m_Buttons[bi].m_Click)
    {
        m_ActiveElement = &m_Buttons[bi];
        Engine::DEBUGPrint("Changed active element, clicked button");
    }

    return &m_Buttons[bi];
}

TextEntryState* UIModule::GetTextEntryState(Rect rect)
{
    TextEntryInfo ti = TextEntryInfo(rect);

    auto got = m_TextEntries.find(ti);

    if (got == m_TextEntries.end())
    {
        TextEntryState newState;
        m_TextEntries[ti] = newState;
    }

    m_TextEntries[ti].m_Alive = true;

    rect.location += GetFrame().location;
    m_TextEntries[ti].m_Click.Update(rect);

    if (m_TextEntries[ti].m_Click)
    {
        m_ActiveElement = &m_TextEntries[ti];
        Engine::DEBUGPrint("Changed active element, clicked text entry");
    }

    return &m_TextEntries[ti];
}

FrameState* UIModule::GetFrameState(FrameInfo& fInfo)
{
    return GetFrameState(fInfo.m_Rect, fInfo.m_BorderWidth, fInfo.m_Name);
}

FrameState* UIModule::GetFrameState(Rect rect, float borderWidth, std::string name)
{
    FrameInfo fi = FrameInfo(rect, borderWidth, name);

    auto got = m_Frames.find(fi);

    if (got == m_Frames.end())
    {
        FrameState newState;
        m_Frames[fi] = newState;
    }

    m_Frames[fi].m_Alive = true;

    //rect.location += GetFrame().location;
    m_Frames[fi].m_Click.Update(rect);

    if (m_Frames[fi].m_Click)
    {
        m_ActiveElement = &m_Frames[fi];
        Engine::DEBUGPrint("Changed active element, clicked frame " + name);
    }

    return &m_Frames[fi];
}

Rect UIModule::GetFrame()
{
    if (m_SubFrameStack.empty())
    {
        return Rect(Vec2f(0.0f, 0.0f), m_WindowSize);
    }
    else
    {
        return m_SubFrameStack.top().m_Rect;
    }
}

bool UIModule::ShouldDisplay()
{
    if (m_SubFrameStack.empty())
    {
        // We're not inside a frame element, so always display
        return true;
    }
    else if (!m_InTab)
    {
        // We're not in a tab, so always display (add more elements maybe later)
        return true;
    }
    else
    {
        // Get the state of the frame element we're in now
        FrameState* frameState = GetFrameState(m_SubFrameStack.top());

        if (frameState->activeTab == m_TabIndexOnCurrentFrame - 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void UIModule::ResetAllElementAliveFlags()
{
    for (auto& Button : m_Buttons)
    {
        Button.second.m_Alive = false;
    }
    for (auto& Frame : m_Frames)
    {
        Frame.second.m_Alive = false;
    }
    for (auto& TextEntry : m_TextEntries)
    {
        TextEntry.second.m_Alive = false;
    }
}

void UIModule::RemoveInactiveElements()
{
    for (auto it = m_Buttons.begin(); it != m_Buttons.end();)
    {
        if (!it->second.m_Alive)
            it = m_Buttons.erase(it);
        else
            ++it;
    }
    for (auto it = m_Frames.begin(); it != m_Frames.end();)
    {
        if (!it->second.m_Alive)
            it = m_Frames.erase(it);
        else
            ++it;
    }
    for (auto it = m_TextEntries.begin(); it != m_TextEntries.end();)
    {
        if (!it->second.m_Alive)
            it = m_TextEntries.erase(it);
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
