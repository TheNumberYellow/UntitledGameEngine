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

struct DebugLineSegment
{
	Vec3f a, b;
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

	//TODO(fraser) Should the renderer be in charge of loading things?
	Texture LoadTexture(std::string filePath);
	Mesh_ID LoadMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

	void DrawMesh(Mesh_ID mesh);

	void ClearScreen();
	void SwapBuffer();

	//TODO(fraser) These three should definitely be changed - SetTime and SetSunDirection should be done in a more general "set uniform with name X" way
	// Also SetCamTransform should take a matrix
	void SetTime(float time);
	void SetCamTransform(Vec3f position, Vec3f direction, Vec3f up);
	void SetSunDirection(Vec3f newDirection);

	void SetVertexPosition(Mesh_ID mesh, unsigned int vertexIndex, Vec3f newPosition);

	std::vector<Vertex*> MapMeshVertices(Mesh_ID mesh);
	void UnmapMeshVertices(Mesh_ID mesh);

	std::vector<unsigned int*> MapMeshElements(Mesh_ID mesh);
	void UnmapMeshElements(Mesh_ID mesh);

	void* MapMeshRange(Mesh_ID mesh, unsigned int offset, unsigned int range);

	//TODO(fraser) should a texture be an attribute of a mesh or a "model"?
	void SetMeshTexture(Mesh_ID meshID, Texture texture);
	
	void SetMeshDrawType(Mesh_ID meshID, DrawType type);

	//TODO(fraser) honestly maybe all of this (scale, position, rotation) information should be managed by a "model" object
	void MoveMesh(Mesh_ID meshID, Vec3f move);
	void ScaleMesh(Mesh_ID meshID, Vec3f scaleFactor);

	Vec3f GetMeshPosition(Mesh_ID meshID);
	void SetMeshPosition(Mesh_ID meshID, Vec3f newPos);

	Vec3f GetMeshScale(Mesh_ID meshID);
	void SetMeshScale(Mesh_ID meshID, Vec3f newScale);

	void RotateMeshAroundAxis(Mesh_ID meshID, Vec3f axis, float rotationAmount);

	void SetMeshRotation(Mesh_ID meshID, Quaternion rotation);
	Quaternion GetMeshRotation(Mesh_ID meshID);

	Mat4x4f GetMeshTransform(Mesh_ID meshID);
	
	void InitializeDebugDraw();
	void DebugDrawLineSegment(DebugLineSegment lineSegment);

	void EnableDepthTesting();
	void DisableDepthTesting();

	void ClearDepthBuffer();

	void SetMeshColour(Mesh_ID meshID, Vec4f colour);

	//TODO(fraser) this should be moved to other "debug draw" functions? (and probably done in a different way anyway)
	Mesh_ID GenerateLineMeshFromMesh(Mesh_ID mesh, Vec4f colour);

private:

	CamInfo defaultCamera;

	Vec2i screenSize;

	bool CheckShaderCompilation(unsigned int shader);

	// This function operates on the currently bound shader
	void UpdateCamTransform();

	void DrawDebugLines();
};