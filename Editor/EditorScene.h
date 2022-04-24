#pragma once
#include "Platform\RendererPlatform.h"
#include "Collisions.h"

#include <vector>

constexpr float TOOL_ARROW_DIST_FROM_OBJECT_ORIGIN = 1.0f;

class EditorScene;

//struct EditableMesh
//{
//	Mesh_ID m_Mesh;
//	Mesh_ID m_LineMesh;
//	bool m_Selected;
//	CollisionMesh m_CollisionTriangles;
//	EditorScene* m_EditorScene;
//
//	void SetPosition(Vec3f position, Renderer* renderer);
//	void SetScale(Vec3f scale, Renderer* renderer);
//
//	void RotateMeshAroundAxis(Vec3f axis, float rotationAmount, Renderer* renderer);
//};
//
//enum class HitType
//{
//	NONE,
//	MESH,
//	TOOL
//};
//
//enum class ToolType
//{
//	NONE,
//	X_ARROW,
//	Y_ARROW,
//	Z_ARROW,
//	X_RING,
//	Y_RING,
//	Z_RING
//};
//
//struct EditorRayCastInfo
//{
//	EditableMesh* hitMesh;
//
//	RayCastHit hitInfo;
//
//	HitType hitType = HitType::NONE;
//	ToolType toolType = ToolType::NONE;
//};
//
//struct ToolMesh
//{
//	Mesh_ID m_Mesh;
//	CollisionMesh m_CollisionTriangles;
//};
//
//class EditorScene : public Scene
//{
//public:
//	EditorScene() 
//		: Scene()
//	{}
//	EditorScene(Renderer* renderer);
//	
//	void DrawScene() override;
//	
//	void AddEditableMesh(Mesh_ID newMesh);
//
//	void SetEditableMeshSelected(EditableMesh* mesh);
//	void UnselectSelectedEditableMesh();
//
//	EditorRayCastInfo RayCast(Ray ray);
//
//	void CycleToolType();
//
//private:
//
//	// Geometry meant to be moved around in the editor, and then saved as static geometry
//	std::vector<EditableMesh> m_EditableStaticGeometry;
//
//	EditableMesh* m_SelectedMesh = nullptr;
//
//	ToolMesh m_XAxisArrow;
//	ToolMesh m_YAxisArrow;
//	ToolMesh m_ZAxisArrow;
//	
//	ToolMesh m_XRotatorRing;
//	ToolMesh m_YRotatorRing;
//	ToolMesh m_ZRotatorRing;
//
//	enum class ToolsType
//	{
//		TRANSLATION = 0,
//		ROTATION = 1
//	};
//
//	ToolsType toolsType = ToolsType::TRANSLATION;
//
//	void UpdateToolMeshPositions(Vec3f position);
//};
