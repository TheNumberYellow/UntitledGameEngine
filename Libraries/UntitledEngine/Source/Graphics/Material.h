#pragma once

#include <json.hpp>

using json = nlohmann::json;

class Texture;

struct Material
{
    Material() {}
    Material(Texture* Albedo, Texture* Normal, Texture* Roughness, Texture* Metallic, Texture* AO);

    Texture* m_Albedo;
    Texture* m_Normal;
    Texture* m_Metallic;
    Texture* m_Roughness;
    Texture* m_AO;
    Texture* m_Height;

    void Save(json& JsonObject);
    static Material Load(json& JsonObject);

    friend bool operator<(const Material& lhs, const Material& rhs);

    friend bool operator==(const Material& lhs, const Material& rhs);
};