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
static bool holdingTab = false;

static bool translatingX = false;
static bool translatingY = false;
static bool translatingZ = false;

static bool rotatingX = false;
static bool rotatingY = false;
static bool rotatingZ = false;

static Vec3f xAxisOffset;
static Vec3f yAxisOffset;
static Vec3f zAxisOffset;

static float xAngleOffset;
static float yAngleOffset;
static float zAngleOffset;

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



	//editorScene.AddEditableMesh(groundPlane, &renderer);

	//arrowMesh = FileLoader::LoadOBJFile("models/boxbbox.obj", renderer);
	//Texture boxTexture = renderer.LoadTexture("textures/Cube.png");
	//renderer.SetMeshTexture(arrowMesh, boxTexture);
	//editorScene.AddEditableMesh(arrowMesh);

	//hoopMesh = FileLoader::LoadOBJFile("models/TranslatePad.obj", renderer);
	//editorScene.AddEditableMesh(hoopMesh);

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

	MoveCamera(inputs, cam, 0.001f);

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
				Vec3f axisNormal = Vec3f(0.0f, 1.0f, 0.0f);
				float denom = Math::dot(axisNormal, cam.direction);
				float t = Math::dot(renderer.GetMeshPosition(editingMesh->m_Mesh) - cam.position, axisNormal) / denom;

				Vec3f intersection = cam.position + (cam.direction * t);

				Vec3f vec = intersection - renderer.GetMeshPosition(editingMesh->m_Mesh);


				Vec3f xAxis = Vec3f(0.0f, 0.0f, 1.0f);
				
				float dot = Math::dot(vec, xAxis);
				float det = (vec.x * xAxis.y * axisNormal.z) + (xAxis.x * axisNormal.y * vec.z) + axisNormal.x * vec.y * xAxis.z - vec.z * xAxis.y * axisNormal.x - xAxis.z * axisNormal.y * vec.x - axisNormal.z * vec.y * xAxis.x;

				float angle = -atan2(det, dot);
				//float angle = std::acos(Math::dot(vec, xAxis) / (Math::magnitude(vec) * Math::magnitude(xAxis)));
				
				
				editingMesh->SetRotationAroundYAxis((angle - xAngleOffset) * 57.2958, &renderer);

			}
			if (rotatingY)
			{
				// Get intersection between mouse ray and y plane
				Vec3f axisNormal = Vec3f(1.0f, 0.0f, 0.0f);
				float denom = Math::dot(axisNormal, cam.direction);
				float t = Math::dot(renderer.GetMeshPosition(editingMesh->m_Mesh) - cam.position, axisNormal) / denom;

				Vec3f intersection = cam.position + (cam.direction * t);

				Vec3f vec = intersection - renderer.GetMeshPosition(editingMesh->m_Mesh);


				Vec3f xAxis = Vec3f(0.0f, 1.0f, 0.0f);

				float dot = Math::dot(vec, xAxis);
				float det = (vec.x * xAxis.y * axisNormal.z) + (xAxis.x * axisNormal.y * vec.z) + axisNormal.x * vec.y * xAxis.z - vec.z * xAxis.y * axisNormal.x - xAxis.z * axisNormal.y * vec.x - axisNormal.z * vec.y * xAxis.x;

				float angle = -atan2(det, dot);
				//float angle = std::acos(Math::dot(vec, xAxis) / (Math::magnitude(vec) * Math::magnitude(xAxis)));


				editingMesh->SetRotationAroundXAxis((angle - yAngleOffset) * 57.2958, &renderer);

			}
			if (rotatingZ)
			{
				// Get intersection between mouse ray and y plane
				Vec3f axisNormal = Vec3f(0.0f, 0.0f, 1.0f);
				float denom = Math::dot(axisNormal, cam.direction);
				float t = Math::dot(renderer.GetMeshPosition(editingMesh->m_Mesh) - cam.position, axisNormal) / denom;

				Vec3f intersection = cam.position + (cam.direction * t);

				Vec3f vec = intersection - renderer.GetMeshPosition(editingMesh->m_Mesh);


				Vec3f xAxis = Vec3f(1.0f, 0.0f, 0.0f);

				float dot = Math::dot(vec, xAxis);
				float det = (vec.x * xAxis.y * axisNormal.z) + (xAxis.x * axisNormal.y * vec.z) + axisNormal.x * vec.y * xAxis.z - vec.z * xAxis.y * axisNormal.x - xAxis.z * axisNormal.y * vec.x - axisNormal.z * vec.y * xAxis.x;

				float angle = -atan2(det, dot);
				//float angle = std::acos(Math::dot(vec, xAxis) / (Math::magnitude(vec) * Math::magnitude(xAxis)));


				editingMesh->SetRotationAroundZAxis((angle - zAngleOffset) * 57.2958, &renderer);
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
						Vec3f axisNormal = Vec3f(0.0f, 1.0f, 0.0f);
						float denom = Math::dot(axisNormal, cam.direction);
						float t = Math::dot(renderer.GetMeshPosition(editingMesh->m_Mesh) - cam.position, axisNormal) / denom;

						Vec3f intersection = cam.position + (cam.direction * t);
						Vec3f vec = intersection - renderer.GetMeshPosition(editingMesh->m_Mesh);

						Vec3f xAxis = Vec3f(0.0f, 0.0f, 1.0f);

						float dot = Math::dot(vec, xAxis);
						float det = (vec.x * xAxis.y * axisNormal.z) + (xAxis.x * axisNormal.y * vec.z) + axisNormal.x * vec.y * xAxis.z - vec.z * xAxis.y * axisNormal.x - xAxis.z * axisNormal.y * vec.x - axisNormal.z * vec.y * xAxis.x;

						float angle = atan2(det, dot);
						xAngleOffset = -angle - renderer.GetMeshRotationAroundYAxis(editingMesh->m_Mesh) * 0.0174533;
						rotatingX = true;
					}
					if (rayCastInfo.toolType == ToolType::Y_RING)
					{
						// Get intersection between mouse ray and x plane
						Vec3f axisNormal = Vec3f(1.0f, 0.0f, 0.0f);
						float denom = Math::dot(axisNormal, cam.direction);
						float t = Math::dot(renderer.GetMeshPosition(editingMesh->m_Mesh) - cam.position, axisNormal) / denom;

						Vec3f intersection = cam.position + (cam.direction * t);
						Vec3f vec = intersection - renderer.GetMeshPosition(editingMesh->m_Mesh);

						Vec3f xAxis = Vec3f(0.0f, 1.0f, 0.0f);

						float dot = Math::dot(vec, xAxis);
						float det = (vec.x * xAxis.y * axisNormal.z) + (xAxis.x * axisNormal.y * vec.z) + axisNormal.x * vec.y * xAxis.z - vec.z * xAxis.y * axisNormal.x - xAxis.z * axisNormal.y * vec.x - axisNormal.z * vec.y * xAxis.x;

						float angle = atan2(det, dot);
						yAngleOffset = -angle - renderer.GetMeshRotationAroundXAxis(editingMesh->m_Mesh) * 0.0174533;
						rotatingY = true;
					}
					if (rayCastInfo.toolType == ToolType::Z_RING)
					{
						// Get intersection between mouse ray and x plane
						Vec3f axisNormal = Vec3f(0.0f, 0.0f, 1.0f);
						float denom = Math::dot(axisNormal, cam.direction);
						float t = Math::dot(renderer.GetMeshPosition(editingMesh->m_Mesh) - cam.position, axisNormal) / denom;

						Vec3f intersection = cam.position + (cam.direction * t);
						Vec3f vec = intersection - renderer.GetMeshPosition(editingMesh->m_Mesh);

						Vec3f xAxis = Vec3f(1.0f, 0.0f, 0.0f);

						float dot = Math::dot(vec, xAxis);
						float det = (vec.x * xAxis.y * axisNormal.z) + (xAxis.x * axisNormal.y * vec.z) + axisNormal.x * vec.y * xAxis.z - vec.z * xAxis.y * axisNormal.x - xAxis.z * axisNormal.y * vec.x - axisNormal.z * vec.y * xAxis.x;

						float angle = atan2(det, dot);
						zAngleOffset = -angle - renderer.GetMeshRotationAroundZAxis(editingMesh->m_Mesh) * 0.0174533;
						rotatingZ = true;
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

	if (inputs.keysDown.alt)
	{
		Engine::UnlockCursor();
		Engine::ShowCursor();
	}
	if (inputs.keysDown.space)
	{
		Engine::LockCursor();
		Engine::HideCursor();
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