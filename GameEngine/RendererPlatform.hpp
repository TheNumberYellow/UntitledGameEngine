#pragma once

#include <string>
#include <vector>


#include "EnginePlatform.hpp"
#include "Math.hpp"

struct Texture
{
	Texture() {}
	Texture(unsigned int textureId) { m_TextureId = textureId; }
	unsigned int m_TextureId = -1;
};

struct Vertex
{
	Vertex() {}
	Vertex(Vec3f position, Vec3f normal, Vec4f colour, Vec2f uv) : position(position), normal(normal), colour(colour), uv(uv) {}
	Vec3f position;
	Vec3f normal;
	Vec4f colour;
	Vec2f uv;
};

#define Mesh_ID unsigned int

enum class DrawType
{
	Triangle, Line
};

struct CamInfo
{
	Vec3f position;
	Vec3f direction;
	Vec3f up;

	bool camMatrixNeedsUpdate = true;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	Texture LoadTexture(std::string filePath);
	Mesh_ID LoadMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

	void DrawMesh(Mesh_ID mesh);

	void ClearScreen();
	void SwapBuffer();

	void SetTime(float time);
	void SetCamTransform(Vec3f position, Vec3f direction, Vec3f up);
	void SetSunDirection(Vec3f newDirection);

	void SetVertexPosition(Mesh_ID mesh, unsigned int vertexIndex, Vec3f newPosition);

	std::vector<Vertex*> MapMeshVertices(Mesh_ID mesh);
	void UnmapMeshVertices(Mesh_ID mesh);

	std::vector<unsigned int*> MapMeshElements(Mesh_ID mesh);
	void UnmapMeshElements(Mesh_ID mesh);

	void* MapMeshRange(Mesh_ID mesh, unsigned int offset, unsigned int range);

	void SetMeshTexture(Mesh_ID meshID, Texture texture);
	void SetMeshDrawType(Mesh_ID meshID, DrawType type);

	void MoveMesh(Mesh_ID meshID, Vec3f move);
	void ScaleMesh(Mesh_ID meshID, Vec3f scaleFactor);

	Vec3f GetMeshPosition(Mesh_ID meshID);
	void SetMeshPosition(Mesh_ID meshID, Vec3f newPos);

	Vec3f GetMeshScale(Mesh_ID meshID);
	void SetMeshScale(Mesh_ID meshID, Vec3f newScale);

	void RotateMeshAroundXAxis(Mesh_ID meshID, float rotationAmount);
	void RotateMeshAroundYAxis(Mesh_ID meshID, float rotationAmount);
	void RotateMeshAroundZAxis(Mesh_ID meshID, float rotationAmount);

	void SetMeshRotationAroundXAxis(Mesh_ID meshID, float rotation);
	void SetMeshRotationAroundYAxis(Mesh_ID meshID, float rotation);
	void SetMeshRotationAroundZAxis(Mesh_ID meshID, float rotation);

	float GetMeshRotationAroundXAxis(Mesh_ID meshID);
	float GetMeshRotationAroundYAxis(Mesh_ID meshID);
	float GetMeshRotationAroundZAxis(Mesh_ID meshID);


	Mat4x4f GetMeshTransform(Mesh_ID meshID);

	void EnableDepthTesting();
	void DisableDepthTesting();

	void ClearDepthBuffer();

	void SetMeshColour(Mesh_ID meshID, Vec4f colour);


	Mesh_ID GenerateLineMeshFromMesh(Mesh_ID mesh, Vec4f colour);

private:

	CamInfo defaultCamera;

	Vec2i screenSize;

	bool CheckShaderCompilation(unsigned int shader);

	// This function operates on the currently bound shader
	void UpdateCamTransform();
};