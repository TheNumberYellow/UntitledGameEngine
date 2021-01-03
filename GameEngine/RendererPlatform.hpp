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

struct OpenGLShader
{
	unsigned int programID;

	// Vertex locations
	unsigned int PositionLocation;
	unsigned int NormalLocation;
	unsigned int ColourLocation;
	unsigned int UVLocation;

	// Uniform locations
	unsigned int TimeLocation;
	unsigned int TransformationLocation;
	unsigned int SunDirectionLocation;

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

private:

	OpenGLShader defaultShader;

	CamInfo defaultCamera;

	Vec2i screenSize;

	bool CheckShaderCompilation(unsigned int shader);

	// This function operates on the currently bound shader
	void UpdateCamTransform();
};