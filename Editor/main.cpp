#include "..\GameEngine\GameEngine.h"
#include "GameEngine.h"

#include "EditorScene.h"

#include "Scene.h"

#include <cmath>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>

static bool FirstPersonPlayerEnabled = false;

static Vec3f SunLight = Vec3f(0.8f, 0.7f, 0.9f);

struct Player
{
    bool grounded = false;
    Vec3f position;
    Vec3f velocity;
    Camera* cam;
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
    VERTEX
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

Texture translateToolTexture;
Texture rotateToolTexture;
Texture scaleToolTexture;

Texture vertexToolTexture;

Material tempWhiteMaterial;

Texture gridTexture;
Model gridModel;

Font testFont;
Font inspectorFont;

Camera cam;

Framebuffer_ID viewportBuffer;
Framebuffer_ID widgetViewportBuffer;

StaticMesh quadMesh;

bool holdingAlt = false;
bool cursorLocked = false;

std::vector<Material> loadedMaterials;
std::vector<Model> loadedModels;

std::vector<Framebuffer_ID> modelFBuffers;
Camera modelCam;

Scene scene;

bool draggingNewModel = false;
Model* draggingModel = nullptr;

bool draggingNewTexture = false;
Material draggingMaterial;

bool draggingNewBehaviour = false;
std::string draggingBehaviourName;

static Model* selectedModelPtr = nullptr;

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

void DrawToolWidgets(ModuleManager& modules)
{
    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();

    // Draw tools on separate buffer to they're in front
    graphics.SetActiveFrameBuffer(widgetViewportBuffer);
    {
        if (selectedModelPtr && toolMode == ToolMode::MOVE)
        {
            graphics.SetRenderMode(RenderMode::FULLBRIGHT);
            if (moveMode == MoveMode::TRANSLATE)
            {
                Vec3f movingModelPos = selectedModelPtr->GetTransform().GetPosition();

                float scale = Math::magnitude(movingModelPos - cam.GetPosition()) * 0.05f;

                xAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(4.0f, 0.0f, 0.0f) * scale);
                yAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(0.0f, 4.0f, 0.0f) * scale);
                zAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(0.0f, 0.0f, 4.0f) * scale);

                xAxisArrow.GetTransform().SetScale(Vec3f(scale / 3.f, scale, scale / 3.f));
                yAxisArrow.GetTransform().SetScale(Vec3f(scale / 3.f, scale, scale / 3.f));
                zAxisArrow.GetTransform().SetScale(Vec3f(scale / 3.f, scale, scale / 3.f));

                graphics.DebugDrawAABB(collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox, Vec3f(1.0f, 1.0f, 0.7f), selectedModelPtr->GetTransform().GetTransformMatrix());

                graphics.Draw(xAxisArrow);
                graphics.Draw(yAxisArrow);
                graphics.Draw(zAxisArrow);
            }

            if (moveMode == MoveMode::ROTATE)
            {
                Vec3f rotatingModelPos = selectedModelPtr->GetTransform().GetPosition();

                xAxisRing.GetTransform().SetPosition(rotatingModelPos);
                yAxisRing.GetTransform().SetPosition(rotatingModelPos);
                zAxisRing.GetTransform().SetPosition(rotatingModelPos);

                float scale = Math::magnitude(rotatingModelPos - cam.GetPosition()) * 0.2f;

                xAxisRing.GetTransform().SetScale(Vec3f(scale, scale, scale));
                yAxisRing.GetTransform().SetScale(Vec3f(scale, scale, scale));
                zAxisRing.GetTransform().SetScale(Vec3f(scale, scale, scale));

                graphics.DebugDrawAABB(collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox, Vec3f(1.0f, 1.0f, 0.7f), selectedModelPtr->GetTransform().GetTransformMatrix());

                graphics.Draw(xAxisRing);
                graphics.Draw(yAxisRing);
                graphics.Draw(zAxisRing);

            }
            if (moveMode == MoveMode::SCALE)
            {
                Vec3f scalingModelPos = selectedModelPtr->GetTransform().GetPosition();
                Quaternion scalingModelRot = selectedModelPtr->GetTransform().GetRotation();

                Vec3f modelScale = selectedModelPtr->GetTransform().GetScale();
                float scale = Math::magnitude(scalingModelPos - cam.GetPosition()) * 0.05f;

                xScaleWidget.GetTransform().SetScale(Vec3f(scale, scale, scale));
                yScaleWidget.GetTransform().SetScale(Vec3f(scale, scale, scale));
                zScaleWidget.GetTransform().SetScale(Vec3f(scale, scale, scale));

                xScaleWidget.GetTransform().SetPosition(scalingModelPos + Vec3f(modelScale.x, 0.0f, 0.0f) * scalingModelRot);
                yScaleWidget.GetTransform().SetPosition(scalingModelPos + Vec3f(0.0f, modelScale.y, 0.0f) * scalingModelRot);
                zScaleWidget.GetTransform().SetPosition(scalingModelPos + Vec3f(0.0f, 0.0f, modelScale.z) * scalingModelRot);

                xScaleWidget.GetTransform().SetRotation(scalingModelRot * Quaternion(Vec3f(0.0f, 1.0f, 0.0f), M_PI_2));
                yScaleWidget.GetTransform().SetRotation(scalingModelRot * Quaternion(Vec3f(1.0f, 0.0f, 0.0f), -M_PI_2));
                zScaleWidget.GetTransform().SetRotation(scalingModelRot * Quaternion(Vec3f(0.0f, 0.0f, 1.0f), M_PI_2));

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
    mousePos.x = mousePosition.x - viewPort.location.x;
    mousePos.y = mousePosition.y - viewPort.location.y;

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

    float speed = CamSpeed * deltaTime;

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

            StaticMesh newMesh = graphics.LoadMesh(fileName);

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
    std::vector<Material> loadedMaterials;

    std::string path = "textures";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        if (ext.string() == ".png" || ext.string() == ".jpg")
        {
            std::string fileName = entry.path().generic_string();

            Engine::DEBUGPrint(fileName);

            Texture newTexture = graphics.LoadTexture(fileName);

            loadedMaterials.push_back(graphics.CreateMaterial(newTexture));
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

    if (input.GetMouseState().IsButtonDown(Mouse::LMB))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

        if (finalHit.rayCastHit.hit)
        {
            selectedModelPtr = finalHit.hitModel;
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

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (draggingNewBehaviour)
        return;

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
    Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);
    
    if (!viewportRect.contains(Engine::GetMousePosition()))
    {
        return;
    }

    if (input.GetMouseState().IsButtonDown(Mouse::LMB))
    {
        if (!draggingNewBox)
        {
            // Find intersect with horizontal plane as a default box start point
            RayCastHit finalHit = collisions.RayCast(mouseRay, Plane{ Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f) });

            // Test against existing level geometry
            SceneRayCastHit levelHit = scene.RayCast(mouseRay, collisions);

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

            selectedModelPtr = nullptr;
        }
    }

}

void UpdateModelPlace(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    if (draggingNewModel)
    {
        if (input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

            if (finalHit.rayCastHit.hit)
            {
                RayCastHit hit = finalHit.rayCastHit;
                draggingModel->GetTransform().SetPosition(hit.hitPoint);
                Quaternion q = Math::VecDiffToQuat(hit.hitNormal, Vec3f(0.0f, 0.0f, 1.0f));
                q = Math::normalize(q);

                draggingModel->GetTransform().SetRotation(q);

            }
            else
            {
                draggingModel->GetTransform().SetPosition(mouseRay.point + mouseRay.direction * 12.0f);

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
        if (!input.GetMouseState().IsButtonDown(Mouse::LMB))
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
            Line axisLine(selectedModelPtr->GetTransform().GetPosition(), Vec3f(1.0f, 0.0f, 0.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            selectedModelPtr->GetTransform().SetPosition(pointAlongAxis);
        }

        if (slidingY)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedModelPtr->GetTransform().GetPosition(), Vec3f(0.0f, 1.0f, 0.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            selectedModelPtr->GetTransform().SetPosition(pointAlongAxis);
        }

        if (slidingZ)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedModelPtr->GetTransform().GetPosition(), Vec3f(0.0f, 0.0f, 1.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            selectedModelPtr->GetTransform().SetPosition(pointAlongAxis);
        }


        return;
    }

    if (selectedModelPtr)
    {
        if (input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            CollisionMesh& arrowToolCollMesh = collisions.GetCollisionMeshFromMesh(xAxisArrow.m_TexturedMeshes[0].m_Mesh);

            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);
            
            RayCastHit hit;

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, xAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - selectedModelPtr->GetTransform().GetPosition();
                slidingX = true;
            }

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, yAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - selectedModelPtr->GetTransform().GetPosition();
                slidingY = true;
            }

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, zAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - selectedModelPtr->GetTransform().GetPosition();
                slidingZ = true;
            }

            if (slidingX || slidingY || slidingZ)
            {
                return;
            }
        }
    }
    
    if (input.GetMouseState().IsButtonDown(Mouse::LMB))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

        if (finalHit.rayCastHit.hit)
        {
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
        if (!input.GetMouseState().IsButtonDown(Mouse::LMB))
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

        axisPlane.center = selectedModelPtr->GetTransform().GetPosition();

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

        Vec3f objectCenter = selectedModelPtr->GetTransform().GetPosition();
        Vec3f hitPoint = planeHit.hitPoint;
        Vec3f angleVec = hitPoint - objectCenter;
        float dot = Math::dot(angleVec, perpUnitVec);
        float det = Math::dot(axisPlane.normal, (Math::cross(angleVec, perpUnitVec)));
        
        float deltaAngle = initialAngle - atan2(det, dot);

        Quaternion axisQuat = Quaternion(axisPlane.normal, deltaAngle);

        selectedModelPtr->GetTransform().SetRotation(axisQuat * initialRotation);
    }
    else if (selectedModelPtr)
    {
        if (input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            CollisionMesh& arrowToolCollMesh = collisions.GetCollisionMeshFromMesh(xAxisRing.m_TexturedMeshes[0].m_Mesh);

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

                axisPlane.center = selectedModelPtr->GetTransform().GetPosition();

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

                initialRotation = selectedModelPtr->GetTransform().GetRotation();

                Vec3f objectCenter = selectedModelPtr->GetTransform().GetPosition();
                Vec3f hitPoint = planeHit.hitPoint;
                Vec3f angleVec = hitPoint - objectCenter;
                float dot = Math::dot(angleVec, perpUnitVec);
                float det = Math::dot(axisPlane.normal, (Math::cross(angleVec, perpUnitVec)));
                initialAngle = atan2(det, dot);
            }
        }
    }
    if (input.GetMouseState().IsButtonDown(Mouse::LMB) && (!rotatingX && !rotatingY && !rotatingZ))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

        if (finalHit.rayCastHit.hit)
        {
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
        if (!input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            scalingX = false;
            scalingY = false;
            scalingZ = false;

            return;
        }

        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        Quaternion scalingModelRot = selectedModelPtr->GetTransform().GetRotation();

        if (scalingX)
        {
            Line mouseLine(mouseRay.point - scalingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedModelPtr->GetTransform().GetPosition(), Vec3f(1.0f, 0.0f, 0.0f) * scalingModelRot);
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            Vec3f prevScale = selectedModelPtr->GetTransform().GetScale();

            float newScale = Math::magnitude(selectedModelPtr->GetTransform().GetPosition() - pointAlongAxis);

            selectedModelPtr->GetTransform().SetScale(Vec3f(newScale, prevScale.y, prevScale.z));
        }
        if (scalingY)
        {
            Line mouseLine(mouseRay.point - scalingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedModelPtr->GetTransform().GetPosition(), Vec3f(0.0f, 1.0f, 0.0f) * scalingModelRot);
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            Vec3f prevScale = selectedModelPtr->GetTransform().GetScale();

            float newScale = Math::magnitude(selectedModelPtr->GetTransform().GetPosition() - pointAlongAxis);

            selectedModelPtr->GetTransform().SetScale(Vec3f(prevScale.x, newScale, prevScale.z));
        }
        if (scalingZ)
        {
            Line mouseLine(mouseRay.point - scalingStartHitPoint, mouseRay.direction);
            Line axisLine(selectedModelPtr->GetTransform().GetPosition(), Vec3f(0.0f, 0.0f, 1.0f) * scalingModelRot);
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            Vec3f prevScale = selectedModelPtr->GetTransform().GetScale();

            float newScale = Math::magnitude(selectedModelPtr->GetTransform().GetPosition() - pointAlongAxis);

            selectedModelPtr->GetTransform().SetScale(Vec3f(prevScale.x, prevScale.y, newScale));
        }
    }
    else if (selectedModelPtr)
    {
        if (input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            CollisionMesh& scaleToolCollMesh = collisions.GetCollisionMeshFromMesh(xScaleWidget.m_TexturedMeshes[0].m_Mesh);
            
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

    if (input.GetMouseState().IsButtonDown(Mouse::LMB) && (!scalingX && !scalingY && !scalingZ))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

        if (finalHit.rayCastHit.hit)
        {
            selectedModelPtr = finalHit.hitModel;
        }
    }
}

void UpdateTexturePlace(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    if (draggingNewTexture)
    {
        if (!input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            draggingNewTexture = false;

            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

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
        if (!input.GetMouseState().IsButtonDown(Mouse::LMB))
        {
            draggingNewBehaviour = false;
            
            Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
            Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

            SceneRayCastHit finalHit = scene.RayCast(mouseRay, collisions);

            if (finalHit.rayCastHit.hit)
            {

                BehaviourRegistry::Get()->AttachNewBehaviour(draggingBehaviourName, finalHit.hitModel);
            }
        }
    }
}

void UpdateEditor(ModuleManager& modules, double deltaTime)
{
    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();
    UIModule& ui = *modules.GetUI();
    InputModule& input = *modules.GetInput();

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
        UpdateModelPlace(input, collisions, graphics);
        UpdateModelTranslate(input, collisions, graphics);
        UpdateModelRotate(input, collisions, graphics);
        UpdateModelScale(input, collisions, graphics);
        UpdateTexturePlace(input, collisions, graphics);
        UpdateBehaviourPlace(input, collisions);
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
        scene.Draw(graphics, viewportBuffer);

        if (draggingModel)
        {
            graphics.Draw(*draggingModel);
        }
        if (selectedModelPtr)
        {
            AABB aabb = collisions.GetCollisionMeshFromMesh(selectedModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox;
            graphics.DebugDrawAABB(aabb, Vec3f(0.6f, 0.95f, 0.65f), selectedModelPtr->GetTransform().GetTransformMatrix());
            //graphics.DebugDrawModelMesh(*selectedModelPtr, Vec3f(0.9f, 0.8f, 0.4f));
        }
    }
    graphics.SetRenderMode(RenderMode::FULLBRIGHT);
    graphics.Draw(gridModel);
    graphics.SetRenderMode(renderMode);

    graphics.ResetFrameBuffer();
    
    DrawToolWidgets(modules);

    Vec2i screen = Engine::GetClientAreaSize();

    ui.BufferPanel(viewportBuffer, newViewport);
    ui.BufferPanel(widgetViewportBuffer, newViewport);


    ui.StartFrame(Rect(Vec2f(0.0f, 0.0f), Vec2f(screen.x - 200.0f, 40.0f)), 0.0f, "MEMES");

    {
        if (ui.ImgButton(playButtonTexture, Rect(Vec2f(0.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f))
        {
            state = State::GAME;
            player.velocity = Vec3f(0.0f, 0.0f, 0.0f);
            player.position = cam.GetPosition();
        }

        if (ui.ImgButton(cameraButtonTexture, Rect(Vec2f(40.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f))
        {
            
        }
        if (ui.TextButton("Open", Rect(Vec2f(80.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f))
        {
            std::string FileName;
            if (Engine::FileOpenDialog(FileName))
            {
                Engine::DEBUGPrint(FileName);
                scene.Load(FileName);
            }
        }
        if (ui.TextButton("Save", Rect(Vec2f(120.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f))
        {
            std::string FileName;
            if (Engine::FileSaveDialog(FileName))
            {
                Engine::DEBUGPrint(FileName);
                scene.Save(FileName);
            }
        }

        //ui.ImgButton(loadedTextures[1], Rect(Vec2f(160.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f);
        //ui.ImgButton(loadedTextures[1], Rect(Vec2f(200.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f);
        //ui.ImgButton(loadedTextures[1], Rect(Vec2f(240.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f);
        //ui.ImgButton(loadedTextures[1], Rect(Vec2f(280.0f, 0.0f), Vec2f(40.0f, 40.0f)), 4.0f);
    }

    ui.EndFrame();
    
    ui.StartFrame(Rect(Vec2f(100.0f, screen.y - 200), Vec2f(screen.x - 300.0f, 200.0f)), 20.0f, "Resources");

    ui.StartTab("Models");
    for (int i = 0; i < loadedModels.size(); ++i)
    {
        
        if (ui.BufferButton(modelFBuffers[i], Rect(Vec2f(i * 100.0f, 0.0f), Vec2f(100, 100)), 10).clicking)
        {
            if (!draggingNewModel)
            {
                draggingNewModel = true;
           
                selectedModelPtr = nullptr;

                draggingModel = new Model(graphics.CloneModel(loadedModels[i]));
            }
        }
    }
    ui.EndTab();

    ui.StartTab("Textures");
    for (int i = 0; i < loadedMaterials.size(); ++i)
    {
        if (ui.ImgButton(loadedMaterials[i].m_DiffuseTexture, Rect(Vec2f(i * 40, 0.0f), Vec2f(40, 80)), 2.5f).clicking)
        {
            if (!draggingNewTexture)
            {
                draggingNewTexture = true;

                selectedModelPtr = nullptr;

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
        if (ui.TextButton(Behaviour.first, Rect(Vec2f(i * 80, 0.0f), Vec2f(80, 80)), 2.0f).clicking)
        {
            if (!draggingNewBehaviour)
            {
                draggingNewBehaviour = true;

                selectedModelPtr = nullptr;

                draggingBehaviourName = Behaviour.first;
            }
        }
        i++;                
    }

    ui.EndTab();

    ui.EndFrame();


    ui.StartFrame(Rect(Vec2f(screen.x - 200.0f, 0.0f), Vec2f(200.0f, screen.y / 2)), 20.0f, "Inspector");

    std::string posText;

    if (selectedModelPtr)
    {
        Vec3f pos = selectedModelPtr->GetTransform().GetPosition();
        std::string xText = "X: " + std::to_string(pos.x);
        std::string yText = "Y: " + std::to_string(pos.y);
        std::string zText = "Z: " + std::to_string(pos.z);

        text.DrawText(xText, &inspectorFont, Vec2f(screen.x - 180.0f, 20.0f), Vec3f(0.0f, 0.0f, 0.0f));
        text.DrawText(yText, &inspectorFont, Vec2f(screen.x - 180.0f, 35.0f), Vec3f(0.0f, 0.0f, 0.0f));
        text.DrawText(zText, &inspectorFont, Vec2f(screen.x - 180.0f, 50.0f), Vec3f(0.0f, 0.0f, 0.0f));

        //static std::string testString = "Hello";
        //text.DrawText(testString, &inspectorFont, Vec2f(screen.x - 180.0f, 65.0f));

        ui.TextEntry(selectedModelPtr->m_Name, Rect(Vec2f(0.0f, 65.0f), Vec2f(160.0f, 20.0f)));
    }
    else
    {
        posText = "No model selected.";
        text.DrawText(posText, &inspectorFont, Vec2f(screen.x - 180.0f, 20.0f), Vec3f(0.0f, 0.0f, 0.0f));
    }


    ui.EndFrame();

    ui.StartFrame(Rect(Vec2f(screen.x - 200.0f, screen.y / 2), Vec2f(200.0f, screen.y / 2)), 20.0f, "Entities");

    if (Model* modelPtr = scene.MenuListEntities(ui, inspectorFont))
    {
        selectedModelPtr = modelPtr;
    }

    ui.EndFrame();

    if (draggingNewTexture)
    {
        ui.ImgPanel(draggingMaterial.m_DiffuseTexture, Rect(Engine::GetMousePosition(), Vec2f(80.0f, 80.0f)));
    }

    if (draggingNewBehaviour)
    {
        ui.Text(draggingBehaviourName, Engine::GetMousePosition(), Vec3f(1.0f, 1.0f, 1.0f));
    }

    if (ui.ImgButton(cursorToolTexture, Rect(Vec2f(0.0f, 40.0f), Vec2f(100.0f, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::SELECT;
    }
    if (ui.ImgButton(boxToolTexture, Rect(Vec2f(0.0f, 140.0f), Vec2f(100.0f, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::GEOMETRY;
    }

    if (moveMode == MoveMode::TRANSLATE)
    {
        if (ui.ImgButton(translateToolTexture, Rect(Vec2f(0.0f, 240.0f), Vec2f(100.0f, 100.0f)), 20.0f))
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
        if (ui.ImgButton(rotateToolTexture, Rect(Vec2f(0.0f, 240.0f), Vec2f(100.0f, 100.0f)), 20.0f))
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
        if (ui.ImgButton(scaleToolTexture, Rect(Vec2f(0.0f, 240.0f), Vec2f(100.0f, 100.0f)), 20.0f))
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

    if (ui.ImgButton(vertexToolTexture, Rect(Vec2f(0.0f, 340.0f), Vec2f(100, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::VERTEX;
    }

    if (ui.ImgButton(cameraButtonTexture, Rect(Vec2f(0.0f, 440.0f), Vec2f(100.0f, 100.0f)), 20.0f))
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

    if (input.IsKeyDown(Key::Escape))
    {
        selectedModelPtr = nullptr;
    }

    if (input.GetKeyState(Key::Delete))
    {
        if (selectedModelPtr) {
            scene.DeleteModel(selectedModelPtr);
            selectedModelPtr = nullptr;
        }
    }

    if (input.IsKeyDown(Key::One))
    {
        toolMode = ToolMode::SELECT;
    }
    if (input.IsKeyDown(Key::Two))
    {
        toolMode = ToolMode::GEOMETRY;
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

    std::string modeString;
    switch (toolMode)
    {
    case ToolMode::SELECT:
        modeString = "Select mode";
        break;
    case ToolMode::GEOMETRY:
        modeString = "Box mode";
        break;
    case ToolMode::MOVE:
        switch (moveMode)
        {
        case MoveMode::TRANSLATE:
            modeString = "Translate mode";
            break;
        case MoveMode::ROTATE:
            modeString = "Rotate mode";
            break;
        case MoveMode::SCALE:
            modeString = "Scale mode";
            break;
        }
        break;
    case ToolMode::VERTEX:
        modeString = "Vertex Edit Mode";
        break;
    }

    text.DrawText(modeString, &testFont, Vec2f(100.0f, 40.0f), Vec3f(0.0f, 0.0f, 0.0f));

    // END DRAW
}

void UpdateGame(ModuleManager& modules, double deltaTime)
{
    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();
    InputModule& input = *modules.GetInput();
    UIModule& ui = *modules.GetUI();

    if (input.IsKeyDown(Key::Escape))
    {
        state = State::EDITOR;
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

        player.velocity.x = inputDir.x;
        player.velocity.y = inputDir.y;
        player.velocity.z += -0.005f;

        //player.velocity += Vec3f(0.0f, 0.0f, -0.01f);

        player.grounded = false;

        SceneRayCastHit movement = scene.RayCast(Ray(player.position, Math::normalize(player.velocity)), collisions);

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
            player.velocity.z = 0.175f;
        }

        if (movement.rayCastHit.hit && movement.rayCastHit.hitDistance > Math::magnitude(player.velocity))
        {
            graphics.DebugDrawPoint(movement.rayCastHit.hitPoint);
        }

        player.cam->SetPosition(player.position + Vec3f(0.0f, 0.0f, 2.5f));
    }

    // Update behaviours
    BehaviourRegistry::Get()->UpdateAllBehaviours(modules, &scene, deltaTime);


    // Drop models

    static Model* droppingModel = nullptr;

    static Vec3f modelSpeed = Vec3f(0.0f, 0.0f, 0.0f);

    //if (input.GetMouseState().IsButtonDown(Mouse::LMB))
    //{
    //    SceneRayCastHit modelHit = scene.RayCast(Ray(cam.GetPosition(), cam.GetDirection()), collisions);
    //    if (modelHit.rayCastHit.hit)
    //    {
    //        droppingModel = modelHit.hitModel;
    //        modelSpeed = Vec3f(0.0f, 0.0f, 0.0f);
    //    }
    //}
    if (droppingModel)
    {
        modelSpeed.z += -0.005f;
        droppingModel->GetTransform().Move(modelSpeed);
    }

    graphics.SetActiveFrameBuffer(viewportBuffer);
    {
        scene.SetCamera(&cam);
        scene.Draw(graphics, viewportBuffer);

    }
    graphics.ResetFrameBuffer();



    Rect screenRect;
    screenRect.size = Engine::GetClientAreaSize();

    ui.BufferPanel(viewportBuffer, screenRect);

    text.DrawText("Frame Time: " + std::to_string(deltaTime), &testFont, Vec2f(0.0f, 0.0f));

    int FPS = round(1.0 / deltaTime);

    text.DrawText("FPS: " + std::to_string(FPS), &testFont, Vec2f(0.0f, 30.0f));
    //text.DrawText(std::to_string(player.position.x) + ", " + std::to_string(player.position.y) + ", " + std::to_string(player.position.z), &testFont, Vec2f(0.0f, 0.0f), Vec3f(0.6f, 0.2f, 0.7f));
    //std::string groundedText = player.grounded ? "Grounded" : "Not Grounded";
    //text.DrawText(groundedText, &testFont, Vec2f(0.0f, 30.0f));

    //text.DrawText(std::to_string(hitDist), &testFont, Vec2f(0.0f, 0.0f), Vec3f(1.0f, 0.5f, 0.5f));
    //text.DrawText(std::to_string(testHitDist), &testFont, Vec2f(0.0f, 24.0f), Vec3f(1.0f, 0.5f, 0.5f));
    //text.DrawText(std::to_string(playerVel), &testFont, Vec2f(0.0f, 48.0f), Vec3f(1.0f, 0.5f, 0.5f));
}

void Initialize(ModuleManager& modules)
{
    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();
    InputModule& input = *modules.GetInput();

    Texture redTexture = graphics.LoadTexture("textures/red.png");
    Texture greenTexture = graphics.LoadTexture("textures/green.png");
    Texture blueTexture = graphics.LoadTexture("textures/blue.png");

    scene.Init(graphics, collisions);
    

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    xAxisArrow = graphics.CreateModel(TexturedMesh(
        graphics.LoadMesh("models/ArrowSmooth.obj"),
        graphics.CreateMaterial(graphics.LoadTexture("textures/whiteTexture.png"))
    ));
    
    yAxisArrow = graphics.CloneModel(xAxisArrow);
    zAxisArrow = graphics.CloneModel(xAxisArrow);

    xAxisArrow.GetTransform().SetRotation(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -M_PI_2));
    zAxisArrow.GetTransform().SetRotation(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), M_PI_2));

    xAxisArrow.SetMaterial(graphics.CreateMaterial(redTexture));
    yAxisArrow.SetMaterial(graphics.CreateMaterial(greenTexture));
    zAxisArrow.SetMaterial(graphics.CreateMaterial(blueTexture));

    xAxisRing = graphics.CreateModel(TexturedMesh(
        graphics.LoadMesh("models/RotationHoop.obj"),
        graphics.CreateMaterial(redTexture)
    ));

    yAxisRing = graphics.CloneModel(xAxisRing);
    zAxisRing = graphics.CloneModel(xAxisRing);

    xAxisRing.GetTransform().SetRotation(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), M_PI_2));
    zAxisRing.GetTransform().SetRotation(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), M_PI_2));

    yAxisRing.SetMaterial(graphics.CreateMaterial(greenTexture));
    zAxisRing.SetMaterial(graphics.CreateMaterial(blueTexture));

    xScaleWidget = graphics.CreateModel(TexturedMesh(
        graphics.LoadMesh("models/ScaleWidget.obj"),
        graphics.CreateMaterial(redTexture)
    ));

    yScaleWidget = graphics.CloneModel(xScaleWidget);
    zScaleWidget = graphics.CloneModel(xScaleWidget);

    yScaleWidget.SetMaterial(graphics.CreateMaterial(greenTexture));
    zScaleWidget.SetMaterial(graphics.CreateMaterial(blueTexture));

    playButtonTexture = graphics.LoadTexture("images/playButton.png");
    cameraButtonTexture = graphics.LoadTexture("images/cameraButton.png");

    cursorToolTexture = graphics.LoadTexture("images/cursorTool.png");
    boxToolTexture = graphics.LoadTexture("images/boxTool.png");
    
    translateToolTexture = graphics.LoadTexture("images/translateTool.png");
    rotateToolTexture = graphics.LoadTexture("images/rotateTool.png");
    scaleToolTexture = graphics.LoadTexture("images/scaleTool.png");

    vertexToolTexture = graphics.LoadTexture("images/vertexTool.png");

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

    std::vector<float> quadVertices =
    {
        -1.0f, -1.0f,       0.0f, 0.0f,
        -1.0f, -0.6f,       0.0f, 1.0f,
        -0.6f, -0.6f,       1.0f, 1.0f,
        -0.6f, -1.0f,       1.0f, 0.0f,
    };

    std::vector<ElementIndex> quadIndices =
    {
        0, 1, 2, 0, 2, 3
    };

    StaticMesh_ID quadMeshId = graphics.m_Renderer.LoadMesh(VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f }), quadVertices, quadIndices);
    quadMesh.Id = quadMeshId;
    quadMesh.LoadedFromFile = false;

    graphics.InitializeDebugDraw(viewportBuffer);

    graphics.SetCamera(&cam);
    
    graphics.SetRenderMode(renderMode);

    tempWhiteMaterial = graphics.CreateMaterial(graphics.LoadTexture("images/white.png", TextureMode::NEAREST, TextureMode::NEAREST));

    gridTexture = graphics.LoadTexture("images/grid.png", TextureMode::LINEAR, TextureMode::NEAREST);
    gridModel = graphics.CreatePlaneModel(Vec2f(-4000.0f, -4000.0f), Vec2f(4000.0f, 4000.0f), graphics.CreateMaterial(gridTexture));

    testFont = text.LoadFont("fonts/ARLRDBD.TTF", 30);
    inspectorFont = text.LoadFont("fonts/ARLRDBD.TTF", 15);

    loadedModels = LoadModels(graphics);
    
    loadedMaterials = LoadMaterials(graphics);

    Vec2i clientArea = Engine::GetClientAreaSize();

    Vec2i newCenter = Vec2i(viewportRect.location.x + viewportRect.size.x / 2, viewportRect.location.y + viewportRect.size.y / 2);
    //TODO(fraser): clean up mouse constrain/input code
    input.SetMouseCenter(newCenter);
    Engine::SetCursorCenter(newCenter);

    player.cam = &cam;
    
    auto Behaviours = BehaviourRegistry::Get()->GetBehaviours();

}

void Update(ModuleManager& modules, double deltaTime)
{
    if (state == State::EDITOR)
    {
        UpdateEditor(modules, deltaTime);
    }
    else if (state == State::GAME)
    {
        UpdateGame(modules, deltaTime);
    }
}

void Resize(ModuleManager& modules, Vec2i newSize)
{
    if (state == State::EDITOR)
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        cam.SetScreenSize(viewportRect.size);
        GraphicsModule* graphics = modules.GetGraphics();
        InputModule* input = modules.GetInput();

        graphics->ResizeFrameBuffer(viewportBuffer, viewportRect.size);
        Vec2i clientArea = Engine::GetClientAreaSize();

        Vec2i newCenter = Vec2i(viewportRect.location.x + viewportRect.size.x / 2, viewportRect.location.y + viewportRect.size.y / 2);
        //TODO(fraser): clean up mouse constrain/input code
        input->SetMouseCenter(newCenter);
        Engine::SetCursorCenter(newCenter);
    }
    else if (state == State::GAME)
    {
        GraphicsModule* graphics = modules.GetGraphics();
        InputModule* input = modules.GetInput();

        cam.SetScreenSize(Engine::GetClientAreaSize());
        graphics->ResizeFrameBuffer(viewportBuffer, Engine::GetClientAreaSize());

        Vec2i newCenter = Vec2i(Engine::GetClientAreaSize().x / 2, Engine::GetClientAreaSize().y / 2);
        //TODO(fraser): clean up mouse constrain/input code
        input->SetMouseCenter(newCenter);
        Engine::SetCursorCenter(newCenter);
    }
}
