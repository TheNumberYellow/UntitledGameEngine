#pragma once

#include <string>
#include "Utils/FilePath.h"
#include "Platform/RendererPlatform.h"

class GraphicsModule;

class Asset
{
public:
    FilePath Path;
    bool LoadedFromFile = false;
};

class Texture : public Asset
{
public:
    Texture_ID Id;

    friend bool operator<(const Texture& lhs, const Texture& rhs)
    {
        return lhs.Id < rhs.Id;
    }

    friend bool operator==(const Texture& lhs, const Texture& rhs)
    {
        return lhs.Id == rhs.Id;
    }
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

    Texture* LoadTexture(FilePath Path);
    StaticMesh* LoadStaticMesh(FilePath Path);

private:

    AssetRegistry() = default;

    std::unordered_map<std::string, Texture> m_LoadedTextures;
    std::unordered_map<std::string, StaticMesh> m_LoadedStaticMeshes;

    GraphicsModule* m_GraphicsModule;

    static AssetRegistry* s_Instance;
};

