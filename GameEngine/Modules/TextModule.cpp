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
    uniform vec2 TextPosition;    

    in vec2 VertPosition;
    in vec2 VertUV;

    smooth out vec2 FragUV;

    void main()
    {
        //TEMP (fraser) z ordering for ui elements
        gl_Position = (Projection * vec4(VertPosition, -0.9, 1.0)) + (Projection * vec4(TextPosition, 0, 0));
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

    Vec2i viewportSize = m_Renderer.GetViewportSize();

    Mat4x4f orthoMatrix = Math::GenerateOrthoMatrix(0.0f, (float)viewportSize.x, 0.0f, (float)viewportSize.y, 0.0f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_TextShader, "Projection", orthoMatrix);


    //TextStringInfo tsf;
    //tsf.m_String = "Yeetus";
    //tsf.m_Bounds = Rect(Vec2f(2.0f, 3.0f), Vec2f(40.0f, 50.0f));
    //tsf.m_Mesh = 123;


    //size_t tsfHash = Hash_Value(tsf);

    //Engine::DEBUGPrint(std::to_string(tsfHash));


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

    TextMeshInfo meshInfo = GetTextMeshInfo(text, *font);
    
    m_Renderer.SetShaderUniformVec3f(m_TextShader, "TextColour", colour);
    
    // Temp(fraser): set up "anchor points" for text
    position.y -= meshInfo.m_Bounds.size.y;
    m_Renderer.SetShaderUniformVec2f(m_TextShader, "TextPosition", position);

    m_Renderer.SetActiveTexture(font->m_TextureAtlas, "Texture");
    
    m_Renderer.DrawMesh(meshInfo.m_Mesh);
}

void TextModule::Resize(Vec2i newSize)
{
    Mat4x4f orthoMatrix = Math::GenerateOrthoMatrix(0.0f, (float)newSize.x, 0.0f, (float)newSize.y, 0.0f, 100.0f);
    m_Renderer.SetShaderUniformMat4x4f(m_TextShader, "Projection", orthoMatrix);

}

TextMeshInfo TextModule::GetTextMeshInfo(std::string text, Font& font)
{
    TextInfo ti = TextInfo{ text, &font };

    auto got = m_CachedStrings.find(ti);

    if (got == m_CachedStrings.end())
    {
        TextMeshInfo meshInfo = GenerateTextMeshInfo(text, font);
        m_CachedStrings[ti] = meshInfo;
    }

    return m_CachedStrings[ti];
}

TextMeshInfo TextModule::GenerateTextMeshInfo(std::string text, Font& font)
{
    TextMeshInfo textInfo;
    textInfo.m_Font = &font;
    textInfo.m_String = text;
    
    VertexBufferFormat vertFormat = { VertAttribute::Vec2f, VertAttribute::Vec2f };

    std::vector<float> textQuadsVertices;
    std::vector<ElementIndex> indexBuffer;

    Vec2i fontAtlasSize = textInfo.m_Font->m_AtlasSize;

    Vec2f cursor = Vec2f(0.0f, 0.0f);

    for (int i = 0; i < text.size(); ++i)
    {
        CharacterInfo c = textInfo.m_Font->m_CharacterInfo[text[i]];

        float xpos = cursor.x + c.Bearing.x;
        float ypos = cursor.y - (c.Size.y - c.Bearing.y);

        float w = (float)c.Size.x;
        float h = (float)c.Size.y;

        textInfo.m_Bounds.expand(Vec2f(xpos, ypos));
        textInfo.m_Bounds.expand(Vec2f(xpos, ypos + h));
        textInfo.m_Bounds.expand(Vec2f(xpos + w, ypos + h));
        textInfo.m_Bounds.expand(Vec2f(xpos + w, ypos));

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

        cursor.x += c.Advance.x >> 6;
        cursor.y += c.Advance.y >> 6;
    }

    textInfo.m_Mesh = m_Renderer.LoadMesh(vertFormat, textQuadsVertices, indexBuffer);

    return textInfo;
}
