#include "ModelLoader.h"

#include "StringUtils.hpp"

#include <fstream>

Mesh_ID ModelLoader::LoadOBJFile(std::string filePath, Renderer& renderer)
{
    std::ifstream objFile(filePath);
    std::string line;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int index = 0;

    if (objFile.is_open())
    {
        std::vector<Vec3f> vecs;
        std::vector<Vec2f> uvs;
        std::vector<Vec3f> norms;

        while (getline(objFile, line))
        {
            std::vector<std::string> words = StringUtils::Split(line, " ");

            // Empty line, go to the next one
            if (words.size() == 0) continue;

            // Lines starting with # are comments
            if (words[0] == "#") continue;

            if (words[0] == "v")
            {
                Vec3f newVec;
                newVec.x = std::stof(words[1]);
                newVec.y = std::stof(words[2]);
                newVec.z = std::stof(words[3]);
                vecs.push_back(newVec);
            }

            if (words[0] == "vt")
            {
                Vec2f newUv;
                newUv.x = std::stof(words[1]);
                newUv.y = std::stof(words[2]);
                uvs.push_back(newUv);
            }

            if (words[0] == "vn")
            {
                Vec3f newNorm;
                newNorm.x = std::stof(words[1]);
                newNorm.y = std::stof(words[2]);
                newNorm.z = std::stof(words[3]);
                norms.push_back(newNorm);
            }

            if (words[0] == "f")
            {
                for (int i = 3; i > 0; --i)
                {
                    std::vector<std::string> indexes;
                    indexes = StringUtils::Split(words[i], "/");

                    Vertex newVert;
                    newVert.position = vecs[std::stoul(indexes[0]) - 1];
                    newVert.uv = uvs[std::stoul(indexes[1]) - 1];
                    newVert.normal = norms[std::stoul(indexes[2]) - 1];
                    
                    newVert.colour = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);

                    vertices.push_back(newVert);

                    // TODO(fraser): here there is no indexed renderering (ie. no exploitation of duplicated vertex data in a mesh)
                    // Come back to this if there are memory concerns
                    indices.push_back(index++);

                }
            }
        }

    }
    return renderer.LoadMesh(vertices, indices);
}
