#include "HotspotTexture.h"

#include "GameEngine.h"
#include <Math/Geometry.h>

#include <fstream>

void HotspotTexture::Save(std::string fileName)
{
    json HotspotTextureJson;
    HotspotTextureJson["TexturePath"] = m_Material.m_Albedo->Path.GetFullPath();

    json MaterialJson;
    m_Material.Save(MaterialJson);

    HotspotTextureJson["Material"] = MaterialJson;

    json HotspotsJson;
    for (Rect& hotspot : m_Hotspots)
    {
        json HotspotJson;
        HotspotJson["Location"] = { hotspot.location.x, hotspot.location.y };
        HotspotJson["Size"] = { hotspot.size.x, hotspot.size.y };
        HotspotsJson.push_back(HotspotJson);
    }
    HotspotTextureJson["Hotspots"] = HotspotsJson;

    HotspotTextureJson["AllowRotation"] = m_AllowRotation;

    std::ofstream File(fileName);
    if (!File.is_open())
    {
        Engine::FatalError("Could not open hotspot texture file for writing: " + fileName);
    }

    File << std::setw(4) << HotspotTextureJson << std::endl;
    File.close();
}

void HotspotTexture::Load(std::string fileName)
{
    m_Hotspots.clear();

    std::ifstream File(fileName);
    if (!File.is_open())
    {
        Engine::FatalError("Could not open hotspot texture file: " + fileName);
    }
    json HotspotJson;
    File >> HotspotJson;

    m_Material = Material::Load(HotspotJson["Material"]);

    json HotspotsJson = HotspotJson["Hotspots"];
    for (json& HotspotJson : HotspotsJson)
    {
        json LocationJson = HotspotJson["Location"];
        json SizeJson = HotspotJson["Size"];
        Rect hotspot;
        hotspot.location = Vec2f(LocationJson[0], LocationJson[1]);
        hotspot.size = Vec2f(SizeJson[0], SizeJson[1]);
        m_Hotspots.push_back(hotspot);
    }

    if (HotspotJson.contains("AllowRotation"))
    {
        m_AllowRotation = HotspotJson["AllowRotation"].get<bool>();
    }
    else
    {
        m_AllowRotation = true;
    }

    File.close();
}