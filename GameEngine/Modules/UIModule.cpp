#include "UIModule.h"

UIModule::UIModule(Renderer& renderer)
    : m_Renderer(renderer)
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

    m_UIShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    Resize(m_Renderer.GetViewportSize());

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });
    m_BorderMesh = m_Renderer.CreateEmptyMesh(vertFormat, true);
    m_RectMesh = m_Renderer.CreateEmptyMesh(vertFormat, true);
}

UIModule::~UIModule()
{
}

void UIModule::AlignLeft()
{
    m_LeftAligned = true;
}

void UIModule::AlightRight()
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

void UIModule::ImgPanel(Texture_ID texture, Rect rect)
{
    MeshData vertexData = GetVertexDataForRect(rect);

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });

    m_Renderer.UpdateMeshData(m_RectMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(texture);
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);

    m_Renderer.DrawMesh(m_RectMesh);
}

void UIModule::BufferPanel(Framebuffer_ID fBuffer, Rect rect)
{
    MeshData vertexData = GetVertexDataForRect(rect);

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });

    m_Renderer.UpdateMeshData(m_RectMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveFBufferTexture(fBuffer);

    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);

    m_Renderer.DrawMesh(m_RectMesh);
}

bool UIModule::ImgButton(Texture_ID texture, Rect rect, float borderWidth)
{
    ClickableState* state = GetClickable(
        std::to_string(texture) +
        std::to_string(rect.location.x) +
        std::to_string(rect.location.y) +
        std::to_string(rect.size.x) +
        std::to_string(rect.size.y) +
        std::to_string(borderWidth) +
        std::to_string(m_HashCount++)
    );

    Vec2i mousePos = Engine::GetMousePosition();
    mousePos.y = Engine::GetClientAreaSize().y - mousePos.y;

    if (state->clicking)
    {
        if (!rect.contains(mousePos))
        {
            // Mouse moved out of button while clicking, button not clicked
            state->clicking = false;
            return false;
        }
        if (!Engine::GetMouseDown())
        {
            // Mouse stopped clicking while on button, button was clicked
            state->clicking = false;
            return true;
        }
    }
    else 
    {
        if (Engine::GetMouseDown() && state->hovering)
        {
            state->clicking = true;
        }
        state->hovering = rect.contains(mousePos);
    }

    //state->hovering = rect.contains(mousePos);

    m_Renderer.DisableDepthTesting();
    MeshData vertexData = GetVertexDataForBorderMesh(rect, borderWidth);

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });

    m_Renderer.UpdateMeshData(m_BorderMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(m_DefaultBorderTexture);
    m_Renderer.SetActiveShader(m_UIShader);
    
    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", state->hovering);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", state->clicking);
    
    m_Renderer.DrawMesh(m_BorderMesh);

    Rect innerRect = rect;
    innerRect.location.x += borderWidth;
    innerRect.location.y += borderWidth;
    innerRect.size.x -= borderWidth * 2;
    innerRect.size.y -= borderWidth * 2;

    vertexData = GetVertexDataForRect(innerRect);

    m_Renderer.UpdateMeshData(m_RectMesh, vertFormat, vertexData.first, vertexData.second);

    m_Renderer.SetActiveTexture(texture);

    m_Renderer.DrawMesh(m_RectMesh);

    m_Renderer.EnableDepthTesting();

    return false;
}

void UIModule::StartFrame(Rect rect, float borderWidth)
{
    m_Renderer.DisableDepthTesting();

    VertexBufferFormat vertFormat = VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f });
    MeshData verts = GetVertexDataForBorderMesh(rect, borderWidth);
    
    m_Renderer.UpdateMeshData(m_BorderMesh, vertFormat, verts.first, verts.second);
    m_Renderer.SetActiveTexture(m_DefaultFrameTexture);
    m_Renderer.SetActiveShader(m_UIShader);

    m_Renderer.SetShaderUniformBool(m_UIShader, "Hovering", false);
    m_Renderer.SetShaderUniformBool(m_UIShader, "Clicking", false);

    m_Renderer.DrawMesh(m_BorderMesh);

    m_Renderer.EnableDepthTesting();

}

void UIModule::OnFrameStart()
{
    m_HashCount = 0;
}

void UIModule::OnFrameEnd()
{
}

MeshData UIModule::GetVertexDataForRect(Rect rect)
{
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

ClickableState* UIModule::GetClickable(std::string value)
{
    size_t hValue = std::hash<std::string>{}(value);

    ClickableState* state = &m_Clickables[hValue];

    return state;
}

void UIModule::Resize(Vec2i newSize)
{
    //Vec2i viewportSize = m_Renderer.GetViewportSize();

    Mat4x4f orthoMatrix = Math::GenerateOrthoMatrix(0.0f, (float)newSize.x, 0.0f, (float)newSize.y, 0.0f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_UIShader, "Projection", orthoMatrix);
}

