#include "Material.h"

void Material::Save(json& JsonObject)
{
    JsonObject[0] = m_Albedo->Path.GetRelativePath();
    JsonObject[1] = m_Normal->Path.GetRelativePath();
    JsonObject[2] = m_Roughness->Path.GetRelativePath();
    JsonObject[3] = m_Metallic->Path.GetRelativePath();
    JsonObject[4] = m_AO->Path.GetRelativePath();
}

Material Material::Load(json& JsonObject)
{
    std::string albedoPath = JsonObject[0].get<std::string>();
    std::string normalPath = JsonObject[1].get<std::string>();
    std::string roughPath = JsonObject[2].get<std::string>();
    std::string metalPath = JsonObject[3].get<std::string>();
    std::string aoPath = JsonObject[4].get<std::string>();

    if (albedoPath.find("Assets") == std::string::npos) albedoPath = "Assets/" + albedoPath;
    if (normalPath.find("Assets") == std::string::npos) normalPath = "Assets/" + normalPath;
    if (roughPath.find("Assets") == std::string::npos) roughPath = "Assets/" + roughPath;
    if (metalPath.find("Assets") == std::string::npos) metalPath = "Assets/" + metalPath;
    if (aoPath.find("Assets") == std::string::npos) aoPath = "Assets/" + aoPath;

    Texture* Albedo = AssetRegistry::Get()->LoadTexture(albedoPath);
    Texture* Normal = AssetRegistry::Get()->LoadTexture(normalPath);
    Texture* Roughness = AssetRegistry::Get()->LoadTexture(roughPath);
    Texture* Metallic = AssetRegistry::Get()->LoadTexture(metalPath);
    Texture* AO = AssetRegistry::Get()->LoadTexture(aoPath);

    return Material(Albedo, Normal, Roughness, Metallic, AO);
}

