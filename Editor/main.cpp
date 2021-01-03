#include "GameEngine.hpp"

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

Scene editorScene;

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

	unitCube = MeshGenerator::GenCube(renderer);

    Texture cubeTexture = renderer.LoadTexture("textures/cubeTexture.png");

	renderer.SetMeshTexture(unitCube, cubeTexture);
	
	//editorScene.AddEditableMesh(unitCube, &renderer);
	
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

	arrowMesh = ModelLoader::LoadOBJFile("models/boxbbox.obj", renderer);
	Texture boxTexture = renderer.LoadTexture("textures/Cube.png");
	renderer.SetMeshTexture(arrowMesh, boxTexture);
	editorScene.AddEditableMesh(arrowMesh, &renderer);

	hoopMesh = ModelLoader::LoadOBJFile("models/TranslatePad.obj", renderer);
	editorScene.AddEditableMesh(hoopMesh, &renderer);

	groundMesh = ModelLoader::LoadOBJFile("models/TestMountain.obj", renderer);
	Texture planeTexture = renderer.LoadTexture("textures/Gravel.jpg");
	renderer.SetMeshTexture(groundMesh, planeTexture);
	editorScene.AddEditableMesh(groundMesh, &renderer);

	renderer.SetSunDirection(Vec3f(0.0f, 0.0f, -1.0f));
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
		Ray testRay;
		testRay.origin = cam.position;
		testRay.direction = cam.direction;

		auto rayCast = editorScene.RayCast(testRay);
		if (rayCast.m_Hit.hit)
		{
			editorScene.UnselectSelectedEditableMesh();
			editorScene.SetEditableMeshSelected(rayCast.m_HitMesh);
		}
	}

    renderer.SetCamTransform(cam.position, cam.direction, cam.up);

    renderer.ClearScreen();

    editorScene.DrawScene(&renderer);

    renderer.SwapBuffer();
}