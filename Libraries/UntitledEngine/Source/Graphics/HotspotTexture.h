#pragma once
#include <string>

#include <json.hpp>

#include "Material.h"
#include "Utils/FilePath.h"
#include "Asset/Asset.h"

using json = nlohmann::json;

struct Rect;

class HotspotTexture : public Asset
{
public:
    Material m_Material;
    std::vector<Rect> m_Hotspots;
    bool m_AllowRotation = true;

    FilePath m_Path;

    void Save(std::string fileName);
    void Load(std::string fileName);
};