//#include "GameEngine.hpp"
//
//#include "include/stb_image.h"
//
//#include "CollisionGeometry.h"
//
//
//struct Camera
//{
//	Vec3f position = Vec3f(0.0f, 0.0f, 0.0f);
//	Vec3f direction = Vec3f(0.0f, 1.0f, 0.0f);
//	Vec3f up = Vec3f(0.0f, 0.0f, 1.0f);
//};
//
//enum class GameMode
//{
//	EDITOR,
//	MENU,
//	PLAY,
//	NO_CHANGE
//};
//
//static GameMode currentGameMode = GameMode::EDITOR;
//static Camera cam;
//
//unsigned int testTextureID;
//static Mesh testMesh;
//static CollisionGeometry geometry;
//
//static Scene testScene;
//
//static std::vector<Vec3f> points;
//
//static unsigned int width, height;
//
//void MoveCamera(ControlInputs& inputs, Camera& cam, float pixelToRadians)
//{
//	cam.direction = Math::rotate(cam.direction, -(float)(inputs.DeltaMouse.x) * pixelToRadians, Vec3f(0.0f, 0.0f, 1.0f));
//
//	Vec3f perpVector = Math::cross(cam.direction, cam.up);
//
//	cam.direction = Math::rotate(cam.direction, -(float)(inputs.DeltaMouse.y) * pixelToRadians, perpVector);
//
//	if (inputs.keysDown.w)
//	{
//		cam.position += cam.direction * 0.02f;
//	}
//	if (inputs.keysDown.s)
//	{
//		cam.position -= cam.direction * 0.02f;
//	}
//	if (inputs.keysDown.a)
//	{
//		cam.position -= perpVector * 0.02f;
//	}
//	if (inputs.keysDown.d)
//	{
//		cam.position += perpVector * 0.02f;
//	}
//}
//
//
//void Initialize(Renderer& renderer)
//{
//	Mesh unitCube = MeshGenerator::GenCube(renderer);
//	testScene.LoadScene("maps/testmap.map", &renderer);
//	
//	testScene.SaveScene("maps/yeetus.map");
//
//	Engine::LockCursor();
//
//	testTextureID = renderer.LoadTexture("player.png");
//	std::vector<Vertex> vertices;
//	std::vector<unsigned int> indices;
//
//	int hMapWidth, hMapHeight, hMapChannels;
//	unsigned char* heightmapData = stbi_load("relief.png", &hMapWidth, &hMapHeight, &hMapChannels, 0);
//
//	width = hMapWidth;
//	height = hMapHeight;
//
//	auto CoordsToIndex = [&](unsigned int x, unsigned int y) -> int
//	{
//		if (x > width - 1 || x < 0 || y > height - 1 || y < 0)
//		{
//			return -1;
//		}
//		else
//		{
//			return (int)(((y * width) + x) * hMapChannels);
//		}
//	};
//
//	float quadSize = 0.01f;
//
//	for (unsigned int i = 0; i < height; ++i)
//	{
//		for (unsigned int j = 0; j < width; ++j)
//		{
//
//			int hMapIndex = CoordsToIndex(j, i);
//
//			float h = ((float)heightmapData[hMapIndex] / 255.f) * 2.0f;
//
//			float r = h * 0.5f;
//			float g = h * 0.25f;
//			float b = 1.0f - (h * 0.5f);
//			float a = 1.0f;
//
//			float x = static_cast<float>(j);
//			float y = static_cast<float>(i);
//
//			// Surface normal calculation
//			int indexLeft = CoordsToIndex(j - 1, i);
//			int indexRight = CoordsToIndex(j + 1, i);
//			int indexUp = CoordsToIndex(j, i + 1);
//			int indexDown = CoordsToIndex(j, i - 1);
//
//			int indexTopLeft = CoordsToIndex(j - 1, i + 1);
//			int indexTopRight = CoordsToIndex(j + 1, i + 1);
//			int indexBotLeft = CoordsToIndex(j - 1, i - 1);
//			int indexBotRight = CoordsToIndex(j + 1, i - 1);
//
//			Vec3f normal;
//			if (indexLeft == -1 || indexRight == -1 || indexUp == -1 || indexDown == -1 ||
//				indexTopLeft == -1 || indexTopRight == -1 || indexBotLeft == -1 || indexBotRight == -1)
//			{
//				normal.x = 0.0f;
//				normal.y = 0.0f;
//				normal.z = 1.0f;
//			}
//			else
//			{
//				normal.x = (float)(heightmapData[indexTopRight] - heightmapData[indexTopLeft]) +
//					(float)(heightmapData[indexBotRight] - heightmapData[indexBotLeft]) +
//					2.0f * (float)(heightmapData[indexRight] - heightmapData[indexLeft]);
//
//				normal.y = (float)(heightmapData[indexTopLeft] - heightmapData[indexBotLeft]) +
//					(float)(heightmapData[indexTopRight] - heightmapData[indexBotRight]) +
//					2.0f * (float(heightmapData[indexUp] - heightmapData[indexDown]));
//				normal.z = 1.0f;
//
//				normal = Math::normalize(normal);
//			}
//
//			// TEMP:
//			//normal = { 0.0f, 0.0f, 0.0f };
//			//
//			////r = 0.0f * 0.5f;
//			////g = 0.0f * 0.25f;
//			////b = 1.0f - (0.0f * 0.5f);
//			//r = (float)rand() / RAND_MAX;
//			//g = (float)rand() / RAND_MAX;
//			//b = (float)rand() / RAND_MAX;
//
//
//			a = 1.0f;
//
//			//Vertex newVertex = { {x * quadSize, y * quadSize, h}, normal, {r, g, b, a}, {x * quadSize / (quadSize * width), y * quadSize / (quadSize * height)} };
//
//			Vertex newVertex = { {x * quadSize, y * quadSize, h}, normal, {r, g, b, a}, {x, y} };
//			//Vertex newVertex = { {x * quadSize, y * quadSize, 0.0f}, normal, {r, g, b, a}, {x, y} };
//
//			points.push_back(Vec3f(x * quadSize, y * quadSize, h));
//			//points.push_back(Vec3f(x * quadSize, y * quadSize, 0.0f));
//
//
//			vertices.push_back(newVertex);
//
//
//		}
//	}
//
//	stbi_image_free(heightmapData);
//
//	for (unsigned int i = 0; i < height - 1; ++i)
//	{
//		for (unsigned int j = 0; j < width - 1; ++j)
//		{
//			unsigned int startIndex = j + (i * width);
//
//			indices.push_back(startIndex);
//			indices.push_back(startIndex + width);
//			indices.push_back(startIndex + width + 1);
//
//			indices.push_back(startIndex);
//			indices.push_back(startIndex + width + 1);
//			indices.push_back(startIndex + 1);
//
//			// Add to collision geometry
//
//			Triangle firstTriangle, secondTriangle;
//
//			firstTriangle.a = &points[startIndex];
//			firstTriangle.b = &points[startIndex + width];
//			firstTriangle.c = &points[startIndex + width + 1];
//
//			secondTriangle.a = &points[startIndex];
//			secondTriangle.b = &points[startIndex + width + 1];
//			secondTriangle.c = &points[startIndex + 1];
//
//			geometry.AddTriangle(firstTriangle);
//			geometry.AddTriangle(secondTriangle);
//		}
//	}
//
//	testMesh = renderer.LoadMesh(vertices, indices);
//	testMesh.SetTexture(Texture(testTextureID));
//}
//
//GameMode UpdateEditor(Renderer& renderer, ControlInputs& inputs)
//{
//
//	if (inputs.keysDown.esc)
//	{
//		Engine::StopGame();
//	}
//
//
//	float pixelToRadians = 0.001f;
//
//	MoveCamera(inputs, cam, pixelToRadians);
//
//	renderer.SetCamTransform(cam.position, cam.direction, cam.up);
//	
//	if (inputs.keysDown.space)
//	{
//		renderer.SetSunDirection(cam.direction);
//
//	}
//
//	renderer.ClearScreen();
//
//	renderer.DrawMesh(testMesh);
//	//renderer.DrawMesh(playerMesh);
//
//	testScene.DrawScene(&renderer);
//
//	renderer.SwapBuffer();
//
//
//	if (inputs.mouse.rightMouseButton)
//	{
//
//		RayCastHit rayCastHit = geometry.RayCast(cam.position, cam.direction);
//		if (rayCastHit.hit)
//		{
//			//std::string hitRegString =
//			//	"We hit at point x:" + std::to_string(rayCastHit.hitPoint.x) +
//			//	", y:" + std::to_string(rayCastHit.hitPoint.y) +
//			//	", z:" + std::to_string(rayCastHit.hitPoint.z) + "\n";
//			//DEBUGPrint(hitRegString);
//
//			Vertex* mappedVertices;
//			//mappedVertices = (Vertex *)renderer.MapMeshRange(testMesh, 0, 5);
//			mappedVertices = (Vertex*)renderer.MapMesh(testMesh);
//
//
//			for (int i = 0; i < testMesh.numVertices; i++)
//			{
//				Vec3f rayHit = rayCastHit.hitPoint;
//				Vec3f point = points[i];
//				float distSquared = ((point.x - rayHit.x) * (point.x - rayHit.x)) +
//					((point.y - rayHit.y) * (point.y - rayHit.y)) +
//					((point.z - rayHit.z) * (point.z - rayHit.z));
//				float MAX_DISTANCE = 0.5f;
//
//				if (distSquared < MAX_DISTANCE * MAX_DISTANCE)
//				{
//					float actualDistance = sqrt(distSquared);
//					points[i].z += Math::clamp((MAX_DISTANCE - actualDistance), 0.0f, MAX_DISTANCE) * 0.05f;
//					
//					// TEMP:
//					mappedVertices[i].position = points[i];
//					mappedVertices[i].colour.x = points[i].z * 0.5f;
//					mappedVertices[i].colour.y = points[i].z * 0.25f;
//					mappedVertices[i].colour.z = 1.0f - (points[i].z * 0.5f);
//
//
//					Vec3f normal;
//
//
//				}
//			}
//			renderer.UnmapMesh(testMesh);
//			
//		}
//		else
//		{
//			//DEBUGPrint("WE DIDN'T HIT IT.\n");
//		}
//	}
//
//	return GameMode::NO_CHANGE;
//}
//
//
//
//GameMode UpdatePlay(Renderer& renderer, ControlInputs& inputs)
//{
//	return GameMode::NO_CHANGE;
//}
//
//
//
//void Update(Renderer& renderer, ControlInputs& inputs)
//{
//	switch (currentGameMode)
//	{
//	case GameMode::EDITOR:
//	{
//		UpdateEditor(renderer, inputs);
//	} break;
//
//	case GameMode::PLAY:
//	{
//		UpdatePlay(renderer, inputs);
//	} break;
//	}
//}
