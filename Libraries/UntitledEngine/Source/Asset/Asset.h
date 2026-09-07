#pragma once

#include "Utils/FilePath.h"

class Asset
{
public:
    FilePath Path;
    bool LoadedFromFile = false;
    bool Loaded = false;
};