#pragma once
#include "RendererPlatform.hpp"

#include "Collisions.hpp"

#include <unordered_map>
#include <stack>
#include <utility>

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

struct EditableMesh
{
	Mesh_ID m_Mesh;
	Mesh_ID m_LineMesh;
	bool m_Selected;
	std::vector<Triangle> m_CollisionTriangles;
};

struct RayCastSceneHit
{
	RayCastHit m_Hit;
	EditableMesh* m_HitMesh;
};

class Scene
{
public:

	Scene();
	Scene(std::string sceneFilePath, Renderer* renderer);

	void LoadScene(std::string sceneFilePath, Renderer* renderer);
	void SaveScene(std::string sceneFilePath);

	void AddEditableMesh(Mesh_ID newMesh, Renderer* renderer);
	void AddStaticMesh(Mesh_ID newMesh, Renderer* renderer);

	void SetEditableMeshSelected(EditableMesh* mesh);
	void UnselectSelectedEditableMesh();

	void DrawScene(Renderer* renderer);

	RayCastSceneHit RayCast(Ray ray);

private:
	static std::vector<Triangle> GenerateCollisionGeometryFromMesh(Mesh_ID mesh, Renderer* renderer);
	static Mesh_ID GenerateLineMeshFromMesh(Mesh_ID mesh, Renderer* renderer);

	std::unordered_map<unsigned int, Texture> m_Textures;
	std::vector<Mesh_ID> m_StaticGeometry;

	// Geometry meant to be moved around in the editor, and then saved as static geometry
	std::vector<EditableMesh> m_EditableStaticGeometry;
	EditableMesh* m_SelectedMesh = nullptr;

	// TODO (fraser) these three similarly named functions have 3 different return values
	void ProcessTexture(std::vector<std::string> line, Renderer* renderer);
	Vertex ProcessVertex(std::vector<std::string> line);
	std::vector<unsigned int> ProcessIndices(std::vector<std::string> line);
};

