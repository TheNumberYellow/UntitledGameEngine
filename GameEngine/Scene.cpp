#include "Scene.h"

#include "StringUtils.hpp"

#include <fstream>
#include <vector>
#include <string>

Token GetTokenFromString(std::string str)
{
    if (str == "texture") { return Token::Texture; }
    if (str == "static_mesh") { return Token::Static_Mesh; }
    if (str == "indices") { return Token::Indices; }
    if (str == "end") { return Token::End; }
    if (str == "vert") { return Token::Vertex; }

    return Token::Error;
}

Scene::Scene()
{
}

Scene::Scene(std::string sceneFilePath, Renderer* renderer)
{
    
    LoadScene(sceneFilePath, renderer);
}

void Scene::LoadScene(std::string sceneFilePath, Renderer* renderer)
{
    std::stack<ParseState> stateStack;
    std::ifstream sceneFile(sceneFilePath);
    std::string line;

    Mesh_ID currentMesh;
    std::vector<Vertex> currentMeshVertices;
    std::vector<unsigned int> currentMeshIndices;
    unsigned int currentMeshTextureId;

    if (sceneFile.is_open())
    {
        while (getline(sceneFile, line))
        {
            std::vector<std::string> words = StringUtils::Split(line, " ");

            if (words.size() == 0) continue;

            if (stateStack.empty())
            {
                // Determine what type of line based on the first word
                switch (GetTokenFromString(words[0]))
                {
                case Token::Texture:
                    ProcessTexture(words, renderer);
                    break;
                case Token::Static_Mesh:
                    // The beginning of a static mesh has the ID of the texture it's using as its second word
                    currentMeshTextureId = std::stoul(words[1]);
                    stateStack.push(ParseState::Static_Mesh);
                    break;
                case Token::Error:
                default:
                    continue;
                }
            }
            else {
                switch (stateStack.top())
                {
                case ParseState::Static_Mesh:
                    switch (GetTokenFromString(words[0]))
                    {
                    case Token::End:
                        // Save the mesh and exit the Mesh state
                        currentMesh = renderer->LoadMesh(currentMeshVertices, currentMeshIndices);
                        renderer->SetMeshTexture(currentMesh, m_Textures[currentMeshTextureId]);
                        m_StaticGeometry.push_back(currentMesh);

                        currentMeshVertices.clear();
                        currentMeshIndices.clear();
                        stateStack.pop();

                        break;
                    case Token::Vertex:
                        currentMeshVertices.push_back(ProcessVertex(words));
                        break;
                    case Token::Indices:
                        currentMeshIndices = ProcessIndices(words);
                        break;

                    default:
                        break;
                    }

                    break;
                }
            }
        }
        sceneFile.close();
    }
}

void Scene::SaveScene(std::string sceneFilePath)
{
    std::ofstream sceneFile(sceneFilePath);

    sceneFile << "Yeetus\n";

    sceneFile.close();
}

void Scene::AddEditableMesh(Mesh_ID newMesh, Renderer* renderer)
{
    EditableMesh editableMesh;
    editableMesh.m_Mesh = newMesh;
    editableMesh.m_CollisionTriangles = GenerateCollisionGeometryFromMesh(newMesh, renderer);
    editableMesh.m_LineMesh = GenerateLineMeshFromMesh(newMesh, renderer);
    editableMesh.m_Selected = false;

    m_EditableStaticGeometry.push_back(editableMesh);
}

void Scene::AddStaticMesh(Mesh_ID newMesh, Renderer* renderer)
{
    m_StaticGeometry.push_back(newMesh);
}

void Scene::SetEditableMeshSelected(EditableMesh* mesh)
{
    m_SelectedMesh = mesh;
    mesh->m_Selected = true;
}

void Scene::UnselectSelectedEditableMesh()
{
    if (m_SelectedMesh)
    {
        m_SelectedMesh->m_Selected = false;
        m_SelectedMesh = nullptr;
    }
}

void Scene::DrawScene(Renderer* renderer)
{
    for (int i = 0; i < m_StaticGeometry.size(); ++i)
    {
        renderer->DrawMesh(m_StaticGeometry[i]);
    }

    for (int i = 0; i < m_EditableStaticGeometry.size(); ++i)
    {
        renderer->DrawMesh(m_EditableStaticGeometry[i].m_Mesh);
        
        // If editable geometry is selected, draw the line mesh
        if (m_EditableStaticGeometry[i].m_Selected)
        {
            renderer->DrawMesh(m_EditableStaticGeometry[i].m_LineMesh);
        }

    }
}

RayCastSceneHit Scene::RayCast(Ray ray)
{
    RayCastSceneHit finalSceneHit;

    for (int i = 0; i < m_EditableStaticGeometry.size(); ++i)
    {
        for (int j = 0; j < m_EditableStaticGeometry[i].m_CollisionTriangles.size(); ++j)
        {
            RayCastHit hit = ::RayCast(ray, m_EditableStaticGeometry[i].m_CollisionTriangles[j]);
            if (hit.hit && hit.hitDistance < finalSceneHit.m_Hit.hitDistance)
            {
                finalSceneHit.m_Hit = hit;
                finalSceneHit.m_HitMesh = &m_EditableStaticGeometry[i];              
            }
        }
    }

    
    return finalSceneHit;
}

std::vector<Triangle> Scene::GenerateCollisionGeometryFromMesh(Mesh_ID mesh, Renderer* renderer)
{
    std::vector<unsigned int*> meshElements = renderer->MapMeshElements(mesh);
    std::vector<Vertex*> meshVertices = renderer->MapMeshVertices(mesh);

    std::vector<Triangle> resultTriangles;

    for (int i = 0; i < meshElements.size(); i += 3)
    {
        Triangle newTriangle;
        newTriangle.a = meshVertices[*meshElements[i]]->position;
        newTriangle.b = meshVertices[*meshElements[(size_t)i + 1]]->position;
        newTriangle.c = meshVertices[*meshElements[(size_t)i + 2]]->position;

        resultTriangles.push_back(newTriangle);
    }
    renderer->UnmapMeshElements(mesh);
    renderer->UnmapMeshVertices(mesh);

    return resultTriangles;
}

Mesh_ID Scene::GenerateLineMeshFromMesh(Mesh_ID mesh, Renderer* renderer)
{
    Mesh_ID newMesh;

    std::vector<unsigned int*> meshElements = renderer->MapMeshElements(mesh);
    std::vector<Vertex*> meshVertices = renderer->MapMeshVertices(mesh);


    std::vector<unsigned int> lineElements;
    std::vector<Vertex> lineVertices;

    for (int i = 0; i < meshElements.size(); i += 3)
    {
        lineElements.push_back(*meshElements[i]);
        lineElements.push_back(*meshElements[(size_t)i + 1]);
        lineElements.push_back(*meshElements[(size_t)i + 1]);
        lineElements.push_back(*meshElements[(size_t)i + 2]);
        lineElements.push_back(*meshElements[(size_t)i + 2]);
        lineElements.push_back(*meshElements[i]);
    }
    for (int i = 0; i < meshVertices.size(); ++i)
    {
        Vertex vert = Vertex(*meshVertices[i]);
        vert.position += vert.normal * 0.005f;
        vert.colour = Vec4f(0.0f, 1.0f, 1.0f, 1.0f);
        lineVertices.push_back(vert);
    }

    newMesh = renderer->LoadMesh(lineVertices, lineElements);

    renderer->SetMeshDrawType(newMesh, DrawType::Line);

    renderer->UnmapMeshElements(mesh);
    renderer->UnmapMeshVertices(mesh);

    return newMesh;
}

void Scene::ProcessTexture(std::vector<std::string> line, Renderer* renderer)
{
    // TODO: Add more texture parameters when we need them

    // First word in line should be "texture" token, second should be texture file path
    Texture newTexture = renderer->LoadTexture(line[1]);

    // Third will be texture ID for reference from meshes
    unsigned int textureID = std::stoul(line[2]);

    m_Textures[textureID] = newTexture;
}

Vertex Scene::ProcessVertex(std::vector<std::string> line)
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

std::vector<unsigned int> Scene::ProcessIndices(std::vector<std::string> line)
{
    std::vector<unsigned int> indices;

    // We skip index 0, that will be the "indices" token
    for (int i = 1; i < line.size(); i++)
    {
        indices.push_back(std::stoul(line[i]));
    }

    return indices;
}
