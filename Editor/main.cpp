#include "GameEngine.hpp"

#include "EditorScene.h"

#include <cmath>

struct Camera
{
    Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f direction = Vec3f(0.0f, 1.0f, 0.0f);
    Vec3f up = Vec3f(0.0f, 0.0f, 1.0f);
};

static Camera cam;

Mesh_ID unitCube;
Mesh_ID groundPlane;
Mesh_ID arrowMesh;
Mesh_ID boxMesh;
Mesh_ID padMesh;
Mesh_ID hoopMesh;
Mesh_ID groundMesh;
Mesh_ID treeStumpMesh;
Mesh_ID treeStumpMesh1;
Mesh_ID treeStumpMesh2;
Mesh_ID treeStumpMesh3;
Mesh_ID treeStumpMesh4;
Mesh_ID treeStumpMesh5;
Mesh_ID treeMesh;

Mesh_ID xArrow;
Mesh_ID yArrow;
Mesh_ID zArrow;

EditorScene editorScene;

static bool holdingMouseLeft = false;
static bool cameraControlEnabled = true;
static bool holdingTab = false;
static bool holdingAlt = false;
static bool holdingSpace = false;

static bool translatingX = false;
static bool translatingY = false;
static bool translatingZ = false;

static bool rotatingX = false;
static bool rotatingY = false;
static bool rotatingZ = false;

static Vec3f xAxisOffset;
static Vec3f yAxisOffset;
static Vec3f zAxisOffset;

static float xAngleLast;
static float yAngleLast;
static float zAngleLast;

bool editorMode = true;


EditableMesh* editingMesh = nullptr;

void MoveCamera(ControlInputs& inputs, Camera& cam, float pixelToRadians)
{
	cam.direction = Math::rotate(cam.direction, -(float)(inputs.DeltaMouse.x) * pixelToRadians, Vec3f(0.0f, 0.0f, 1.0f));

	Vec3f perpVector = Math::cross(cam.direction, cam.up);

	cam.direction = Math::rotate(cam.direction, -(float)(inputs.DeltaMouse.y) * pixelToRadians, perpVector);

	if (inputs.keysDown.w)
	{
		cam.position += cam.direction * 0.02f;
	}
	if (inputs.keysDown.s)
	{
		cam.position -= cam.direction * 0.02f;
	}
	if (inputs.keysDown.a)
	{
		cam.position -= perpVector * 0.02f;
	}
	if (inputs.keysDown.d)
	{
		cam.position += perpVector * 0.02f;
	}
}

Ray GetPointerRay()
{
	Mat4x4f camMatrix;

}

void Initialize(Renderer& renderer)
{
	Engine::LockCursor();

	editorScene = EditorScene(&renderer);

	unitCube = FileLoader::LoadOBJFile("models/Monkey.obj", renderer);

    Texture cubeTexture = renderer.LoadTexture("textures/cubeTexture.png");

	renderer.SetMeshTexture(unitCube, cubeTexture);
	
	editorScene.AddEditableMesh(unitCube);
	
	std::vector<Vertex> planeVertices =
	{
		Vertex(Vec3f(-100.0f, -100.0f, -2.0f), Vec3f(0.0f, 0.0f, 1.0f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(-100.0f, -100.0f)),
		Vertex(Vec3f(-100.0f, 100.0f, -2.0f), Vec3f(0.0f, 0.0f, 1.0f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(-100.0f, 100.0f)),
		Vertex(Vec3f(100.0f, 100.0f, -2.0f), Vec3f(0.0f, 0.0f, 1.0f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(100.0f, 100.0f)),
		Vertex(Vec3f(100.0f, -100.0f, -2.0f), Vec3f(0.0f, 0.0f, 1.0f), Vec4f(1.0f, 1.0f, 1.0f, 1.0f), Vec2f(100.0f, -100.0f)),
	};
	std::vector<unsigned int> planeIndices =
	{
		0, 1, 2, 0, 2, 3
	};

	groundPlane = renderer.LoadMesh(planeVertices, planeIndices);

	groundMesh = FileLoader::LoadOBJFile("models/TestMountain.obj", renderer);
	Texture planeTexture = renderer.LoadTexture("textures/dirt.png");
	renderer.SetMeshTexture(groundMesh, planeTexture);
	editorScene.AddEditableMesh(groundMesh);

	treeStumpMesh = FileLoader::LoadOBJFile("models/TreeStumpTri.obj", renderer);
	renderer.SetMeshColour(treeStumpMesh, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	Texture treeStumpTexture = renderer.LoadTexture("textures/TreeStump.png");
	renderer.SetMeshTexture(treeStumpMesh, treeStumpTexture);
	editorScene.AddEditableMesh(treeStumpMesh);

	treeStumpMesh1 = FileLoader::LoadOBJFile("models/TreeStumpTri.obj", renderer);
	renderer.SetMeshColour(treeStumpMesh1, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	renderer.SetMeshTexture(treeStumpMesh1, treeStumpTexture);
	editorScene.AddEditableMesh(treeStumpMesh1);

	treeStumpMesh2 = FileLoader::LoadOBJFile("models/TreeStumpTri.obj", renderer);
	renderer.SetMeshColour(treeStumpMesh2, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	renderer.SetMeshTexture(treeStumpMesh2, treeStumpTexture);
	editorScene.AddEditableMesh(treeStumpMesh2);

	treeStumpMesh3 = FileLoader::LoadOBJFile("models/TreeStumpTri.obj", renderer);
	renderer.SetMeshColour(treeStumpMesh3, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	renderer.SetMeshTexture(treeStumpMesh3, treeStumpTexture);
	editorScene.AddEditableMesh(treeStumpMesh3);

	treeStumpMesh4 = FileLoader::LoadOBJFile("models/TreeStumpTri.obj", renderer);
	renderer.SetMeshColour(treeStumpMesh4, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	renderer.SetMeshTexture(treeStumpMesh4, treeStumpTexture);
	editorScene.AddEditableMesh(treeStumpMesh4);

	treeStumpMesh5 = FileLoader::LoadOBJFile("models/TreeStumpTri.obj", renderer);
	renderer.SetMeshColour(treeStumpMesh5, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	renderer.SetMeshTexture(treeStumpMesh5, treeStumpTexture);
	editorScene.AddEditableMesh(treeStumpMesh5);

	treeMesh = FileLoader::LoadOBJFile("models/TreeTri.obj", renderer);
	renderer.SetMeshColour(treeMesh, Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	editorScene.AddEditableMesh(treeMesh);

	renderer.SetSunDirection(Vec3f(0.0f, 0.0f, -1.0f));

	Engine::HideCursor();
}


void Update(Renderer& renderer, ControlInputs& inputs)
{


    if (inputs.keysDown.esc)
    {
        Engine::StopGame();
        return;
    }
	if (cameraControlEnabled)
	{
		MoveCamera(inputs, cam, 0.001f);
	}

	if (inputs.mouse.leftMouseButton)
	{
		if (holdingMouseLeft)
		{
			if (translatingX)
			{
				Line mouseLine;
				mouseLine.point = cam.position - xAxisOffset;
				mouseLine.direction = cam.direction;

				Line axisLine;
				axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
				axisLine.direction = Vec3f(1.0f, 0.0f, 0.0f);

				Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

				editingMesh->SetPosition(pointAlongAxis, &renderer);

			}
			if (translatingY)
			{
				Line mouseLine;
				mouseLine.point = cam.position - yAxisOffset;
				mouseLine.direction = cam.direction;

				Line axisLine;
				axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
				axisLine.direction = Vec3f(0.0f, 1.0f, 0.0f);

				Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

				editingMesh->SetPosition(pointAlongAxis, &renderer);
			}
			if (translatingZ)
			{
				Line mouseLine;
				mouseLine.point = cam.position - zAxisOffset;
				mouseLine.direction = cam.direction;

				Line axisLine;
				axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
				axisLine.direction = Vec3f(0.0f, 0.0f, 1.0f);

				Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

				editingMesh->SetPosition(pointAlongAxis, &renderer);
			}
			if (rotatingX)
			{
				// Get intersection between mouse ray and x plane
				Plane xPlane;
				xPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
				xPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);

				RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), xPlane);

				Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
				if (intersection.hit)
				{
					Vec3f point = intersection.hitPoint - objectCenter;
					Vec3f unitVec = Vec3f(0.0f, 0.0f, 1.0f);
					float dot = Math::dot(point, unitVec);
					float det = Math::dot(xPlane.normal, (Math::cross(point, unitVec)));
					float angle = atan2(det, dot);
					
					float deltaAngle = xAngleLast - angle;

					xAngleLast = angle;

					editingMesh->RotateMeshAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), deltaAngle, &renderer);
					
					DebugLineSegment debugLine;
					debugLine.a = point + objectCenter;
					debugLine.b = objectCenter;
					renderer.DebugDrawLineSegment(debugLine);

					debugLine.a = unitVec + objectCenter;
					renderer.DebugDrawLineSegment(debugLine);
				}
			}
			if (rotatingY)
			{
				// Get intersection between mouse ray and y plane
				Plane yPlane;
				yPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
				yPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);

				RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), yPlane);

				Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
				if (intersection.hit)
				{
					Vec3f point = intersection.hitPoint - objectCenter;
					Vec3f unitVec = Vec3f(1.0f, 0.0f, 0.0f);
					float dot = Math::dot(point, unitVec);
					float det = Math::dot(yPlane.normal, (Math::cross(point, unitVec)));
					float angle = atan2(det, dot);

					float deltaAngle = yAngleLast - angle;

					yAngleLast = angle;

					editingMesh->RotateMeshAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), deltaAngle, &renderer);

					DebugLineSegment debugLine;
					debugLine.a = point + objectCenter;
					debugLine.b = objectCenter;
					renderer.DebugDrawLineSegment(debugLine);

					debugLine.a = unitVec + objectCenter;
					renderer.DebugDrawLineSegment(debugLine);
				}
			}
			if (rotatingZ)
			{
				// Get intersection between mouse ray and z plane
				Plane zPlane;
				zPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
				zPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);

				RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), zPlane);

				Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
				if (intersection.hit)
				{
					Vec3f point = intersection.hitPoint - objectCenter;
					Vec3f unitVec = Vec3f(0.0f, 1.0f, 0.0f);
					float dot = Math::dot(point, unitVec);
					float det = Math::dot(zPlane.normal, (Math::cross(point, unitVec)));
					float angle = atan2(det, dot);

					float deltaAngle = zAngleLast - angle;

					zAngleLast = angle;

					editingMesh->RotateMeshAroundAxis(Vec3f(0.0f, 0.0f, 1.0f), deltaAngle, &renderer);
				
					DebugLineSegment debugLine;
					debugLine.a = point + objectCenter;
					debugLine.b = objectCenter;
					renderer.DebugDrawLineSegment(debugLine);

					debugLine.a = unitVec + objectCenter;
					renderer.DebugDrawLineSegment(debugLine);
				}
			}
		}
		else {
			holdingMouseLeft = true;

			Ray testRay;
			testRay.origin = cam.position;
			testRay.direction = cam.direction;

			EditorRayCastInfo rayCastInfo = editorScene.RayCast(testRay);

			if (rayCastInfo.hitMesh)
			{
				if (rayCastInfo.hitType == HitType::MESH)
				{
					editorScene.UnselectSelectedEditableMesh();
					editorScene.SetEditableMeshSelected(rayCastInfo.hitMesh);

					editingMesh = rayCastInfo.hitMesh;
				}
				if (rayCastInfo.hitType == HitType::TOOL)
				{
					if (rayCastInfo.toolType == ToolType::X_ARROW)
					{
						xAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
						translatingX = true;
					}
					if (rayCastInfo.toolType == ToolType::Y_ARROW)
					{
						yAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
						translatingY = true;
					}
					if (rayCastInfo.toolType == ToolType::Z_ARROW)
					{
						zAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
						translatingZ = true;
					}
					if (rayCastInfo.toolType == ToolType::X_RING)
					{
						// Get intersection between mouse ray and x plane
						Plane xPlane;
						xPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
						xPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
						
						RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), xPlane);
						
						Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
						if (intersection.hit)
						{
							Vec3f point = intersection.hitPoint - objectCenter;
							Vec3f unitVec = Vec3f(0.0f, 0.0f, 1.0f);
							float dot = Math::dot(point, unitVec);
							float det = Math::dot(xPlane.normal, (Math::cross(point, unitVec)));
							float angle = atan2(det, dot);
							xAngleLast = angle;
							rotatingX = true;
						}
					}

					if (rayCastInfo.toolType == ToolType::Y_RING)
					{
						// Get intersection between mouse ray and y plane
						Plane yPlane;
						yPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
						yPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);

						RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), yPlane);

						Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
						if (intersection.hit)
						{
							Vec3f point = intersection.hitPoint - objectCenter;
							Vec3f unitVec = Vec3f(1.0f, 0.0f, 0.0f);
							float dot = Math::dot(point, unitVec);
							float det = Math::dot(yPlane.normal, (Math::cross(point, unitVec)));
							float angle = atan2(det, dot);
							yAngleLast = angle;
							rotatingY = true;
						}
					}
					if (rayCastInfo.toolType == ToolType::Z_RING)
					{
						// Get intersection between mouse ray and z plane
						Plane zPlane;
						zPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
						zPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);

						RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), zPlane);

						Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
						if (intersection.hit)
						{
							Vec3f point = intersection.hitPoint - objectCenter;
							Vec3f unitVec = Vec3f(0.0f, 1.0f, 0.0f);
							float dot = Math::dot(point, unitVec);
							float det = Math::dot(zPlane.normal, (Math::cross(point, unitVec)));
							float angle = atan2(det, dot);
							zAngleLast = angle;
							rotatingZ = true;
						}
					}
					
				}

			}
		}
	}
	else {
		translatingX = false;
		translatingY = false;
		translatingZ = false;

		rotatingX = false;
		rotatingY = false;
		rotatingZ = false;

		holdingMouseLeft = false;
	}

    renderer.SetCamTransform(cam.position, cam.direction, cam.up);

	if (inputs.keysDown.alt && !holdingAlt)
	{
		Engine::UnlockCursor();
		Engine::ShowCursor();
		cameraControlEnabled = false;

		holdingAlt = true;
	}
	else if (!inputs.keysDown.alt && holdingAlt)
	{
		holdingAlt = false;
	}
	if (inputs.keysDown.space && !holdingSpace)
	{
		Engine::LockCursor();
		Engine::HideCursor();
		cameraControlEnabled = true;
		
		holdingSpace = true;
	}
	else if (!inputs.keysDown.space && holdingSpace)
	{
		holdingSpace = false;
	}

	if (inputs.keysDown.tab && !holdingTab)
	{
		holdingTab = true;
	}
	else if (!inputs.keysDown.tab && holdingTab)
	{
		holdingTab = false;
		editorScene.CycleToolType();
	}

    renderer.ClearScreen();

    editorScene.DrawScene();

    renderer.SwapBuffer();
}