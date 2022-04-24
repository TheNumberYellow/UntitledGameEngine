#include "FileLoader.h"

#include "StringUtils.h"

#include <fstream>
#include <stack>


Mesh_ID FileLoader::LoadOBJFile(std::string filePath, Renderer& renderer)
{
    std::ifstream objFile(filePath);
    std::string line;

    VertexBufferFormat vertFormat({ VertAttribute::Vec3f, VertAttribute::Vec3f, VertAttribute::Vec4f, VertAttribute::Vec2f });
    std::vector<float> vertices;
    std::vector<ElementIndex> indices;

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

                    vertices.insert(vertices.end(), { 
                        newVert.position.x, newVert.position.y, newVert.position.z,
                        newVert.normal.x, newVert.normal.y, newVert.normal.z,
                        newVert.colour.x, newVert.colour.y, newVert.colour.z, newVert.colour.w,
                        newVert.uv.x, newVert.uv.y
                    });

                    // TODO(fraser): here there is no indexed renderering (ie. no exploitation of duplicated vertex data in a mesh)
                    // Come back to this if there are memory concerns
                    indices.push_back(index++);

                }
            }
        }

    }
    return renderer.LoadMesh(vertFormat, vertices, indices);
}


Token GetTokenFromString(std::string str)
{
    if (str == "texture") { return Token::Texture; }
    if (str == "static_mesh") { return Token::Static_Mesh; }
    if (str == "indices") { return Token::Indices; }
    if (str == "end") { return Token::End; }
    if (str == "vert") { return Token::Vertex; }

    return Token::Error;
}

//
//Scene FileLoader::LoadScene(std::string sceneFilePath, Renderer* renderer)
//{
//    std::stack<ParseState> stateStack;
//    std::ifstream sceneFile(sceneFilePath);
//    std::string line;
//
//    std::unordered_map<unsigned int, Texture_ID> textures;
//    std::vector<Mesh_ID> staticGeometry;
//
//    Mesh_ID currentMesh;
//    std::vector<Vertex> currentMeshVertices;
//    std::vector<unsigned int> currentMeshIndices;
//    unsigned int currentMeshTextureId;
//
//    if (sceneFile.is_open())
//    {
//        while (getline(sceneFile, line))
//        {
//            std::vector<std::string> words = StringUtils::Split(line, " ");
//
//            if (words.size() == 0) continue;
//
//            if (stateStack.empty())
//            {
//                // Determine what type of line based on the first word
//                switch (GetTokenFromString(words[0]))
//                {
//                case Token::Texture:
//                    textures.insert(ProcessTexture(words, renderer));
//                    break;
//                case Token::Static_Mesh:
//                    // The beginning of a static mesh has the ID of the texture it's using as its second word
//                    currentMeshTextureId = std::stoul(words[1]);
//                    stateStack.push(ParseState::Static_Mesh);
//                    break;
//                case Token::Error:
//                default:
//                    continue;
//                }
//            }
//            else {
//                switch (stateStack.top())
//                {
//                case ParseState::Static_Mesh:
//                    switch (GetTokenFromString(words[0]))
//                    {
//                    case Token::End:
//                        // Save the mesh and exit the Mesh state
//                        currentMesh = renderer->LoadMesh(currentMeshVertices, currentMeshIndices);
//                        //renderer->SetMeshTexture(currentMesh, textures[currentMeshTextureId]);
//                        staticGeometry.push_back(currentMesh);
//
//                        currentMeshVertices.clear();
//                        currentMeshIndices.clear();
//                        stateStack.pop();
//
//                        break;
//                    case Token::Vertex:
//                        currentMeshVertices.push_back(ProcessVertex(words));
//                        break;
//                    case Token::Indices:
//                        currentMeshIndices = ProcessIndices(words);
//                        break;
//
//                    default:
//                        break;
//                    }
//
//                    break;
//                }
//            }
//        }
//        sceneFile.close();
//    }
//    return Scene(textures, staticGeometry);
//}

//std::pair<unsigned int, Texture_ID> FileLoader::ProcessTexture(std::vector<std::string> line, Renderer* renderer)
//{
//    // TODO: Add more texture parameters when we need them
//
//    // First word in line should be "texture" token, second should be texture file path
//    Texture newTexture = renderer->LoadTexture(line[1]);
//
//    // Third will be texture ID for reference from meshes
//    unsigned int textureID = std::stoul(line[2]);
//
//    return std::pair<unsigned int, Texture>(textureID, newTexture);
//}

Vertex FileLoader::ProcessVertex(std::vector<std::string> line)
{
    // TODO (Fraser) take another look at this (generalize if we do more similar to this later)
    Vertex newVertex;
    std::vector<float> floats;
    for (int i = 1; i < line.size(); ++i)
    {
        floats.push_back(std::stof(line[i]));
    }
    newVertex.position = Vec3f(floats[0], floats[1], floats[2]);
    newVertex.normal = Vec3f(floats[3], floats[4], floats[5]);
    newVertex.colour = Vec4f(floats[6], floats[7], floats[8], floats[9]);
    newVertex.uv = Vec2f(floats[10], floats[11]);

    return newVertex;

}

std::vector<unsigned int> FileLoader::ProcessIndices(std::vector<std::string> line)
{
    std::vector<unsigned int> indices;

    // We skip index 0, that will be the "indices" token
    for (int i = 1; i < line.size(); i++)
    {
        indices.push_back(std::stoul(line[i]));
    }

    return indices;
}