#pragma once

#include "Platform\RendererPlatform.hpp"

enum class Token
{
    Texture,
    Static_Mesh,
    Vertex,
    Indices,
    End,
    Error
};

enum class ParseState
{
    Static_Mesh
};

class FileLoader
{
public:
    static Mesh_ID LoadOBJFile(std::string filePath, Renderer& renderer);
    //static Scene LoadScene(std::string sceneFilePath, Renderer* renderer);

private:

    //static std::pair<unsigned int, Texture_ID> ProcessTexture(std::vector<std::string> line, Renderer* renderer);
    static Vertex ProcessVertex(std::vector<std::string> line);
    static std::vector<unsigned int> ProcessIndices(std::vector<std::string> line);
};

