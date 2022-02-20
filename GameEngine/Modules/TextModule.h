#pragma once

#include "..\Platform\RendererPlatform.hpp"
#include <map>

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;

struct CharacterInfo {
    Vec2i Advance;          // Offset to advance to next glyph
    Vec2i Size;             // Size of glyph
    Vec2i Bearing;          // Offset from baseline to left/top of glyph
    float AtlasOffset;      // Horizontal offset into texture atlas
};

struct Font
{
    std::map<char, CharacterInfo> m_CharacterInfo;

    Texture_ID m_TextureAtlas;
    Vec2i m_AtlasSize;
};

class TextModule
{
public:
    TextModule(Renderer& renderer);
    ~TextModule();

    Font LoadFont(std::string filePath, int pixelSize);

    // Screen-space only for now!!!!
    void DrawText(std::string text, Font* font, Vec2f position, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

private:

    FT_Library m_Library;
    Renderer& m_Renderer;

    Shader_ID m_TextShader;
    Mesh_ID m_TextQuadsMesh;
};

