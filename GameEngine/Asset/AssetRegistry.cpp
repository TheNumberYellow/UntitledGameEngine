#include "AssetRegistry.h"

AssetRegistry* AssetRegistry::Instance = nullptr;

AssetRegistry* AssetRegistry::Get()
{
    if (!Instance)
    {
        Instance = new AssetRegistry();
    }
    
    return Instance;
}

void AssetRegistry::LoadAsset(std::string Path)
{
    LoadAsset(FilePath(Path));
}

void AssetRegistry::LoadAsset(FilePath Path)
{
    std::string Ext = Path.GetExt();


}
