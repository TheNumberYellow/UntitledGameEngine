#include <string>

#include <json.hpp>

#include "Material.h"

using json = nlohmann::json;

struct Rect;

class HotspotTexture
{
public:
    Material m_Material;
    std::vector<Rect> m_Hotspots;
    bool m_AllowRotation = true;

    void Save(std::string fileName);
    void Load(std::string fileName);
};