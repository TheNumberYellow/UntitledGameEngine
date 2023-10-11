#include "AssetRegistry.h"

#include "Modules/GraphicsModule.h"

AssetRegistry::AssetRegistry(GraphicsModule* InGraphicsModule)
    : m_GraphicsModule(InGraphicsModule)
{

}

AssetRegistry* AssetRegistry::s_Instance = nullptr;

AssetRegistry* AssetRegistry::Get()
{
    if (!s_Instance)
    {
        s_Instance = new AssetRegistry(GraphicsModule::Get());
    }
    
    return s_Instance;
}

void AssetRegistry::LoadAsset(std::string Path)
{
    LoadAsset(FilePath(Path));
}

void AssetRegistry::LoadAsset(FilePath Path)
{
    std::string Ext = Path.GetExt();


}

Texture* AssetRegistry::LoadTexture(FilePath Path)
{
    auto it = m_LoadedTextures.find(Path.GetFullPath());
    if (it != m_LoadedTextures.end())
    {
        return &it->second;
    }
    m_LoadedTextures[Path.GetFullPath()] = m_GraphicsModule->LoadTexture(Path.GetFullPath());
    return &m_LoadedTextures[Path.GetFullPath()];
}

StaticMesh* AssetRegistry::LoadStaticMesh(FilePath Path)
{
    auto it = m_LoadedStaticMeshes.find(Path.GetFullPath());
    if (it != m_LoadedStaticMeshes.end())
    {
        return &it->second;
    }
    m_LoadedStaticMeshes[Path.GetFullPath()] = m_GraphicsModule->LoadMesh(Path.GetFullPath());
    return &m_LoadedStaticMeshes[Path.GetFullPath()];
}
