#pragma once

#include "Asset.h"
#include "Graphics/HotspotTexture.h"
#include "Platform/RendererPlatform.h"
#include "Utils/FilePath.h"

#include <string>
#include <unordered_map>

class GraphicsModule;


class Texture : public Asset
{
public:
    
    Texture_ID GetID();

    void SetID(Texture_ID inId);

    friend bool operator<(const Texture& lhs, const Texture& rhs)
    {
        return lhs.Id < rhs.Id;
    }

    friend bool operator==(const Texture& lhs, const Texture& rhs)
    {
        return lhs.Id == rhs.Id;
    }
private:
    Texture_ID Id;
};

class StaticMesh : public Asset
{
public:
    StaticMesh_ID Id;

    friend bool operator<(const StaticMesh& lhs, const StaticMesh& rhs)
    {
        return lhs.Id < rhs.Id;
    }

    friend bool operator==(const StaticMesh& lhs, const StaticMesh& rhs)
    {
        return lhs.Id == rhs.Id;
    }
};


class AssetRegistry
{
public:
    AssetRegistry(GraphicsModule* InGraphicsModule);

    static AssetRegistry* Get();
    
    void LoadAsset(std::string Path);
    void LoadAsset(FilePath Path);

    Texture* LoadTexture(FilePath Path, bool LazyLoad = false);
    StaticMesh* LoadStaticMesh(FilePath Path);
    HotspotTexture* LoadHotspotTexture(FilePath Path);

    Texture_ID DefaultTexture;

private:

    AssetRegistry() = default;

    std::unordered_map<std::string, Texture*> m_LoadedTextures;
    std::unordered_map<std::string, StaticMesh> m_LoadedStaticMeshes;
    std::unordered_map<std::string, HotspotTexture> m_LoadedHotspotTextures;

    GraphicsModule* m_GraphicsModule;

    static AssetRegistry* s_Instance;
};

