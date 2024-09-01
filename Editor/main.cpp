#if 0
#include "GameEngine.h"

#include "EditorScene.h"

#include "Scene.h"

#include <cmath>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>

struct BrushEdge;

struct BrushVert
{
    Transform m_Transform;
    BrushEdge* m_Edge;
};

struct BrushEdge
{
};

struct BrushFace
{
    BrushEdge* m_Edge;
};

struct Brush
{
    std::vector<BrushFace> m_Faces;

};

void DebugDrawBrush(Brush& brush, GraphicsModule& graphics)
{
    for (BrushFace& face : brush.m_Faces)
    {

    }
}

static bool FirstPersonPlayerEnabled = false;

float TransSnap = 1.0f;
float RotSnap = Math::Pi() / 18.0f;

static Vec3f SunLight = Vec3f(1.0f, 1.0f, 1.0f);

static Vec3f Black = Vec3f(0.5f, 0.5f, 0.5f);

struct Player
{
    bool grounded = false;
    Vec3f position;
    Vec3f velocity;
    Camera* cam = nullptr;
};

enum class State
{
    EDITOR,
    GAME
};

enum class ToolMode
{
    SELECT,
    GEOMETRY,
    MOVE,
    VERTEX,
    SCULPT
};

enum class GeometryMode
{
    BOX,
    PLANE
};

enum class MoveMode
{
    TRANSLATE,
    ROTATE,
    SCALE
};

RenderMode renderMode = RenderMode::DEFAULT;

Player player;

Texture playButtonTexture;
Texture cameraButtonTexture;

Texture cursorToolTexture;

Texture boxToolTexture;
Texture planeToolTexture;

Texture translateToolTexture;
Texture rotateToolTexture;
Texture scaleToolTexture;

Texture vertexToolTexture;

Texture sculptToolTexture;
Texture lightToolTexture;

Material tempWhiteMaterial;

Texture gridTexture;
Model gridModel;

Font testFont;
Font inspectorFont;

Camera cam;

Framebuffer_ID viewportBuffer;
Framebuffer_ID widgetViewportBuffer;

GBuffer gBuffer;

bool holdingAlt = false;
bool cursorLocked = false;

bool gridEnabled = true;

std::vector<Material> loadedMaterials;
std::vector<Model> loadedModels;

std::vector<Framebuffer_ID> modelFBuffers;
Camera modelCam;

Scene scene;
Scene runtimeScene;

bool draggingNewModel = false;
Model* draggingModel = nullptr;

bool draggingNewTexture = false;
Material draggingMaterial;

bool draggingNewBehaviour = false;
std::string draggingBehaviourName;

bool draggingNewPointLight = false;

static Model* selectedModelPtr = nullptr;
static Transform* selectedTransformPtr = nullptr;
//static CollisionMesh* selectedColMeshPtr = nullptr;

Model xAxisArrow;
Model yAxisArrow;
Model zAxisArrow;

Model xAxisRing;
Model yAxisRing;
Model zAxisRing;

Model xScaleWidget;
Model yScaleWidget;
Model zScaleWidget;

ToolMode toolMode = ToolMode::SELECT;
GeometryMode geometryMode = GeometryMode::BOX;
MoveMode moveMode = MoveMode::TRANSLATE;

State state = State::EDITOR;

std::string ipString;

bool m_IsServer = false;
bool m_IsClient = false;

Model OtherPlayerModel;

int PrevFrameTimeCount = 0;
float PrevFrameTimeSum = 0.0f;
int PrevAveFPS = 0;

void CycleMoveMode()
{
    if (moveMode == MoveMode::TRANSLATE)
    {
        moveMode = MoveMode::ROTATE;
    }
    else if (moveMode == MoveMode::ROTATE)
    {
        moveMode = MoveMode::SCALE;
    }
    else if (moveMode == MoveMode::SCALE)
    {
        moveMode = MoveMode::TRANSLATE;
    }
}

void CycleGeometryMode()
{
    if (geometryMode == GeometryMode::BOX)
    {
        geometryMode = GeometryMode::PLANE;
    }
    else if (geometryMode == GeometryMode::PLANE)
    {
        geometryMode = GeometryMode::BOX;
    }
}

void DrawToolWidgets()
{
    GraphicsModule& graphics = *GraphicsModule::Get();
    CollisionModule& collisions = *CollisionModule::Get();

    // Draw tools on separate buffer so they're in front
    graphics.SetActiveFrameBuffer(widgetViewportBuffer);
    {
        if (selectedTransformPtr && toolMode == ToolMode::MOVE)
        {
            graphics.SetRenderMode(RenderMode::FULLBRIGHT);
            if (moveMode == MoveMode::TRANSLATE)
            {
                Vec3f movingModelPos = selectedTransformPtr->GetPosition();

                float scale = Math::magnitude(movingModelPos - cam.GetPosition()) * 0.05f;

                xAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(4.0f, 0.0f, 0.0f) * scale);
                yAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(0.0f, 4.0f, 0.0f) * scale);
                zAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(0.0f, 0.0f, 4.0f) * scale);

                xAxisArrow.GetTransform().SetScale(Vec3f(scale / 3.f, scale, scale / 3.f));
                yAxisArrow.GetTransform().SetScale(Vec3f(scale / 3.f, scale, scale / 3.f));
                zAxisArrow.GetTransform().SetScale(Vec3f(scale / 3.f, scale, scale / 3.f));

                //graphics.DebugDrawAABB(collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox, Vec3f(1.0f, 1.0f, 0.7f), selectedModelPtr->GetTransform().GetTransformMatrix());

                graphics.Draw(xAxisArrow);
                graphics.Draw(yAxisArrow);
                graphics.Draw(zAxisArrow);
            }

            if (moveMode == MoveMode::ROTATE)
            {
                Vec3f rotatingModelPos = selectedTransformPtr->GetPosition();

                xAxisRing.GetTransform().SetPosition(rotatingModelPos);
                yAxisRing.GetTransform().SetPosition(rotatingModelPos);
                zAxisRing.GetTransform().SetPosition(rotatingModelPos);

                float scale = Math::magnitude(rotatingModelPos - cam.GetPosition()) * 0.2f;

                xAxisRing.GetTransform().SetScale(Vec3f(scale, scale, scale));
                yAxisRing.GetTransform().SetScale(Vec3f(scale, scale, scale));
                zAxisRing.GetTransform().SetScale(Vec3f(scale, scale, scale));

                //graphics.DebugDrawAABB(collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox, Vec3f(1.0f, 1.0f, 0.7f), selectedModelPtr->GetTransform().GetTransformMatrix());

                graphics.Draw(xAxisRing);
                graphics.Draw(yAxisRing);
                graphics.Draw(zAxisRing);

            }
            if (moveMode == MoveMode::SCALE)
            {
                Vec3f scalingModelPos = selectedTransformPtr->GetPosition();
                Quaternion scalingModelRot = selectedTransformPtr->GetRotation();

                Vec3f modelScale = selectedTransformPtr->GetScale();
                float scale = Math::magnitude(scalingModelPos - cam.GetPosition()) * 0.05f;

                xScaleWidget.GetTransform().SetScale(Vec3f(scale, scale, scale));
                yScaleWidget.GetTransform().SetScale(Vec3f(scale, scale, scale));
                zScaleWidget.GetTransform().SetScale(Vec3f(scale, scale, scale));

                xScaleWidget.GetTransform().SetPosition(scalingModelPos + Vec3f(modelScale.x, 0.0f, 0.0f) * scalingModelRot);
                yScaleWidget.GetTransform().SetPosition(scalingModelPos + Vec3f(0.0f, modelScale.y, 0.0f) * scalingModelRot);
                zScaleWidget.GetTransform().SetPosition(scalingModelPos + Vec3f(0.0f, 0.0f, modelScale.z) * scalingModelRot);

                xScaleWidget.GetTransform().SetRotation(scalingModelRot * Quaternion(Vec3f(0.0f, 1.0f, 0.0f), (float)M_PI_2));
                yScaleWidget.GetTransform().SetRotation(scalingModelRot * Quaternion(Vec3f(1.0f, 0.0f, 0.0f), (float)-M_PI_2));
                zScaleWidget.GetTransform().SetRotation(scalingModelRot * Quaternion(Vec3f(0.0f, 0.0f, 1.0f), (float)M_PI_2));

                graphics.Draw(xScaleWidget);
                graphics.Draw(yScaleWidget);
                graphics.Draw(zScaleWidget);
            }
        }
        graphics.SetRenderMode(renderMode);
    }
    graphics.ResetFrameBuffer();
}

Rect GetViewportSizeFromScreenSize(Vec2i screenSize)
{
    Rect newViewport = Rect(Vec2f(100.0f, 40.0f), screenSize - Vec2f(300, 240));

    return newViewport;
} 

void DrawEditorGrid(GraphicsModule& graphics)
{
    static float EDITOR_GRID_SIZE = 500.0f;
    static int EDITOR_GRID_NUM = 50;

    static Vec3f gridColour = Vec3f(0.4f, 0.4f, 0.4f);
    static Vec3f gridSecondaryColour = Vec3f(167.0f / 255.0f, 120.0f / 255.0f, 38.0f / 255.0f);

    for (int i = 0; i < EDITOR_GRID_NUM + 1; ++i)
    {
        Vec3f thisGridColour = i % 5 == 0 ? gridSecondaryColour : gridColour;
        graphics.DebugDrawLine(
            Vec3f(EDITOR_GRID_SIZE / 2.0f, (i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, 0.0f),
            Vec3f(-EDITOR_GRID_SIZE / 2.0f, (i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, 0.0f),
            thisGridColour
        );

        graphics.DebugDrawLine(
            Vec3f((i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, EDITOR_GRID_SIZE / 2.0f, 0.0f),
            Vec3f((i * (EDITOR_GRID_SIZE / EDITOR_GRID_NUM)) - EDITOR_GRID_SIZE / 2.0f, -EDITOR_GRID_SIZE / 2.0f, 0.0f),
            thisGridColour
        );
    }
}

Ray GetMouseRay(Camera& cam, Vec2i mousePosition, Rect viewPort)
{
    Mat4x4f invCamMatrix = cam.GetInvCamMatrix();

    Vec3f a, b;

    Vec2i screenSize = viewPort.size;

    Vec2i mousePos;
    mousePos.x = mousePosition.x - (int)viewPort.location.x;
    mousePos.y = mousePosition.y - (int)viewPort.location.y;

    float x = ((2.0f * (float)mousePos.x) / (float)screenSize.x) - 1.0f;
    float y = 1.0f - ((2.0f * (float)mousePos.y) / (float)screenSize.y);
    float z = 1.0f;

    Vec3f ray_nds = Vec3f(x, y, z);

    Vec4f ray_clip = Vec4f(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

    Vec4f ray_eye = ray_clip * Math::inv(cam.GetProjectionMatrix());

    ray_eye = Vec4f(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    Vec4f ray_world = ray_eye * Math::inv(cam.GetViewMatrix());

    Vec3f ray = Vec3f(ray_world.x, ray_world.y, ray_world.z);
    ray = Math::normalize(ray);

    return Ray(cam.GetPosition(), ray);
}

void MoveCamera(InputModule& inputs, GraphicsModule& graphics, Camera& cam, float pixelToRadians, double deltaTime)
{
    const float CamSpeed = 10.0f;

    float speed = CamSpeed * (float)deltaTime;

    if (inputs.IsKeyDown(Key::Shift))
    {
        speed *= 5.0f;
    }

    if (inputs.IsKeyDown(Key::W))
    {
        cam.Move(cam.GetDirection() * speed);
    }
    if (inputs.IsKeyDown(Key::S))
    {
        cam.Move(-cam.GetDirection() * speed);
    }
    if (inputs.IsKeyDown(Key::D))
    {
        cam.Move(Math::normalize(cam.GetPerpVector()) * speed);
    }
    if (inputs.IsKeyDown(Key::A))
    {
        cam.Move(-Math::normalize(cam.GetPerpVector()) * speed);
    }

    if (inputs.IsKeyDown(Key::Space))
    {
        cam.Move(Vec3f(0.0f, 0.0f, speed));
    }
    if (inputs.IsKeyDown(Key::Ctrl))
    {
        cam.Move(Vec3f(0.0f, 0.0f, -speed));
    }

    cam.RotateCamBasedOnDeltaMouse(inputs.GetMouseState().GetDeltaMousePos(), pixelToRadians);

    if (inputs.IsKeyDown(Key::Q))
    {
       
        scene.SetDirectionalLight(DirectionalLight{ cam.GetDirection(), SunLight });

        //graphics.m_Renderer.SetShaderUniformVec3f(graphics.m_TexturedMeshShader, "SunDirection", cam.GetDirection());
    }
}

std::vector<Model> LoadModels(GraphicsModule& graphics)
{
    AssetRegistry* Registry = AssetRegistry::Get();

    auto CurrentPath = std::filesystem::current_path();

    Engine::DEBUGPrint(CurrentPath.generic_string());

    std::vector<Model> loadedModels;

    std::string path = "models";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        if (ext.string() == ".obj")
        {
            std::string fileName = entry.path().generic_string();

            Engine::DEBUGPrint(fileName);

            StaticMesh newMesh = *Registry->LoadStaticMesh(fileName);

            // For now we load all models with a temporary white texture
            Model newModel = graphics.CreateModel(TexturedMesh(newMesh, tempWhiteMaterial));

            loadedModels.push_back(newModel);
            modelFBuffers.push_back(graphics.CreateFBuffer(Vec2i(100, 100), FBufferFormat::COLOUR));
        }

    }

    return loadedModels;
}

std::vector<Material> LoadMaterials(GraphicsModule& graphics)
{
    AssetRegistry* Registry = AssetRegistry::Get();

    std::vector<Material> loadedMaterials;

    std::string path = "textures";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        std::string extensionString = ext.string();
        if (extensionString == ".png" || extensionString == ".jpg")
        {
            std::string fileName = entry.path().generic_string();

            if (StringUtils::Contains(fileName, ".norm."))
            {
                continue;
            }
            if (StringUtils::Contains(fileName, ".metal."))
            {
                continue;
            }
            if (StringUtils::Contains(fileName, ".rough."))
            {
                continue;
            }
            if (StringUtils::Contains(fileName, ".ao."))
            {
                continue;
            }

            Engine::DEBUGPrint(fileName);

            Texture newTexture = *Registry->LoadTexture(fileName);

            std::string NormalMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".norm.png";
            std::string RoughnessMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".rough.png";
            std::string MetallicMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".metal.png";
            std::string AOMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".ao.png";

            if (std::filesystem::exists(NormalMapString))
            {
                if (std::filesystem::exists(RoughnessMapString))
                {
                    if (std::filesystem::exists(MetallicMapString))
                    {
                        if (std::filesystem::exists(AOMapString))
                        {
                            Texture newNormal = *Registry->LoadTexture(NormalMapString);
                            Texture newRoughness = *Registry->LoadTexture(RoughnessMapString);
                            Texture newMetal = *Registry->LoadTexture(MetallicMapString);
                            Texture newAO = *Registry->LoadTexture(AOMapString);
                            loadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal, newRoughness, newMetal, newAO));
                        }
                        else
                        {
                            Texture newNormal = *Registry->LoadTexture(NormalMapString);
                            Texture newRoughness = *Registry->LoadTexture(RoughnessMapString);
                            Texture newMetal = *Registry->LoadTexture(MetallicMapString);
                            loadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal, newRoughness, newMetal));
                        }
                    }
                    else
                    {
                        Texture newNormal = *Registry->LoadTexture(NormalMapString);
                        Texture newRoughness = *Registry->LoadTexture(RoughnessMapString);
                        loadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal, newRoughness));
                    }
                }
                else
                {
                    Texture newNormal = *Registry->LoadTexture(NormalMapString);
                    loadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal));
                }
            }
            else
            {
                loadedMaterials.push_back(graphics.CreateMaterial(newTexture));
            }

        }
    }

    return loadedMaterials;
}

void UpdateSelectTool(InputModule& input, CollisionModule& collisions)
{
    if (toolMode != ToolMode::SELECT)
        return;
    
    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    if (draggingNewPointLight)
        return;

    if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        if (viewportRect.Contains(Engine::GetMousePosition()))
        {
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay);

            if (finalHit.rayCastHit.hit)
            {
                selectedTransformPtr = &finalHit.hitModel->GetTransform();
                selectedModelPtr = finalHit.hitModel;
            }
        }
    }
}

void UpdateBoxCreate(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool draggingNewBox = false;
    static Vec3f boxStartPoint;
    static float boxHeight = 2.0f;
    static AABB aabbBox;

    static const float snap = 1.0f;

    if (toolMode != ToolMode::GEOMETRY)
        return;

    if (geometryMode != GeometryMode::BOX)
        return;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;
    
    if (draggingNewPointLight)
        return;

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
    Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);
    
    if (!viewportRect.Contains(Engine::GetMousePosition()))
    {
        return;
    }

    if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
    {
        if (!draggingNewBox)
        {
            // Find intersect with horizontal plane as a default box start point
            RayCastHit finalHit = collisions.RayCast(mouseRay, Plane{ Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f) });

            // Test against existing level geometry
            SceneRayCastHit levelHit = scene.RayCast(mouseRay);

            if (levelHit.rayCastHit.hitDistance < finalHit.hitDistance) finalHit = levelHit.rayCastHit;

            if (finalHit.hit)
            {
                finalHit.hitPoint.x = Math::Round(finalHit.hitPoint.x, snap);
                finalHit.hitPoint.y = Math::Round(finalHit.hitPoint.y, snap);
                finalHit.hitPoint.z = Math::Round(finalHit.hitPoint.z, snap);

                boxStartPoint = finalHit.hitPoint;
                draggingNewBox = true;
            }
        }
        else
        {
            RayCastHit hit = collisions.RayCast(mouseRay, Plane{ boxStartPoint, Vec3f(0.0f, 0.0f, 1.0f) });
            
            int DeltaMouseWheel = input.GetMouseState().GetDeltaMouseWheel();
            if (DeltaMouseWheel > 0)
            {
                boxHeight += snap;
            }
            else if (DeltaMouseWheel < 0)
            {
                boxHeight -= snap;
            }

            Vec3f originalHitPoint = hit.hitPoint;

            hit.hitPoint.x = Math::Round(hit.hitPoint.x, snap);
            hit.hitPoint.y = Math::Round(hit.hitPoint.y, snap);
            hit.hitPoint.z = Math::Round(hit.hitPoint.z, snap);

            float minX = std::min(hit.hitPoint.x, boxStartPoint.x);
            float minY = std::min(hit.hitPoint.y, boxStartPoint.y);

            float maxX = std::max(hit.hitPoint.x, boxStartPoint.x);
            float maxY = std::max(hit.hitPoint.y, boxStartPoint.y);

            aabbBox.min = Vec3f(minX, minY, hit.hitPoint.z);
            aabbBox.max = Vec3f(maxX, maxY, hit.hitPoint.z + boxHeight);


            graphics.DebugDrawAABB(aabbBox, Vec3f(0.1f, 1.0f, 0.3f));
            graphics.DebugDrawLine(originalHitPoint, hit.hitPoint, Vec3f(1.0f, 0.5f, 0.5f));
        }
    }
    else
    {
        if (draggingNewBox)
        {
            Model* newBox = new Model(graphics.CreateBoxModel(aabbBox));
            scene.AddModel(*newBox);
            draggingNewBox = false;

            selectedTransformPtr = nullptr;
            selectedModelPtr = nullptr;
        }
    }

}

void UpdatePlaneCreate(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool draggingNewPlane = false;
    static Vec3f planeStartPoint;
    static Vec3f planeMin, planeMax;

    static int subdivisions = 1;

    static const float snap = 1.0f;

    if (toolMode != ToolMode::GEOMETRY)
        return;

    if (geometryMode != GeometryMode::PLANE)
        return;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    if (draggingNewPointLight)
        return;

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
    Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

    if (!viewportRect.Contains(Engine::GetMousePosition()))
    {
        return;
    }

    if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
    {
        if (!draggingNewPlane)
        {
            // Find intersect with horizontal plane as a default box start point
            RayCastHit finalHit = collisions.RayCast(mouseRay, Plane{ Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f) });

            // Test against existing level geometry
            SceneRayCastHit levelHit = scene.RayCast(mouseRay);

            if (levelHit.rayCastHit.hitDistance < finalHit.hitDistance) finalHit = levelHit.rayCastHit;

            if (finalHit.hit)
            {
                finalHit.hitPoint.x = Math::Round(finalHit.hitPoint.x, snap);
                finalHit.hitPoint.y = Math::Round(finalHit.hitPoint.y, snap);
                finalHit.hitPoint.z = Math::Round(finalHit.hitPoint.z, snap);

                planeStartPoint = finalHit.hitPoint;
                draggingNewPlane = true;
            }
        }
        else
        {
            RayCastHit hit = collisions.RayCast(mouseRay, Plane{ planeStartPoint, Vec3f(0.0f, 0.0f, 1.0f) });

            int DeltaMouseWheel = input.GetMouseState().GetDeltaMouseWheel();
            if (DeltaMouseWheel > 0)
            {
                subdivisions *= 2;
            }
            else if (DeltaMouseWheel < 0)
            {
                subdivisions /= 2;
                if (subdivisions < 1)
                {
                    subdivisions = 1;
                }
            }

            Vec3f originalHitPoint = hit.hitPoint;

            hit.hitPoint.x = Math::Round(hit.hitPoint.x, snap);
            hit.hitPoint.y = Math::Round(hit.hitPoint.y, snap);
            hit.hitPoint.z = Math::Round(hit.hitPoint.z, snap);

            float minX = std::min(hit.hitPoint.x, planeStartPoint.x);
            float minY = std::min(hit.hitPoint.y, planeStartPoint.y);

            float maxX = std::max(hit.hitPoint.x, planeStartPoint.x);
            float maxY = std::max(hit.hitPoint.y, planeStartPoint.y);

            planeMin = Vec3f(minX, minY, hit.hitPoint.z);
            planeMax = Vec3f(maxX, maxY, hit.hitPoint.z);

            Vec3f Green = Vec3f(0.1f, 1.0f, 0.3f);

            Vec3f SouthWest = planeMin;
            Vec3f NorthWest = Vec3f(minX, maxY, hit.hitPoint.z);
            Vec3f NorthEast = planeMax;
            Vec3f SouthEast = Vec3f(maxX, minY, hit.hitPoint.z);

            //graphics.DebugDrawLine(SouthWest, NorthWest, Green);
            //graphics.DebugDrawLine(NorthWest, NorthEast, Green);
            //graphics.DebugDrawLine(NorthEast, SouthEast, Green);
            //graphics.DebugDrawLine(SouthEast, SouthWest, Green);

            for (int i = 0; i < subdivisions + 1; ++i)
            {
                Vec3f HorizonalLeft = SouthWest + ((float)i * ((NorthWest - SouthWest) / (float)subdivisions));
                Vec3f HorizontalRight = SouthEast + ((float)i * ((NorthEast - SouthEast) / (float)subdivisions));

                Vec3f VerticalBottom = SouthWest + ((float)i * ((SouthEast - SouthWest) / (float)subdivisions));
                Vec3f VerticalTop = NorthWest + ((float)i * ((NorthEast - NorthWest) / (float)subdivisions));

                graphics.DebugDrawLine(HorizonalLeft, HorizontalRight, Green);
                graphics.DebugDrawLine(VerticalBottom, VerticalTop, Green);
            }

            //graphics.DebugDrawAABB(aabbBox, Vec3f(0.1f, 1.0f, 0.3f));
            graphics.DebugDrawLine(originalHitPoint, hit.hitPoint, Vec3f(1.0f, 0.5f, 0.5f));

        }
    }
    else
    {
        if (draggingNewPlane)
        {
            Model* newPlane = new Model(graphics.CreatePlaneModel(Vec2f(planeMin.x, planeMin.y), Vec2f(planeMax.x, planeMax.y), planeMin.z, subdivisions));
            scene.AddModel(*newPlane);
            draggingNewPlane = false;

            selectedTransformPtr = nullptr;
            selectedModelPtr = nullptr;
        }
    }
}

void UpdateModelPlace(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    if (draggingNewModel)
    {
        if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay);

            if (input.GetMouseState().GetDeltaMouseWheel() > 0)
            {
                Vec3f PrevScale = draggingModel->GetTransform().GetScale();
                PrevScale += Vec3f(0.1f, 0.1f, 0.1f);
                draggingModel->GetTransform().SetScale(PrevScale);
            }
            else if (input.GetMouseState().GetDeltaMouseWheel() < 0)
            {
                Vec3f PrevScale = draggingModel->GetTransform().GetScale();
                PrevScale -= Vec3f(0.1f, 0.1f, 0.1f);
                draggingModel->GetTransform().SetScale(PrevScale);
            }

            if (finalHit.rayCastHit.hit)
            {
                RayCastHit hit = finalHit.rayCastHit;

                Vec3f modelPos = hit.hitPoint;

                modelPos.x = Math::Round(modelPos.x, TransSnap);
                modelPos.y = Math::Round(modelPos.y, TransSnap);
                modelPos.z = Math::Round(modelPos.z, TransSnap);

                draggingModel->GetTransform().SetPosition(modelPos);
                Quaternion q = Math::VecDiffToQuat(hit.hitNormal, Vec3f(0.0f, 0.0f, 1.0f));
                q = Math::normalize(q);

                draggingModel->GetTransform().SetRotation(q);

            }
            else
            {
                Vec3f modelPos = mouseRay.point + mouseRay.direction * 12.0f;

                modelPos.x = Math::Round(modelPos.x, TransSnap);
                modelPos.y = Math::Round(modelPos.y, TransSnap);
                modelPos.z = Math::Round(modelPos.z, TransSnap);

                draggingModel->GetTransform().SetPosition(modelPos);

                draggingModel->GetTransform().SetRotation(Quaternion());
            }
        }
        else
        {
            // Actually add the model to the scene

            scene.AddModel(*draggingModel);

            delete draggingModel;

            draggingNewModel = false;
            draggingModel = nullptr;
        }
    }
}

void UpdateModelTranslate(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool slidingX = false;
    static bool slidingY = false;
    static bool slidingZ = false;

    static Vec3f slidingStartHitPoint;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    if (draggingNewPointLight)
        return;

    if (moveMode != MoveMode::TRANSLATE || toolMode != ToolMode::MOVE)
    {
        slidingX = false;
        slidingY = false;
        slidingZ = false;

        return;
    }

    if (draggingNewModel || draggingNewTexture || draggingNewBehaviour)
    {
        slidingX = false;
        slidingY = false;
        slidingZ = false;

        return;
    }

    if (slidingX || slidingY || slidingZ)
    {
        if (!input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            slidingX = false;
            slidingY = false;
            slidingZ = false;

            return;
        }

        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        if (slidingX)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedTransformPtr->GetPosition(), Vec3f(1.0f, 0.0f, 0.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            pointAlongAxis.x = Math::Round(pointAlongAxis.x, TransSnap / 2.0f);

            selectedTransformPtr->SetPosition(pointAlongAxis);
        }

        if (slidingY)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedTransformPtr->GetPosition(), Vec3f(0.0f, 1.0f, 0.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            pointAlongAxis.y = Math::Round(pointAlongAxis.y, TransSnap / 2.0f);

            selectedTransformPtr->SetPosition(pointAlongAxis);
        }

        if (slidingZ)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedTransformPtr->GetPosition(), Vec3f(0.0f, 0.0f, 1.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            pointAlongAxis.z = Math::Round(pointAlongAxis.z, TransSnap / 2.0f);

            selectedTransformPtr->SetPosition(pointAlongAxis);
        }


        return;
    }

    if (selectedTransformPtr)
    {
        if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            CollisionMesh& arrowToolCollMesh = *collisions.GetCollisionMeshFromMesh(xAxisArrow.m_TexturedMeshes[0].m_Mesh);

            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);
            
            RayCastHit hit;

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, xAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - selectedTransformPtr->GetPosition();
                slidingX = true;
            }

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, yAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - selectedTransformPtr->GetPosition();
                slidingY = true;
            }

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, zAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - selectedTransformPtr->GetPosition();
                slidingZ = true;
            }

            if (slidingX || slidingY || slidingZ)
            {
                return;
            }
        }
    }
    
    if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay);

        if (finalHit.rayCastHit.hit)
        {
            selectedTransformPtr = &finalHit.hitModel->GetTransform();
            selectedModelPtr = finalHit.hitModel;
        }
    }
}

void UpdateModelRotate(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool rotatingX = false;
    static bool rotatingY = false;
    static bool rotatingZ = false;

    static float initialAngle;
    static Quaternion initialRotation;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    if (draggingNewPointLight)
        return;

    if (moveMode != MoveMode::ROTATE || toolMode != ToolMode::MOVE)
    {
        rotatingX = false;
        rotatingY = false;
        rotatingZ = false;

        return;
    }

    if (draggingNewModel || draggingNewTexture || draggingNewBehaviour)
    {
        rotatingX = false;
        rotatingY = false;
        rotatingZ = false;

        return;
    }

    if (rotatingX || rotatingY || rotatingZ)
    {
        if (!input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            rotatingX = false;
            rotatingY = false;
            rotatingZ = false;

            return;
        }
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        Plane axisPlane;
        Vec3f perpUnitVec;

        axisPlane.center = selectedTransformPtr->GetPosition();

        if (rotatingX)
        {
            axisPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
            perpUnitVec = Vec3f(0.0f, 0.0f, 1.0f);
        }
        else if (rotatingY)
        {
            axisPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);
            perpUnitVec = Vec3f(1.0f, 0.0f, 0.0f);
        }
        else if (rotatingZ)
        {
            axisPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);
            perpUnitVec = Vec3f(0.0f, 1.0f, 0.0f);
        }

        RayCastHit planeHit = collisions.RayCast(mouseRay, axisPlane);

        Vec3f objectCenter = selectedTransformPtr->GetPosition();
        Vec3f hitPoint = planeHit.hitPoint;
        Vec3f angleVec = hitPoint - objectCenter;
        float dot = Math::dot(angleVec, perpUnitVec);
        float det = Math::dot(axisPlane.normal, (Math::cross(angleVec, perpUnitVec)));

        float deltaAngle = initialAngle - atan2(det, dot);

        deltaAngle = Math::Round(deltaAngle, RotSnap);

        Quaternion axisQuat = Quaternion(axisPlane.normal, deltaAngle);

        selectedTransformPtr->SetRotation(axisQuat * initialRotation);
    }
    else if (selectedTransformPtr)
    {
        if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            CollisionMesh& arrowToolCollMesh = *collisions.GetCollisionMeshFromMesh(xAxisRing.m_TexturedMeshes[0].m_Mesh);

            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            RayCastHit xHit = collisions.RayCast(mouseRay, arrowToolCollMesh, xAxisRing.GetTransform());
            RayCastHit yHit = collisions.RayCast(mouseRay, arrowToolCollMesh, yAxisRing.GetTransform());
            RayCastHit zHit = collisions.RayCast(mouseRay, arrowToolCollMesh, zAxisRing.GetTransform());

            const RayCastHit* closest = CollisionModule::Closest({ xHit, yHit, zHit });

            if (closest->hit)
            {
                if (*closest == xHit) { rotatingX = true; }
                else if (*closest == yHit) { rotatingY = true; }
                else if (*closest == zHit) { rotatingZ = true; }

                Plane axisPlane;
                Vec3f perpUnitVec;

                axisPlane.center = selectedTransformPtr->GetPosition();

                if (rotatingX)
                {
                    axisPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
                    perpUnitVec = Vec3f(0.0f, 0.0f, 1.0f);  
                }
                else if (rotatingY)
                {
                    axisPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);
                    perpUnitVec = Vec3f(1.0f, 0.0f, 0.0f);
                }
                else if (rotatingZ)
                {
                    axisPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);
                    perpUnitVec = Vec3f(0.0f, 1.0f, 0.0f);
                }

                RayCastHit planeHit = collisions.RayCast(mouseRay, axisPlane);

                initialRotation = selectedTransformPtr->GetRotation();

                Vec3f objectCenter = selectedTransformPtr->GetPosition();
                Vec3f hitPoint = planeHit.hitPoint;
                Vec3f angleVec = hitPoint - objectCenter;
                float dot = Math::dot(angleVec, perpUnitVec);
                float det = Math::dot(axisPlane.normal, (Math::cross(angleVec, perpUnitVec)));
                initialAngle = atan2(det, dot);
            }
        }
    }
    if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB) && (!rotatingX && !rotatingY && !rotatingZ))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay);

        if (finalHit.rayCastHit.hit)
        {
            selectedTransformPtr = &finalHit.hitModel->GetTransform();
            selectedModelPtr = finalHit.hitModel;
        }
    }
}

void UpdateModelScale(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool scalingX = false;
    static bool scalingY = false;
    static bool scalingZ = false;

    static Vec3f scalingStartHitPoint;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    if (draggingNewPointLight)
        return;

    if (moveMode != MoveMode::SCALE || toolMode != ToolMode::MOVE)
    {
        scalingX = false;
        scalingY = false;
        scalingZ = false;

        return;
    }

    if (draggingNewModel || draggingNewTexture || draggingNewBehaviour)
    {
        scalingX = false;
        scalingY = false;
        scalingZ = false;

        return;
    }

    if (scalingX || scalingY || scalingZ)
    {
        if (!input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            scalingX = false;
            scalingY = false;
            scalingZ = false;

            return;
        }

        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        Quaternion scalingModelRot = selectedTransformPtr->GetRotation();

        if (scalingX)
        {
            Line mouseLine(mouseRay.point - scalingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedTransformPtr->GetPosition(), Vec3f(1.0f, 0.0f, 0.0f) * scalingModelRot);
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            Vec3f prevScale = selectedTransformPtr->GetScale();

            float newScale = Math::magnitude(selectedTransformPtr->GetPosition() - pointAlongAxis);

            selectedTransformPtr->SetScale(Vec3f(newScale, prevScale.y, prevScale.z));
        }
        if (scalingY)
        {
            Line mouseLine(mouseRay.point - scalingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedTransformPtr->GetPosition(), Vec3f(0.0f, 1.0f, 0.0f) * scalingModelRot);
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            Vec3f prevScale = selectedTransformPtr->GetScale();

            float newScale = Math::magnitude(selectedTransformPtr->GetPosition() - pointAlongAxis);

            selectedTransformPtr->SetScale(Vec3f(prevScale.x, newScale, prevScale.z));
        }
        if (scalingZ)
        {
            Line mouseLine(mouseRay.point - scalingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedTransformPtr->GetPosition(), Vec3f(0.0f, 0.0f, 1.0f) * scalingModelRot);
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            Vec3f prevScale = selectedTransformPtr->GetScale();

            float newScale = Math::magnitude(selectedTransformPtr->GetPosition() - pointAlongAxis);

            selectedTransformPtr->SetScale(Vec3f(prevScale.x, prevScale.y, newScale));
        }
    }
    else if (selectedTransformPtr)
    {
        if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            CollisionMesh& scaleToolCollMesh = *collisions.GetCollisionMeshFromMesh(xScaleWidget.m_TexturedMeshes[0].m_Mesh);
            
            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            RayCastHit xHit = collisions.RayCast(mouseRay, scaleToolCollMesh, xScaleWidget.GetTransform());
            RayCastHit yHit = collisions.RayCast(mouseRay, scaleToolCollMesh, yScaleWidget.GetTransform());
            RayCastHit zHit = collisions.RayCast(mouseRay, scaleToolCollMesh, zScaleWidget.GetTransform());

            const RayCastHit* closest = CollisionModule::Closest({ xHit, yHit, zHit });

            if (closest->hit)
            {
                Vec3f widgetPoint;
                if (*closest == xHit) { scalingX = true; widgetPoint = xScaleWidget.GetTransform().GetPosition(); }
                else if (*closest == yHit) { scalingY = true; widgetPoint = yScaleWidget.GetTransform().GetPosition(); }
                else if (*closest == zHit) { scalingZ = true; widgetPoint = zScaleWidget.GetTransform().GetPosition(); }

                Vec3f hitPoint = closest->hitPoint;

                scalingStartHitPoint = hitPoint - widgetPoint;
            }
        }
    }

    if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB) && (!scalingX && !scalingY && !scalingZ))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay);

        if (finalHit.rayCastHit.hit)
        {
            selectedTransformPtr = &finalHit.hitModel->GetTransform();
            selectedModelPtr = finalHit.hitModel;
        }
    }
}

void UpdateVertexEdit(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{

}

void UpdateSculptTool(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics, double deltaTime)
{
    static const float SculptSpeed = 3.0f;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    if (draggingNewPointLight)
        return;

    if (toolMode != ToolMode::SCULPT)
    {
        return;
    }

    static float radius = 1.0f;

    if (input.GetMouseState().GetDeltaMouseWheel() > 0)
    {
        radius += 0.1f;
    }
    else if (input.GetMouseState().GetDeltaMouseWheel() < 0)
    {
        radius -= 0.1f;
    }

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
    Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

    SceneRayCastHit finalHit = scene.RayCast(mouseRay);

    if (finalHit.rayCastHit.hit)
    {
        Vec3f HitPoint = finalHit.rayCastHit.hitPoint;

        graphics.DebugDrawSphere(HitPoint, radius, Vec3f(0.6f, 0.3f, 0.9f));

        if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB) || input.GetMouseState().GetMouseButtonState(MouseButton::RMB))
        {
            if (finalHit.hitModel->Type == ModelType::PLANE)
            {
                Model* PlaneModel = finalHit.hitModel;
                Vec3f ModelSpaceVertPos = HitPoint * Math::inv(PlaneModel->GetTransform().GetTransformMatrix());

                float VerticalDir = input.GetMouseState().GetMouseButtonState(MouseButton::RMB) ? -SculptSpeed : SculptSpeed;

                StaticMesh_ID Mesh =  PlaneModel->m_TexturedMeshes[0].m_Mesh.Id;

                std::vector<Vertex*> Vertices = graphics.m_Renderer.MapMeshVertices(Mesh);

                for (auto& Vert : Vertices)
                {
                    float Dist = Math::magnitude(Vert->position - ModelSpaceVertPos);
                    //if (Dist < radius)
                    //{
                        float Strength = Math::SmoothStep(Dist, radius, 0.5f) * VerticalDir * (radius * 0.25f);
                        Vert->position += Vec3f(0.0f, 0.0f, Strength) * (float)deltaTime;
                    //}
                }

                graphics.m_Renderer.UnmapMeshVertices(Mesh);
                collisions.InvalidateMeshCollisionData(Mesh);
                graphics.RecalculateTerrainModelNormals(*PlaneModel);
            }
        }
        else if (input.GetMouseState().GetMouseButtonState(MouseButton::MIDDLE))
        {
            if (finalHit.hitModel->Type == ModelType::PLANE)
            {
                Model* PlaneModel = finalHit.hitModel;
                Vec3f ModelSpaceVertPos = HitPoint * Math::inv(PlaneModel->GetTransform().GetTransformMatrix());

                StaticMesh_ID Mesh = PlaneModel->m_TexturedMeshes[0].m_Mesh.Id;

                std::vector<Vertex*> Vertices = graphics.m_Renderer.MapMeshVertices(Mesh);

                std::vector<Vertex*> VerticesInRange;
                float AverageElevation = 0.0f;

                for (auto& Vert : Vertices)
                {
                    float Dist = Math::magnitude(Vert->position - ModelSpaceVertPos);
                    if (Dist < radius)
                    {
                        VerticesInRange.push_back(Vert);
                        AverageElevation += Vert->position.z;
                    }
                }
                AverageElevation /= VerticesInRange.size();

                for (auto& InRangeVert : VerticesInRange)
                {
                    float Dist = Math::magnitude(InRangeVert->position - ModelSpaceVertPos);
                    float Strength = Math::SmoothStep(Dist, radius, 0.0f);

                    float Diff = AverageElevation - InRangeVert->position.z;

                    InRangeVert->position.z += SculptSpeed * Diff * Strength * (float)deltaTime;
                }

                graphics.m_Renderer.UnmapMeshVertices(Mesh);
                collisions.InvalidateMeshCollisionData(Mesh);
                graphics.RecalculateTerrainModelNormals(*PlaneModel);
            }
        }
    }
}

void UpdateTexturePlace(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    if (draggingNewTexture)
    {
        if (!input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            draggingNewTexture = false;

            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay);

            if (finalHit.rayCastHit.hit)
            {
                finalHit.hitModel->SetMaterial(draggingMaterial);
            }
        }
    }

}

void UpdateBehaviourPlace(InputModule& input, CollisionModule& collisions)
{
    if (draggingNewBehaviour)
    {
        if (!input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            draggingNewBehaviour = false;
            
            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay);

            if (finalHit.rayCastHit.hit)
            {

                BehaviourRegistry::Get()->AttachNewBehaviour(draggingBehaviourName, finalHit.hitModel);
            }
        }
    }
}

void UpdatePointLightPlace(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static Vec3f LightPos;
    if (draggingNewPointLight)
    {
        if (input.GetMouseState().GetMouseButtonState(MouseButton::LMB))
        {
            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay);

            

            if (finalHit.rayCastHit.hit)
            {
                static float NormalOffset = TransSnap;

                if (input.GetMouseState().GetDeltaMouseWheel() > 0)
                {
                    NormalOffset += TransSnap;
                }
                else if (input.GetMouseState().GetDeltaMouseWheel() < 0)
                {
                    NormalOffset -= TransSnap;
                    if (NormalOffset < TransSnap)
                    {
                        NormalOffset = TransSnap;
                    }
                }

                RayCastHit hit = finalHit.rayCastHit;

                LightPos = hit.hitPoint + (finalHit.rayCastHit.hitNormal * NormalOffset);

                LightPos.x = Math::Round(LightPos.x, TransSnap);
                LightPos.y = Math::Round(LightPos.y, TransSnap);
                LightPos.z = Math::Round(LightPos.z, TransSnap);

            }
            else
            {
                LightPos = mouseRay.point + mouseRay.direction * 12.0f;

                LightPos.x = Math::Round(LightPos.x, TransSnap);
                LightPos.y = Math::Round(LightPos.y, TransSnap);
                LightPos.z = Math::Round(LightPos.z, TransSnap);
            }

            PointLightRenderCommand RenderCommand;
            RenderCommand.m_Colour = Vec3f(1.0f, 1.0f, 1.0f);
            RenderCommand.m_Position = LightPos;

            BillboardRenderCommand BBRenderCommand;
            BBRenderCommand.m_Texture = graphics.GetLightTexture();
            BBRenderCommand.m_Position = LightPos;

            graphics.AddRenderCommand(RenderCommand);
            graphics.AddRenderCommand(BBRenderCommand);

        }
        else
        {
            // Place the light in the scene
            draggingNewPointLight = false;

            PointLight NewLight;
            NewLight.position = LightPos;
            scene.AddPointLight(NewLight);
        }
    }
}

void UpdateEditor(double deltaTime)
{
    GraphicsModule& graphics = *GraphicsModule::Get();
    CollisionModule& collisions = *CollisionModule::Get();
    TextModule& text = *TextModule::Get();
    UIModule& ui = *UIModule::Get();
    InputModule& input = *InputModule::Get();

    if (cursorLocked)
    {
        MoveCamera(input, graphics, cam, 0.001f, deltaTime);
    }

    Vec2i mousePos = Engine::GetMousePosition();

    Rect newViewport = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    if (Engine::IsWindowFocused())
    {
        UpdateSelectTool(input, collisions);
        UpdateBoxCreate(input, collisions, graphics);
        UpdatePlaneCreate(input, collisions, graphics);
        UpdateModelPlace(input, collisions, graphics);
        UpdateModelTranslate(input, collisions, graphics);
        UpdateModelRotate(input, collisions, graphics);
        UpdateModelScale(input, collisions, graphics);
        UpdateVertexEdit(input, collisions, graphics);
        UpdateSculptTool(input, collisions, graphics, deltaTime);
        UpdateTexturePlace(input, collisions, graphics);
        UpdateBehaviourPlace(input, collisions);
        UpdatePointLightPlace(input, collisions, graphics);
    }

    if (input.IsKeyDown(Key::Alt))
    {
        if (!holdingAlt)
        {
            if (cursorLocked)
            {
                Engine::UnlockCursor();
                Engine::ShowCursor();
                cursorLocked = false;
            }
            else {
                Engine::LockCursor();
                Engine::HideCursor();
                cursorLocked = true;
            }
            holdingAlt = true;
        }
    }
    else {
        holdingAlt = false;
    }

    // BEGIN DRAW

    //DrawEditorGrid(graphics);

    // Draw all my little meshies (for the models tab)
    graphics.SetCamera(&modelCam);
    for (int i = 0; i < modelFBuffers.size(); ++i)
    {
        loadedModels[i].GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.005f));
        graphics.SetActiveFrameBuffer(modelFBuffers[i]);
        graphics.Draw(loadedModels[i]);
    }

    graphics.SetCamera(&cam);
    graphics.SetActiveFrameBuffer(viewportBuffer);
    {
        scene.SetCamera(&cam);

        if (selectedTransformPtr)
        {
            CollisionMesh& ColMesh = *collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh);

            OctreeNode* CurNode = ColMesh.OctreeHead;

            //DrawOctreeNode(CurNode, selectedModelPtr->GetTransform(), &graphics);

            //AABB aabb = collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox;
            //graphics.DebugDrawAABB(aabb, Vec3f(0.6f, 0.95f, 0.65f), selectedModelPtr->GetTransform().GetTransformMatrix());
            //graphics.DebugDrawModelMesh(*selectedModelPtr, Vec3f(0.9f, 0.8f, 0.4f));
        }

        if (draggingModel)
        {
            StaticMeshRenderCommand draggingModelRC;
            draggingModelRC.m_Material = draggingModel->m_TexturedMeshes[0].m_Material;
            draggingModelRC.m_Mesh = draggingModel->m_TexturedMeshes[0].m_Mesh.Id;
            draggingModelRC.m_Transform = draggingModel->GetTransform();
            graphics.AddRenderCommand(draggingModelRC);
            //graphics.Draw(*draggingModel);
        }
        
        scene.Draw(graphics, gBuffer);

        NetworkModule& Network = *NetworkModule::Get();

        Vec3f OtherPlayerPosition;

        static bool GotOtherPlayerPos = false;

        if (m_IsClient)
        {
            Packet Data;
            while (Network.ClientPollData(Data))
            {
                std::vector<std::string> Tokens = StringUtils::Split(Data.Data, " ");
                if (Tokens.size() >= 3)
                {
                    OtherPlayerPosition.x = std::stof(Tokens[0]);
                    OtherPlayerPosition.y = std::stof(Tokens[1]);
                    OtherPlayerPosition.z = std::stof(Tokens[2]);

                    OtherPlayerModel.GetTransform().SetPosition(OtherPlayerPosition);

                    GotOtherPlayerPos = true;
                }
            }

            Packet OutData;
            Vec3f MyPos = cam.GetPosition();

            OutData.Data = std::to_string(MyPos.x) + " " + std::to_string(MyPos.y) + " " + std::to_string(MyPos.z) + " ";

            Network.ClientSendData(OutData.Data);
        }

        if (m_IsServer)
        {
            Packet Data;
            while (Network.ServerPollData(Data))
            {
                std::vector<std::string> Tokens = StringUtils::Split(Data.Data, " ");
                if (Tokens.size() >= 3)
                {
                    OtherPlayerPosition.x = std::stof(Tokens[0]);
                    OtherPlayerPosition.y = std::stof(Tokens[1]);
                    OtherPlayerPosition.z = std::stof(Tokens[2]);

                    OtherPlayerModel.GetTransform().SetPosition(OtherPlayerPosition);

                    GotOtherPlayerPos = true;
                }
            }

            Packet OutData;
            Vec3f MyPos = cam.GetPosition();

            OutData.Data = std::to_string(MyPos.x) + " " + std::to_string(MyPos.y) + " " + std::to_string(MyPos.z) + " ";

            Network.ServerSendData(OutData.Data);
        }

        if (GotOtherPlayerPos && (m_IsClient || m_IsServer))
        {
            graphics.Draw(OtherPlayerModel);
        }
    }
    graphics.SetRenderMode(RenderMode::FULLBRIGHT);
    if (gridEnabled)
    {
        //graphics.Draw(gridModel);
    }
    graphics.SetRenderMode(renderMode);

    graphics.ResetFrameBuffer();
    
    DrawToolWidgets();

    Vec2i screen = Engine::GetClientAreaSize();

    //ui.BufferPanel(viewportBuffer, newViewport);



    ui.BufferPanel(gBuffer.FinalOutput, newViewport);
    ui.BufferPanel(widgetViewportBuffer, newViewport);
    
    //ui.ImgPanel(gBuffer.PositionTex, Rect(Vec2f(100.0f, 40.0f), Vec2f(200.0f, 200.0f)));
    //ui.ImgPanel(gBuffer.NormalTex, Rect(Vec2f(300.0f, 40.0f), Vec2f(200.0f, 200.0f)));
    //ui.ImgPanel(gBuffer.AlbedoTex, Rect(Vec2f(500.0f, 40.0f), Vec2f(200.0f, 200.0f)));
    //ui.ImgPanel(gBuffer.MetallicTex, Rect(Vec2f(700.0f, 40.0f), Vec2f(200.0f, 200.0f)));
    //ui.ImgPanel(gBuffer.RoughnessTex, Rect(Vec2f(900.0f, 40.0f), Vec2f(200.0f, 200.0f)));
    //ui.ImgPanel(gBuffer.AOTex, Rect(Vec2f(1100.0f, 40.0f), Vec2f(200.0f, 200.0f)));
    //
    //ui.ImgPanel(gBuffer.SkyTex, Rect(Vec2f(100.0f, 240.0f), Vec2f(200.0f, 200.0f)));
    //ui.ImgPanel(gBuffer.LightTex, Rect(Vec2f(300.0f, 240.0f), Vec2f(200.0f, 200.0f)));


    ui.StartFrame("Memes", Rect(Vec2f(0.0f, 0.0f), Vec2f(screen.x - 200.0f, 40.0f)), 0.0f);

    {
        if (ui.ImgButton("PlayButton", playButtonTexture, Vec2f(40.0f, 40.0f), 4.0f))
        {
            state = State::GAME;
            player.velocity = Vec3f(0.0f, 0.0f, 0.0f);
            player.position = cam.GetPosition();
            runtimeScene = Scene(scene);
            
            Resize(Engine::GetClientAreaSize());
        }

        if (ui.ImgButton("CameraButton", cameraButtonTexture, Vec2f(40.0f, 40.0f), 4.0f))
        {
            
        }
        if (ui.TextButton("New", Vec2f(40.0f, 40.0f), 4.0f))
        {
            scene.Clear();
        }
        if (ui.TextButton("Open", Vec2f(40.0f, 40.0f), 4.0f))
        {
            std::string FileName;
            if (Engine::FileOpenDialog(FileName))
            {
                Engine::DEBUGPrint(FileName);
                scene.Load(FileName);
            }
        }
        if (ui.TextButton("Save", Vec2f(40.0f, 40.0f), 4.0f))
        {
            std::string FileName;
            if (Engine::FileSaveDialog(FileName))
            {
                Engine::DEBUGPrint(FileName);
                scene.Save(FileName);
            }
        }

        if (ui.TextButton("Grid", Vec2f(40.0f, 40.0f), 4.0f))
        {
            gridEnabled = !gridEnabled;
        }

        NetworkModule& Network = *NetworkModule::Get();

        if (ui.TextButton("Host", Vec2f(40, 40), 4.0f) && !m_IsClient)
        {
            Network.StartServer();
            m_IsServer = true;
        }

        ui.TextEntry("IpEntry", ipString, Vec2f(200.0f, 40.0f));

        if (ui.TextButton("Join", Vec2f(40, 40), 4.0f) && !m_IsServer)
        {
            Network.StartClient(ipString);
            m_IsClient = true;
        }

        if (ui.TextButton("Ping", Vec2f(40, 40), 4.0f))
        {
            if (m_IsServer)
            {
                Network.ServerPing();
            }
            if (m_IsClient)
            {
                Network.ClientPing();
            }
        }

        if (ui.TextButton("Disconnect", Vec2f(80, 40), 4.0f))
        {
            Network.DisconnectAll();
        }
    }

    ui.EndFrame();
    
    ui.StartFrame("Resources", Rect(Vec2f(100.0f, screen.y - 200.0f), Vec2f(screen.x - 300.0f, 200.0f)), 20.0f);

    ui.StartTab("Models");
    for (int i = 0; i < loadedModels.size(); ++i)
    {
        
        if (ui.BufferButton(loadedModels[i].m_TexturedMeshes[0].m_Mesh.Path.GetFileName(), modelFBuffers[i], Vec2f(100, 100), 10, Black).clicking)
        {
            if (!draggingNewModel)
            {
                draggingNewModel = true;
           
                selectedTransformPtr = nullptr;

                draggingModel = new Model(graphics.CloneModel(loadedModels[i]));
            }
        }
    }
    ui.EndTab();

    ui.StartTab("Textures");
    for (int i = 0; i < loadedMaterials.size(); ++i)
    {
        if (ui.ImgButton(loadedMaterials[i].m_Albedo.Path.GetFileName(), loadedMaterials[i].m_Albedo, Vec2f(40, 80), 2.5f, Black).clicking)
        {
            if (!draggingNewTexture)
            {
                draggingNewTexture = true;

                selectedTransformPtr = nullptr;

                draggingMaterial = loadedMaterials[i];
            }
        }
    }
    ui.EndTab();

    ui.StartTab("Behaviours");

    auto BehaviourMap = BehaviourRegistry::Get()->GetBehaviours();

    int i = 0;
    for (auto Behaviour : BehaviourMap)
    {
        if (ui.TextButton(Behaviour.first, Vec2f(80, 80), 2.0f).clicking)
        {
            if (!draggingNewBehaviour)
            {
                draggingNewBehaviour = true;

                selectedTransformPtr = nullptr;
                selectedModelPtr = nullptr;

                draggingBehaviourName = Behaviour.first;
            }
        }
        i++;                
    }

    ui.EndTab();

    ui.EndFrame();

    ui.StartFrame("Inspector", Rect(Vec2f(screen.x - 200.0f, 0.0f), Vec2f(200.0f, screen.y / 2.0f)), 20.0f);

    std::string posText;

    if (selectedTransformPtr)
    {
        Vec2f Cursor = Vec2f(0.0f, 5.0f);

        Vec3f Position = selectedTransformPtr->GetPosition();
        Vec3f Scale = selectedTransformPtr->GetScale();

        Vec3f NewPos;

        ui.Text("Position", Cursor, Vec3f(0.0f, 0.0f, 0.0f));
        Cursor.y += 15.0f;

        std::string xString = std::to_string(Position.x);
        xString.erase(xString.find_last_not_of('0') + 1, std::string::npos);
        //xString.erase(xString.find_last_not_of('.') + 1, std::string::npos);
        ui.TextEntry("X", xString, Vec2f(160.0f, 15.0f));
        NewPos.x = std::stof(xString);
        Cursor.y += 15.0f;
        
        std::string yString = std::to_string(Position.y);
        ui.TextEntry("Y", yString, Vec2f(160.0f, 15.0f));
        NewPos.y = std::stof(yString);
        Cursor.y += 15.0f;

        std::string zString = std::to_string(Position.z);
        ui.TextEntry("Z", zString, Vec2f(160.0f, 15.0f));
        NewPos.z = std::stof(zString);
        Cursor.y += 25.0f;

        selectedTransformPtr->SetPosition(NewPos);

        ui.Text("Tags", Cursor, Vec3f(0.0f, 0.0f, 0.0f));
        Cursor.y += 15.0f;

        //ui.TextEntry(selectedModelPtr->m_Name, Rect(Cursor, Vec2f(160.0f, 20.0f)));
        Cursor.y += 25.0f;

        //std::vector<std::string> BehavioursOnModel = BehaviourRegistry::Get()->GetBehavioursAttachedToEntity(selectedModelPtr);

        //ui.Text("Behaviours:", Cursor, Vec3f(0.0f, 0.0f, 0.0f));
        //Cursor.y += 15.0f;
        //for (auto& Behaviour : BehavioursOnModel)
        //{
        //    ui.TextButton(Behaviour, Rect(Cursor, Vec2f(160.0f, 20.0f)), 4.0f);
        //    Cursor.y += 20.0f;
        //}
    }
    else
    {
        posText = "No model selected.";
        text.DrawText(posText, &inspectorFont, Vec2f(screen.x - 180.0f, 20.0f), Vec3f(0.0f, 0.0f, 0.0f));
    }

    ui.EndFrame();

    ui.StartFrame("Entities", Rect(Vec2f(screen.x - 200.0f, screen.y / 2.0f), Vec2f(200.0f, screen.y / 2.0f)), 20.0f);

    if (Model* modelPtr = scene.MenuListEntities(ui, inspectorFont))
    {
        //selectedModelPtr = modelPtr;
    }

    ui.EndFrame();

    if (draggingNewTexture)
    {
        ui.ImgPanel(draggingMaterial.m_Albedo, Rect(Engine::GetMousePosition(), Vec2f(80.0f, 80.0f)));
    }

    if (draggingNewBehaviour)
    {
        ui.Text(draggingBehaviourName, Engine::GetMousePosition(), Vec3f(1.0f, 1.0f, 1.0f));
    }

    ui.StartFrame("Tools", Rect(Vec2f(0.0f, 40.0f), Vec2f(100.0f, screen.y - 40.0f)), 0.0f);
    {
        if (ui.ImgButton("SelectTool", cursorToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
        {
            toolMode = ToolMode::SELECT;
        }

        if (geometryMode == GeometryMode::BOX)
        {
            if (ui.ImgButton("BoxTool", boxToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
            {
                if (toolMode == ToolMode::GEOMETRY)
                {
                    CycleGeometryMode();
                }
                else
                {
                    toolMode = ToolMode::GEOMETRY;
                }
            }
        }
        else if (geometryMode == GeometryMode::PLANE)
        {
            if (ui.ImgButton("GeometryMode", planeToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
            {
                if (toolMode == ToolMode::GEOMETRY)
                {
                    CycleGeometryMode();
                }
                else
                {
                    toolMode = ToolMode::GEOMETRY;
                }
            }
        }


        if (moveMode == MoveMode::TRANSLATE)
        {
            if (ui.ImgButton("TranslateTool", translateToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
            {
                if (toolMode == ToolMode::MOVE)
                {
                    CycleMoveMode();
                }
                else
                {
                    toolMode = ToolMode::MOVE;
                }
            }
        }
        else if (moveMode == MoveMode::ROTATE)
        {
            if (ui.ImgButton("RotateTool", rotateToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
            {
                if (toolMode == ToolMode::MOVE)
                {
                    CycleMoveMode();
                }
                else
                {
                    toolMode = ToolMode::MOVE;
                }
            }
        }
        else if (moveMode == MoveMode::SCALE)
        {
            if (ui.ImgButton("ScaleTool", scaleToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
            {
                if (toolMode == ToolMode::MOVE)
                {
                    CycleMoveMode();
                }
                else
                {
                    toolMode = ToolMode::MOVE;
                }
            }
        }

        if (ui.ImgButton("VertexTool", vertexToolTexture, Vec2f(100, 100.0f), 20.0f))
        {
            toolMode = ToolMode::VERTEX;
        }

        if (ui.ImgButton("CameraTool", cameraButtonTexture, Vec2f(100.0f, 100.0f), 20.0f))
        {
            if (renderMode == RenderMode::DEFAULT)
            {
                renderMode = RenderMode::FULLBRIGHT;
            }
            else
            {
                renderMode = RenderMode::DEFAULT;
            }
            graphics.SetRenderMode(renderMode);
        }

        if (ui.ImgButton("SculptTool", sculptToolTexture, Vec2f(100.0f, 100.0f), 20.0f))
        {
            toolMode = ToolMode::SCULPT;
        }

        if (ui.ImgButton("LightTool", lightToolTexture, Vec2f(100.0f, 100.0f), 20.0f).clicking)
        {
            if (!draggingNewPointLight)
            {
                draggingNewPointLight = true;

            }
        }
    }
    ui.EndFrame();

    if (input.IsKeyDown(Key::Escape))
    {
        selectedTransformPtr = nullptr;
        selectedModelPtr = nullptr;
    }

    if (input.GetKeyState(Key::Delete))
    {
        //if (selectedModelPtr) {
        //    scene.DeleteModel(selectedModelPtr);
        //    selectedModelPtr = nullptr;
        //}
    }

    if (input.IsKeyDown(Key::One))
    {
        toolMode = ToolMode::SELECT;
    }
    if (input.IsKeyDown(Key::Two))
    {
        if (toolMode == ToolMode::GEOMETRY)
        {
            if (input.GetKeyState(Key::Two).justPressed)
            {
                CycleGeometryMode();
            }
        }
        else
        {
            toolMode = ToolMode::GEOMETRY;
        }
    }
    if (input.IsKeyDown(Key::Three))
    {
        if (toolMode == ToolMode::MOVE)
        {
            if (input.GetKeyState(Key::Three).justPressed)
            {
                CycleMoveMode();
            }
        }
        else
        {
            toolMode = ToolMode::MOVE;
        }
    }
    if (input.IsKeyDown(Key::Four))
    {
        toolMode = ToolMode::VERTEX;
    }
    if (input.IsKeyDown(Key::Five))
    {
        toolMode = ToolMode::SCULPT;
    }

    std::string modeString;
    switch (toolMode)
    {
    case ToolMode::SELECT:
        modeString = "Select Mode";
        break;
    case ToolMode::GEOMETRY:
        switch (geometryMode)
        {
        case GeometryMode::BOX:
            modeString = "Box Mode";
            break;
        case GeometryMode::PLANE:
            modeString = "Plane Mode";
            break;
        }
        break;
    case ToolMode::MOVE:
        switch (moveMode)
        {
        case MoveMode::TRANSLATE:
            modeString = "Translate Mode";
            break;
        case MoveMode::ROTATE:
            modeString = "Rotate Mode";
            break;
        case MoveMode::SCALE:
            modeString = "Scale Mode";
            break;
        }
        break;
    case ToolMode::VERTEX:
        modeString = "Vertex Edit Mode";
        break;
    case ToolMode::SCULPT:
        modeString = "Terrain Sculpt Mode";
        break;
    }

    if (input.GetKeyState(Key::Space).justReleased)
    {
        //AudioModule* Audio = AudioModule::Get();
        //Audio->PlayAsyncSound("");
    }

    text.DrawText(modeString, &testFont, Vec2f(100.0f, 40.0f), Vec3f(0.0f, 0.0f, 0.0f));

    // END DRAW

}

void UpdateGame(double deltaTime)
{
    GraphicsModule& graphics = *GraphicsModule::Get();
    CollisionModule& collisions = *CollisionModule::Get();
    TextModule& text = *TextModule::Get();
    InputModule& input = *InputModule::Get();
    UIModule& ui = *UIModule::Get();

    if (input.IsKeyDown(Key::Escape))
    {
        // TODO(Fraser): This is where I might do something with the runtime scene (right now I'm doing nothing and recreating it whenever I enter game mode)

        state = State::EDITOR;
        Resize(Engine::GetClientAreaSize());
    }

    if (input.IsKeyDown(Key::Alt))
    {
        if (!holdingAlt)
        {
            if (cursorLocked)
            {
                Engine::UnlockCursor();
                Engine::ShowCursor();
                cursorLocked = false;
            }
            else {
                Engine::LockCursor();
                Engine::HideCursor();
                cursorLocked = true;
            }
            holdingAlt = true;
        }
    }
    else {
        holdingAlt = false;
    }

    // Update player

    if (FirstPersonPlayerEnabled)
    {
        if (cursorLocked)
        {
            MoveCamera(input, graphics, cam, 0.001f, deltaTime);
        }

        Vec3f inputDir = Vec3f();

        Vec3f leftVec = Math::normalize(cam.GetPerpVector());
        Vec3f forwardVec = Math::cross(leftVec, Vec3f(0.0f, 0.0f, 1.0f));

        bool movedLaterally = false;

        if (input.IsKeyDown(Key::A))
        {
            inputDir -= leftVec;
            movedLaterally = true;
        }
        if (input.IsKeyDown(Key::D))
        {
            inputDir += leftVec;
            movedLaterally = true;
        }
        if (input.IsKeyDown(Key::W))
        {
            inputDir -= forwardVec;
            movedLaterally = true;
        }
        if (input.IsKeyDown(Key::S))
        {
            inputDir += forwardVec;
            movedLaterally = true;
        }

        //if (input.IsKeyDown(Key::Space))
        //{
        //    inputDir += Vec3f(0.0f, 0.0f, 0.1f);
        //}

        //if (input.IsKeyDown(Key::Ctrl))
        //{
        //    inputDir += Vec3f(0.0f, 0.0f, -0.1f);
        //}

        if (movedLaterally)
        {
            inputDir = Math::normalize(inputDir) * 0.05f;
        }

        if (player.grounded)
        {
            SceneRayCastHit groundCheck = runtimeScene.RayCast(Ray(player.position, Vec3f(0.0f, 0.0f, -1.0f)));
            
            if (!groundCheck.rayCastHit.hit)
            {
                player.grounded = false;
            }
            else
            {
                if (groundCheck.rayCastHit.hitDistance > 0.02)
                {
                    player.grounded = false;
                }
            }
        }


        player.velocity.x = inputDir.x;
        player.velocity.y = inputDir.y;
        
        if (!player.grounded)
        {
            player.velocity.z += -0.005f;
        }

        //player.velocity += Vec3f(0.0f, 0.0f, -0.01f);

        //player.grounded = false;

        SceneRayCastHit movement = runtimeScene.RayCast(Ray(player.position, Math::normalize(player.velocity)));

        float hitDist = 0.0f;
        float testHitDist = 0.0f;
        float playerVel = 0.0f;

        if (movement.rayCastHit.hit)
        {
            hitDist = movement.rayCastHit.hitDistance;
            testHitDist = Math::magnitude(movement.rayCastHit.hitPoint - player.position);
            playerVel = Math::magnitude(player.velocity);
            if (hitDist <= playerVel + 0.0001f) // TODO: whyyy is this necessary
            {
                graphics.DebugDrawPoint(movement.rayCastHit.hitPoint, Vec3f(1.0f, 0.3f, 0.2f));
                player.position = movement.rayCastHit.hitPoint + (movement.rayCastHit.hitNormal * 0.01f);
                player.velocity.z = 0.0f;
                player.grounded = true;
            }
            else
            {
                graphics.DebugDrawPoint(movement.rayCastHit.hitPoint);
                player.position += player.velocity;
            }
        }
        else
        {
            player.position += player.velocity;
        }

        if (input.GetKeyState(Key::Space) && player.grounded)
        {
            player.velocity.z = 0.145f;
        }

        if (movement.rayCastHit.hit && movement.rayCastHit.hitDistance > Math::magnitude(player.velocity))
        {
            graphics.DebugDrawPoint(movement.rayCastHit.hitPoint);
        }

        player.cam->SetPosition(player.position + Vec3f(0.0f, 0.0f, 2.5f));

    }

    // Update behaviours
    runtimeScene.UpdateBehaviours((float)deltaTime);
    //BehaviourRegistry::Get()->UpdateAllBehaviours(modules, &runtimeScene, deltaTime);


    // Drop models

    static Model* droppingModel = nullptr;

    static Vec3f modelSpeed = Vec3f(0.0f, 0.0f, 0.0f);

    if (droppingModel)
    {
        modelSpeed.z += -0.005f;
        droppingModel->GetTransform().Move(modelSpeed);
    }

    graphics.SetActiveFrameBuffer(viewportBuffer);
    {
        runtimeScene.SetCamera(&cam);
        runtimeScene.Draw(graphics, gBuffer);

    }
    graphics.ResetFrameBuffer();



    Rect screenRect;
    screenRect.size = Engine::GetClientAreaSize();

    //ui.BufferPanel(viewportBuffer, screenRect);
    ui.BufferPanel(gBuffer.FinalOutput, screenRect);

    text.DrawText("Frame Time: " + std::to_string(deltaTime), &testFont, Vec2f(0.0f, 0.0f));

    PrevFrameTimeCount++;
    PrevFrameTimeSum += deltaTime;

    if (PrevFrameTimeSum > 0.5f)
    {
        PrevAveFPS = (int)round(1.0f / (PrevFrameTimeSum / PrevFrameTimeCount));
        PrevFrameTimeCount = 0;
        PrevFrameTimeSum -= 0.5f;
    }

    text.DrawText("FPS: " + std::to_string(PrevAveFPS), &testFont, Vec2f(0.0f, 30.0f));
    text.DrawText("Player grounded: " + std::to_string(player.grounded), &testFont, Vec2f(0.0f, 60.0f));
}

void Initialize()
{
    GraphicsModule& graphics = *GraphicsModule::Get();
    CollisionModule& collisions = *CollisionModule::Get();
    TextModule& text = *TextModule::Get();
    InputModule& input = *InputModule::Get();

    AssetRegistry* Registry = AssetRegistry::Get();

    Texture redTexture = *Registry->LoadTexture("textures/red.png");
    Texture greenTexture = *Registry->LoadTexture("textures/green.png");
    Texture blueTexture = *Registry->LoadTexture("textures/blue.png");

    //scene.Init(graphics, collisions);
    

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    xAxisArrow = graphics.CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ArrowSmooth.obj"),
        graphics.CreateMaterial(*Registry->LoadTexture("textures/whiteTexture.png"))
    ));
    
    yAxisArrow = graphics.CloneModel(xAxisArrow);
    zAxisArrow = graphics.CloneModel(xAxisArrow);

    xAxisArrow.GetTransform().SetRotation(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), (float)-M_PI_2));
    zAxisArrow.GetTransform().SetRotation(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), (float)M_PI_2));

    xAxisArrow.SetMaterial(graphics.CreateMaterial(redTexture));
    yAxisArrow.SetMaterial(graphics.CreateMaterial(greenTexture));
    zAxisArrow.SetMaterial(graphics.CreateMaterial(blueTexture));

    xAxisRing = graphics.CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/RotationHoop.obj"),
        graphics.CreateMaterial(redTexture)
    ));

    yAxisRing = graphics.CloneModel(xAxisRing);
    zAxisRing = graphics.CloneModel(xAxisRing);

    xAxisRing.GetTransform().SetRotation(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), (float)M_PI_2));
    zAxisRing.GetTransform().SetRotation(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), (float)M_PI_2));

    yAxisRing.SetMaterial(graphics.CreateMaterial(greenTexture));
    zAxisRing.SetMaterial(graphics.CreateMaterial(blueTexture));

    xScaleWidget = graphics.CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ScaleWidget.obj"),
        graphics.CreateMaterial(redTexture)
    ));

    yScaleWidget = graphics.CloneModel(xScaleWidget);
    zScaleWidget = graphics.CloneModel(xScaleWidget);

    yScaleWidget.SetMaterial(graphics.CreateMaterial(greenTexture));
    zScaleWidget.SetMaterial(graphics.CreateMaterial(blueTexture));

    playButtonTexture = *Registry->LoadTexture("images/playButton.png");
    cameraButtonTexture = *Registry->LoadTexture("images/cameraButton.png");

    cursorToolTexture = *Registry->LoadTexture("images/cursorTool.png");
    boxToolTexture = *Registry->LoadTexture("images/boxTool.png");
    planeToolTexture = *Registry->LoadTexture("images/planeTool.png");

    translateToolTexture = *Registry->LoadTexture("images/translateTool.png");
    rotateToolTexture = *Registry->LoadTexture("images/rotateTool.png");
    scaleToolTexture = *Registry->LoadTexture("images/scaleTool.png");

    vertexToolTexture = *Registry->LoadTexture("images/vertexTool.png");
    sculptToolTexture = *Registry->LoadTexture("images/sculptTool.png");
    lightToolTexture = *Registry->LoadTexture("images/lightTool.png");

    Vec2i screenSizeI = Engine::GetClientAreaSize();
    cam = Camera(Projection::Perspective);
    cam.SetScreenSize(screenSizeI);
    
    cam.SetPosition(Vec3f(0.0f, -8.0f, 6.0f));
    cam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), -0.7f));

    modelCam = Camera(Projection::Perspective);
    modelCam.SetScreenSize(Vec2f(100.0f, 100.0f));
    modelCam.SetPosition(Vec3f(0.0f, -2.5f, 2.5f));
    modelCam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), -0.7f));

    scene.SetDirectionalLight(DirectionalLight{ cam.GetDirection(), SunLight });

    viewportBuffer = graphics.CreateFBuffer(Vec2i(viewportRect.size), FBufferFormat::COLOUR);

    widgetViewportBuffer = graphics.CreateFBuffer(Vec2i(viewportRect.size), FBufferFormat::COLOUR);

    gBuffer = graphics.CreateGBuffer(Vec2i(viewportRect.size));

    graphics.InitializeDebugDraw(gBuffer.FinalOutput);

    graphics.SetCamera(&cam);
    
    graphics.SetRenderMode(renderMode);

    tempWhiteMaterial = graphics.CreateMaterial(*Registry->LoadTexture("images/white.png"));

    gridTexture = *Registry->LoadTexture("images/grid.png");
    gridModel = graphics.CreatePlaneModel(Vec2f(-4000.0f, -4000.0f), Vec2f(4000.0f, 4000.0f), graphics.CreateMaterial(gridTexture));

    testFont = text.LoadFont("fonts/ARLRDBD.TTF", 30);
    inspectorFont = text.LoadFont("fonts/ARLRDBD.TTF", 15);

    loadedModels = LoadModels(graphics);
    
    loadedMaterials = LoadMaterials(graphics);

    Vec2i clientArea = Engine::GetClientAreaSize();

    Vec2i newCenter = Vec2i((int)(viewportRect.location.x + viewportRect.size.x / 2.0f), (int)(viewportRect.location.y + viewportRect.size.y / 2.0f));
    //TODO(fraser): clean up mouse constrain/input code
    input.SetMouseCenter(newCenter);
    Engine::SetCursorCenter(newCenter);

    player.cam = &cam;
    
    auto Behaviours = BehaviourRegistry::Get()->GetBehaviours();

    OtherPlayerModel = graphics.CloneModel(xAxisArrow);
}

void Update(double deltaTime)
{
    if (state == State::EDITOR)
    {
        UpdateEditor(deltaTime);
    }
    else if (state == State::GAME)
    {
        UpdateGame(deltaTime);
    }
}

void Resize(Vec2i newSize)
{
    if (state == State::EDITOR)
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        cam.SetScreenSize(viewportRect.size);
        GraphicsModule* graphics = GraphicsModule::Get();
        InputModule* input = InputModule::Get();

        graphics->ResizeFrameBuffer(viewportBuffer, viewportRect.size);
        graphics->ResizeGBuffer(gBuffer, viewportRect.size);
        Vec2i clientArea = Engine::GetClientAreaSize();

        Vec2i newCenter = Vec2i((int)(viewportRect.location.x + viewportRect.size.x / 2.0f), (int)(viewportRect.location.y + viewportRect.size.y / 2.0f));
        //TODO(fraser): clean up mouse constrain/input code
        input->SetMouseCenter(newCenter);
        Engine::SetCursorCenter(newCenter);
    }
    else if (state == State::GAME)
    {
        GraphicsModule* graphics = GraphicsModule::Get();
        InputModule* input = InputModule::Get();

        cam.SetScreenSize(Engine::GetClientAreaSize());
        graphics->ResizeFrameBuffer(viewportBuffer, Engine::GetClientAreaSize());
        graphics->ResizeGBuffer(gBuffer, Engine::GetClientAreaSize());

        Vec2i newCenter = Vec2i(Engine::GetClientAreaSize().x / 2, Engine::GetClientAreaSize().y / 2);
        //TODO(fraser): clean up mouse constrain/input code
        input->SetMouseCenter(newCenter);
        Engine::SetCursorCenter(newCenter);
    }
}
#else 

#ifdef USE_EDITOR

#include "GameEngine.h"

#include "States/Editor/EditorState.h"
#include "State/StateMachine.h"

StateMachine Machine;

void Initialize(ArgsList args)
{
    EditorState* EdState = new EditorState();
    Machine.PushState(EdState);
}

void Update(double deltaTime)
{
    Machine.Update(deltaTime);
}

void Resize(Vec2i newSize)
{
    Machine.Resize();
}

#else

#include "GameEngine.h"

#include "States/Game/GameState.h"
#include "State/StateMachine.h"

StateMachine Machine;

// CHANGE THIS TO SET INITIAL LEVEL
//const std::string InitialLevelName = "levels\\BullTest.lvl";
//const std::string InitialLevelName = "levels\\Bouncy.lvl";
//const std::string InitialLevelName = "levels\\PhysWorld1Ball.lvl";
const std::string InitialLevelName = "levels\\PhysWorld.lvl";
//const std::string InitialLevelName = "levels\\2BallsPlat.lvl";
const std::string TitleBarText = "Bounce";

void Initialize(ArgsList args)
{
    Engine::SetWindowTitleText(TitleBarText);

    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();

    GameState* GState = new GameState();
    
    Graphics->SetRenderMode(RenderMode::DEFAULT);
   
    AssetRegistry* Registry = AssetRegistry::Get();

    Registry->LoadStaticMesh("models/ArrowSmooth.obj"),
    Registry->LoadStaticMesh("models/RotationHoop.obj");

    //GameScene->SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });
    
    GState->LoadScene(InitialLevelName);
    
    Machine.PushState(GState);
}

void Update(double deltaTime)
{
    Machine.Update(deltaTime);
    if (!Machine.HasState())
    {
        Engine::StopGame();
    }
}

void Resize(Vec2i newSize)
{
    Machine.Resize();
}


#endif

#endif