#include "TextModule.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>

TextModule::TextModule(Renderer& renderer)
    : m_Renderer(renderer)
{
    FT_Error error = FT_Init_FreeType(&m_Library);

    if (error)
    {
        Engine::FatalError("FreeType could not initialize.");
    }

    // Load my special little text shader ^_^
    std::string vertShaderSource = R"(
    #version 400
    
    uniform mat4 Projection;
    
    in vec2 VertPosition;
    in vec2 VertUV;

    smooth out vec2 FragUV;

    void main()
    {
        //TEMP (fraser) z ordering for ui elements
        gl_Position = Projection * vec4(VertPosition, -0.9, 1.0);
        FragUV = VertUV;
    }
    )";

    std::string fragShaderSource = R"(
    #version 400
    
    uniform sampler2D Texture;
    uniform vec3 TextColour;
    
    smooth in vec2 FragUV;
    out vec4 OutColour;

    void main()
    {
        //OutColour = vec4(1.0);
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(Texture, FragUV).r);
        OutColour = vec4(TextColour, 1.0) * sampled;
    }
    )";

    m_TextShader = m_Renderer.LoadShader(vertShaderSource, fragShaderSource);

    VertexBufferFormat vertFormat = { VertAttribute::Vec2f, VertAttribute::Vec2f };

    m_TextQuadsMesh = m_Renderer.CreateEmptyMesh(vertFormat, true);
}

TextModule::~TextModule()
{
    FT_Done_FreeType(m_Library);
} 

Font TextModule::LoadFont(std::string filePath, int pixelSize)
{
    FT_Face newFace;

    FT_Error error = FT_New_Face(m_Library, filePath.c_str(), 0, &newFace);

    if (error)
    {
        Engine::FatalError("Could not load font " + filePath);
    }

    error = FT_Set_Pixel_Sizes(newFace, 0, pixelSize);
    if (error)
    {
        Engine::FatalError("Could not load font " + filePath + " at pixel size " + std::to_string(pixelSize));
    }

    Font newFont;
    Vec2i fontAtlasSize = Vec2i(0, 0);

    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(newFace, c, FT_LOAD_RENDER))
        {
            Engine::DEBUGPrint("Could not load font character " + c);
            continue;
        }
        
        fontAtlasSize.x += newFace->glyph->bitmap.width;
        fontAtlasSize.y = std::max((int)newFace->glyph->bitmap.rows, fontAtlasSize.y);
    }

    newFont.m_TextureAtlas = m_Renderer.CreateEmptyTexture(fontAtlasSize, ColourFormat::Red);
    newFont.m_AtlasSize = fontAtlasSize;

    int currentXPos = 0;
    for (unsigned char c = 0; c < 128; ++c)
    {
        if (FT_Load_Char(newFace, c, FT_LOAD_RENDER))
            continue;

        // Insert character glyph into font atlas
        Recti glyphRect;
        glyphRect.location = Vec2i(currentXPos, 0);
        glyphRect.size = Vec2i(newFace->glyph->bitmap.width, newFace->glyph->bitmap.rows);

        std::vector<unsigned char> glyphData;
        unsigned char* bufferPtr = newFace->glyph->bitmap.buffer;

        for (int i = 0; i < glyphRect.size.x * glyphRect.size.y; ++i)
        {
            glyphData.push_back(*bufferPtr);
            bufferPtr++;
        }

        //newFont.m_TextureAtlas = m_Renderer.LoadTexture(glyphRect.size, glyphData, ColourFormat::Red);
        m_Renderer.UpdateTextureData(newFont.m_TextureAtlas, glyphRect, glyphData, ColourFormat::Red);

        // Insert character info into font map
        CharacterInfo newChar =
        {
            Vec2i(newFace->glyph->advance.x, newFace->glyph->advance.y),
            Vec2i(newFace->glyph->bitmap.width, newFace->glyph->bitmap.rows),
            Vec2i(newFace->glyph->bitmap_left, newFace->glyph->bitmap_top),
            (float)currentXPos / fontAtlasSize.x
        };

        newFont.m_CharacterInfo[c] = newChar;

        currentXPos += newFace->glyph->bitmap.width;
    }

    FT_Done_Face(newFace);

    return newFont;
}

void TextModule::DrawText(std::string text, Font* font, Vec2f position, Vec3f colour)
{
    m_Renderer.SetActiveShader(m_TextShader);
    
    m_Renderer.SetShaderUniformVec3f(m_TextShader, "TextColour", colour);

    Vec2f screenSize = Engine::GetClientAreaSize();
    // Temp(fraser) : connect to camera in some way?

    Vec2i viewportSize = m_Renderer.GetViewportSize();

    Mat4x4f orthoMatrix = Math::GenerateOrthoMatrix(0.0f, (float)viewportSize.x, 0.0f, (float)viewportSize.y, 0.0f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_TextShader, "Projection", orthoMatrix);

    VertexBufferFormat vertFormat = { VertAttribute::Vec2f, VertAttribute::Vec2f };

    std::vector<float> textQuadsVertices;
    std::vector<ElementIndex> indexBuffer;

    Vec2i fontAtlasSize = font->m_AtlasSize;

    for (int i = 0; i < text.size(); ++i)
    {
        CharacterInfo c = font->m_CharacterInfo[text[i]];

        float xpos = position.x + c.Bearing.x;
        float ypos = position.y - (c.Size.y - c.Bearing.y);

        float w = (float)c.Size.x;
        float h = (float)c.Size.y;

        textQuadsVertices.insert(
            textQuadsVertices.end(),
            {
                xpos,     ypos,       c.AtlasOffset,                                            (float)c.Size.y / fontAtlasSize.y,
                xpos,     ypos + h,   c.AtlasOffset,                                            0.0f,
                xpos + w, ypos + h,   c.AtlasOffset + ((float)c.Size.x / fontAtlasSize.x),      0.0f,
                xpos + w, ypos,       c.AtlasOffset + ((float)c.Size.x / fontAtlasSize.x),      (float)c.Size.y / fontAtlasSize.y,
            }
        );

        unsigned int e = 4 * i;
        indexBuffer.insert(
            indexBuffer.end(), 
            { e, e + 1, e + 2, e, e + 2, e + 3 }
        );

        position.x += c.Advance.x >> 6;
        position.y += c.Advance.y >> 6;
    }
    m_Renderer.UpdateMeshData(m_TextQuadsMesh, vertFormat, textQuadsVertices, indexBuffer);

    m_Renderer.SetActiveTexture(font->m_TextureAtlas, 0);
    m_Renderer.DrawMesh(m_TextQuadsMesh);
}
