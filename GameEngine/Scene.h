#pragma once
#include "RendererPlatform.hpp"

#include "Collisions.hpp"

#include <unordered_map>
#include <stack>
#include <utility>

class Scene
{
public:

	Scene();
	Scene(Renderer* renderer);
	Scene(std::unordered_map<unsigned int, Texture> textures, std::vector<Mesh_ID> staticGeometry);
	Scene(std::string sceneFilePath, Renderer* renderer);

	void SaveScene(std::string sceneFilePath);

	virtual void DrawScene();

protected:

	Renderer* m_Renderer;

	std::unordered_map<unsigned int, Texture> m_Textures;
	std::vector<Mesh_ID> m_StaticGeometry;
};

