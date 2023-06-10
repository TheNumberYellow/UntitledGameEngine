#pragma once

#include <string>
#include "Utils/FilePath.h"

class Asset
{
public:
    FilePath Path;
};


class AssetRegistry
{
public:

    AssetRegistry* Get();
    
    void LoadAsset(std::string Path);
    void LoadAsset(FilePath Path);

private:

    AssetRegistry() = default;

    static AssetRegistry* Instance;
};

