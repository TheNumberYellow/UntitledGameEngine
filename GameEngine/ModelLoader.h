#pragma once

#include "RendererPlatform.hpp"

class ModelLoader
{
public:
    static Mesh_ID LoadOBJFile(std::string filePath, Renderer& renderer);
};

