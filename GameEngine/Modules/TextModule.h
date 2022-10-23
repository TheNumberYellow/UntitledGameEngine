#pragma once

#include "Platform\RendererPlatform.h"
#include "Interfaces\Resizeable_i.h"

#include "Utils\Hash.h"

#include <unordered_map>

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_* FT_Library;

enum class Anchor
{
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};


struct CharacterInfo {
    Vec2i Advance;          // Offset to advance to next glyph
    Vec2i Size;             // Size of glyph
    Vec2i Bearing;          // Offset from baseline to left/top of glyph
    float AtlasOffset;      // Horizontal offset into texture atlas
};

struct Font
{
    std::unordered_map<char, CharacterInfo> m_CharacterInfo;

    Texture_ID m_TextureAtlas;
    Vec2i m_AtlasSize;
};

struct TextInfo
{
    std::string m_String;
    Font* m_Font;

    friend size_t Hash_Value(const TextInfo& ti)
    {
        size_t h = Hash::Hash_Value(ti.m_String);
        return Hash::Combine(h, Hash::Hash_Value(ti.m_Font));
    }

    friend bool operator==(const TextInfo& lhs, const TextInfo& rhs)
    {
        return (lhs.m_String == rhs.m_String) && (lhs.m_Font == rhs.m_Font);
    }
};

struct TextMeshInfo
{
    std::string m_String;
    Rect m_Bounds;
    Mesh_ID m_Mesh;
    Font* m_Font;
};

class TextModule : public IResizeable
{
public:
    TextModule(Renderer& renderer);
    ~TextModule();

    Font LoadFont(std::string filePath, int pixelSize);

    // Screen-space only for now!!!!
    void DrawText(std::string text, Font* font, Vec2f position, Vec3f colour = Vec3f(1.0f, 1.0f, 1.0f));

    // Inherited via IResizeable
    virtual void Resize(Vec2i newSize) override;
private:

    std::unordered_map<TextInfo, TextMeshInfo, Hash::Hasher<TextInfo>> m_CachedStrings;

    TextMeshInfo GetTextMeshInfo(std::string text, Font& font);
    TextMeshInfo GenerateTextMeshInfo(std::string text, Font& font);

    FT_Library m_Library;
    Renderer& m_Renderer;

    Shader_ID m_TextShader;

};

