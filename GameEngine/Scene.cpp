#include "Scene.h"

#include "Math.hpp"

#include "FileLoader.h"
#include "StringUtils.hpp"

#include <fstream>
#include <vector>
#include <string>

Scene::Scene()
    : m_Renderer(nullptr)
{
}

Scene::Scene(Renderer* renderer)
    : m_Renderer(renderer)
{
}

Scene::Scene(std::unordered_map<unsigned int, Texture> textures, std::vector<Mesh_ID> staticGeometry)
{
    m_Textures = textures;
    m_StaticGeometry = staticGeometry;
}

Scene::Scene(std::string sceneFilePath, Renderer* renderer) 
    : Scene(FileLoader::LoadScene(sceneFilePath, renderer))
{
}


void Scene::SaveScene(std::string sceneFilePath)
{
    std::ofstream sceneFile(sceneFilePath);

    sceneFile << "Yeetus\n";

    sceneFile.close();
}

void Scene::DrawScene()
{
    for (int i = 0; i < m_StaticGeometry.size(); ++i)
    {
        m_Renderer->DrawMesh(m_StaticGeometry[i]);
    }   
}
