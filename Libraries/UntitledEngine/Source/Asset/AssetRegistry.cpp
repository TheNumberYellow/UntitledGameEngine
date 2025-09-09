#include "AssetRegistry.h"

#include "Modules/GraphicsModule.h"

Texture_ID Texture::GetID()
{
    if (Loaded)
    {
        return Id;
    }
    else
    {
        return AssetRegistry::Get()->DefaultTexture;
    }
}

void Texture::SetID(Texture_ID inId)
{
    Id = inId;
}

AssetRegistry::AssetRegistry(GraphicsModule* InGraphicsModule)
    : m_GraphicsModule(InGraphicsModule)
{
    DefaultTexture = m_GraphicsModule->LoadTexture("Assets/textures/DefaultTexture.png", TextureMode::NEAREST, TextureMode::NEAREST)->GetID();

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

Texture* AssetRegistry::LoadTexture(FilePath Path, bool LazyLoad)
{
    auto it = m_LoadedTextures.find(Path.GetFullPath());
    if (it != m_LoadedTextures.end())
    {
        return it->second;
    }
    if (LazyLoad)
    {
        m_LoadedTextures[Path.GetFullPath()] = m_GraphicsModule->LoadTextureAsync(Path.GetFullPath());
    }
    else
    {
        m_LoadedTextures[Path.GetFullPath()] = m_GraphicsModule->LoadTexture(Path.GetFullPath());
    }
    
    return m_LoadedTextures[Path.GetFullPath()];
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
