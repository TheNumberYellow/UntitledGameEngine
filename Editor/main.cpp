#include "..\GameEngine\GameEngine.h"
#include "GameEngine.h"

#include "EditorScene.h"

#include "Scene.h"

#include <cmath>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>

struct Player
{
    bool grounded = false;
    Vec3f position;
    Vec3f velocity;
    Camera* cam;
};

struct EditorModel
{
    Model* model;
    CollisionMesh* colMesh;
};

enum class State
{
    EDITOR,
    GAME
};

enum class ToolMode
{
    NORMAL,
    BOX,
    MOVE,
    VERTEX
};

Player player;

Texture_ID playButtonTexture;

Texture_ID cursorToolTexture;
Texture_ID boxToolTexture;
Texture_ID moveToolTexture;
Texture_ID vertexToolTexture;

Texture_ID tempWhiteTexture;

Font testFont;
Font inspectorFont;

Camera cam;

Model previewPSpawn;

Framebuffer_ID viewportBuffer;

Framebuffer_ID shadowBuffer;

Mesh_ID quadMesh;

Camera shadowCam;

Shader_ID shadowShader;
Shader_ID testDepthShader;

bool holdingAlt = false;
bool cursorLocked = false;

std::vector<Texture_ID> loadedTextures;
std::vector<Model> loadedModels;

std::vector<Framebuffer_ID> modelFBuffers;
Camera modelCam;

Scene scene;

bool draggingNewModel = false;
Model* draggingModel = nullptr;

bool draggingNewTexture = false;
Texture_ID draggingTexture;

static Model* movingModelPtr = nullptr;

Model xAxisArrow;
Model yAxisArrow;
Model zAxisArrow;

ToolMode toolMode = ToolMode::NORMAL;

State state = State::EDITOR;

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

void MoveCamera(InputModule& inputs, GraphicsModule& graphics, Camera& cam, float pixelToRadians)
{
    float speed = 0.075f;

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

    Vec3f shadowCamPos = cam.GetPosition() + (-shadowCam.GetDirection() * 40.0f);
    shadowCam.SetPosition(shadowCamPos);

    if (inputs.IsKeyDown(Key::Q))
    {
        shadowCam.SetPosition(cam.GetPosition());
        shadowCam.SetDirection(cam.GetDirection());
        
        //graphics.m_Renderer.SetShaderUniformVec3f(graphics.m_TexturedMeshShader, "SunDirection", cam.GetDirection());
    }
}

std::vector<Model> LoadModels(GraphicsModule& graphics)
{
    std::vector<Model> loadedModels;

    std::string path = "models";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        if (ext.string() == ".obj")
        {
            std::string fileName = entry.path().string();

            Engine::DEBUGPrint(fileName);

            Mesh_ID newMesh = graphics.LoadMesh(fileName);

            // For now we load all models with a temporary white texture
            Model newModel = graphics.CreateModel(TexturedMesh(newMesh, tempWhiteTexture));

            loadedModels.push_back(newModel);
            modelFBuffers.push_back(graphics.CreateFBuffer(Vec2i(100, 100), FBufferFormat::COLOUR));
        }

    }

    return loadedModels;
}

std::vector<Texture_ID> LoadTextures(GraphicsModule& graphics)
{
    std::vector<Texture_ID> loadedTextures;

    std::string path = "textures";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        if (ext.string() == ".png" || ext.string() == ".jpg")
        {
            std::string fileName = entry.path().string();

            Engine::DEBUGPrint(fileName);

            Texture_ID newTexture = graphics.LoadTexture(fileName);

            loadedTextures.push_back(newTexture);
        }
    }

    return loadedTextures;
}

void UpdateNormalDraw(InputModule& input, CollisionModule& collision, GraphicsModule& graphics)
{
    if (toolMode != ToolMode::NORMAL)
        return;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
        return;

    if (input.GetMouseState().IsButtonDown(Mouse::LMB))
    {
        Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());
        Ray mouseRay = GetMouseRay(cam, Engine::GetMousePosition(), viewportRect);

        SceneRayCastHit finalHit;

        finalHit = scene.RayCast(mouseRay, collision);

        if (finalHit.rayCastHit.hit)
        {
            RayCastHit hit = finalHit.rayCastHit;
            graphics.DebugDrawLine(hit.hitPoint, hit.hitPoint + hit.hitNormal, Vec3f(1.0f, 0.7f, 0.4f));
        }
    }
}

void UpdateBoxCreate(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool draggingNewBox = false;
    static Vec3f boxStartPoint;
    static float boxHeight = 5.0f;
    static AABB aabbBox;

    if (toolMode != ToolMode::BOX)
        return;

    if (draggingNewModel)
        return;

    if (draggingNewTexture)
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
                finalHit.hitPoint.x = Math::Round(finalHit.hitPoint.x, 2.5f);
                finalHit.hitPoint.y = Math::Round(finalHit.hitPoint.y, 2.5f);
                finalHit.hitPoint.z = Math::Round(finalHit.hitPoint.z, 2.5f);

                boxStartPoint = finalHit.hitPoint;
                draggingNewBox = true;
            }
        }
        else
        {
            RayCastHit hit = collisions.RayCast(mouseRay, Plane{ boxStartPoint, Vec3f(0.0f, 0.0f, 1.0f) });

            Vec3f originalHitPoint = hit.hitPoint;

            hit.hitPoint.x = Math::Round(hit.hitPoint.x, 2.5);
            hit.hitPoint.y = Math::Round(hit.hitPoint.y, 2.5f);
            hit.hitPoint.z = Math::Round(hit.hitPoint.z, 2.5f);

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
            EditorModel newBox;
            newBox.model = new Model(graphics.CreateBoxModel(aabbBox));
            newBox.colMesh = new CollisionMesh(collisions.GetCollisionMeshFromMesh(newBox.model->m_TexturedMeshes[0].m_Mesh));
            scene.AddModel(*(newBox.model));
            boxHeight = 5.0f;
            draggingNewBox = false;
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
                draggingModel->GetTransform().SetPosition(mouseRay.point + mouseRay.direction * 5);

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

void UpdateModelMove(InputModule& input, CollisionModule& collisions, GraphicsModule& graphics)
{
    static bool slidingX = false;
    static bool slidingY = false;
    static bool slidingZ = false;

    static Vec3f slidingStartHitPoint;

    if (toolMode != ToolMode::MOVE)
    {
        movingModelPtr = nullptr;
        slidingX = false;
        slidingY = false;
        slidingZ = false;

        return;
    }

    if (draggingNewModel || draggingNewTexture)
    {
        movingModelPtr = nullptr;
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
            Line axisLine(movingModelPtr->GetTransform().GetPosition(), Vec3f(1.0f, 0.0f, 0.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            movingModelPtr->GetTransform().SetPosition(pointAlongAxis);
        }

        if (slidingY)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(movingModelPtr->GetTransform().GetPosition(), Vec3f(0.0f, 1.0f, 0.0f));        
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            movingModelPtr->GetTransform().SetPosition(pointAlongAxis);
        }

        if (slidingZ)
        {
            Line mouseLine(mouseRay.point - slidingStartHitPoint, mouseRay.direction);
            Line axisLine(movingModelPtr->GetTransform().GetPosition(), Vec3f(0.0f, 0.0f, 1.0f));
            Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

            movingModelPtr->GetTransform().SetPosition(pointAlongAxis);
        }


        return;
    }

    if (movingModelPtr)
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
                slidingStartHitPoint = hit.hitPoint - movingModelPtr->GetTransform().GetPosition();
                slidingX = true;
            }

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, yAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - movingModelPtr->GetTransform().GetPosition();
                slidingY = true;
            }

            hit = collisions.RayCast(mouseRay, arrowToolCollMesh, zAxisArrow.GetTransform());
            if (hit.hit)
            {
                slidingStartHitPoint = hit.hitPoint - movingModelPtr->GetTransform().GetPosition();
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
            movingModelPtr = finalHit.hitModel;
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
                finalHit.hitModel->SetTexture(draggingTexture);
            }
        }
    }

}

void UpdateEditor(ModuleManager& modules)
{
    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();
    UIModule& ui = *modules.GetUI();
    InputModule& input = *modules.GetInput();

    //if (input.IsKeyDown(Key::Escape))
    //{
    //    Engine::StopGame();
    //    return;
    //}

    if (cursorLocked)
    {
        MoveCamera(input, graphics, cam, 0.001f);
    }

    Vec2i mousePos = Engine::GetMousePosition();

    Rect newViewport = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    if (Engine::IsWindowFocused())
    {
        UpdateNormalDraw(input, collisions, graphics);
        UpdateBoxCreate(input, collisions, graphics);
        UpdateModelPlace(input, collisions, graphics);
        UpdateModelMove(input, collisions, graphics);
        UpdateTexturePlace(input, collisions, graphics);
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

    DrawEditorGrid(graphics);

    // Draw all my little meshies
    graphics.SetCamera(&modelCam);
    for (int i = 0; i < modelFBuffers.size(); ++i)
    {
        loadedModels[i].GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.005f));
        graphics.SetActiveFrameBuffer(modelFBuffers[i]);
        graphics.Draw(loadedModels[i]);
    }

    // THIS IS ALL TEMPORARY WHILE I WORK ON A SHADOW SYSTEM IN THE GRAPHICS MODULE :^) 
    graphics.SetActiveFrameBuffer(shadowBuffer);
    //graphics.m_Renderer.SetCulling(Cull::Front);
    graphics.m_Renderer.SetActiveShader(shadowShader);
    {
        //graphics.m_Renderer.SetShaderUniformMat4x4f(shadowShader, "lightSpaceMatrix", shadowCam.GetCamMatrix());
        //for (int i = 0; i < boxes.size(); ++i)
        //{
        //    graphics.m_Renderer.SetShaderUniformMat4x4f(shadowShader, "Transformation", boxes[i].model->GetTransform().GetTransformMatrix());
        //    graphics.m_Renderer.DrawMesh(boxes[i].model->m_TexturedMeshes[0].m_Mesh);
        //}
        //for (int i = 0; i < sceneModels.size(); ++i)
        //{
        //    graphics.m_Renderer.SetShaderUniformMat4x4f(shadowShader, "Transformation", sceneModels[i].model->GetTransform().GetTransformMatrix());
        //    graphics.m_Renderer.DrawMesh(sceneModels[i].model->m_TexturedMeshes[0].m_Mesh);
        //}
    }
    graphics.ResetFrameBuffer();
    //graphics.m_Renderer.SetCulling(Cull::Back);

    // THIS IS ALL TEMPORARY WHILE I WORK ON A SHADOW SYSTEM IN THE GRAPHICS MODULE :^) 
    graphics.SetCamera(&cam);
    graphics.SetActiveFrameBuffer(viewportBuffer);
    {
        //graphics.m_Renderer.SetActiveFBufferTexture(shadowBuffer, 1);
        //graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_TexturedMeshShader, "Camera", graphics.m_Camera->GetCamMatrix());
        //graphics.m_Renderer.SetShaderUniformVec3f(graphics.m_TexturedMeshShader, "CameraPos", graphics.m_Camera->GetPosition());
        //graphics.m_Renderer.SetShaderUniformMat4x4f(graphics.m_TexturedMeshShader, "LightSpaceMatrix", shadowCam.GetCamMatrix());

        scene.SetCamera(&cam);
        scene.Draw(graphics);

        if (draggingModel)
        {
            graphics.Draw(*draggingModel);
        }

        if (movingModelPtr)
        {
            Vec3f movingModelPos = movingModelPtr->GetTransform().GetPosition();

            xAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(4.0f, 0.0f, 0.0f));
            yAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(0.0f, 4.0f, 0.0f));
            zAxisArrow.GetTransform().SetPosition(movingModelPos + Vec3f(0.0f, 0.0f, 4.0f));

            graphics.DebugDrawAABB(collisions.GetCollisionMeshFromMesh(movingModelPtr->m_TexturedMeshes[0].m_Mesh).boundingBox, Vec3f(1.0f, 1.0f, 0.7f), movingModelPtr->GetTransform().GetTransformMatrix());

            graphics.Draw(xAxisArrow);
            graphics.Draw(yAxisArrow);
            graphics.Draw(zAxisArrow);
        }
    }
    graphics.ResetFrameBuffer();


    //graphics.m_Renderer.SetActiveShader(testDepthShader);
    //graphics.m_Renderer.SetActiveFBufferTexture(shadowBuffer);
    //graphics.m_Renderer.DrawMesh(quadMesh);

    Vec2i screen = Engine::GetClientAreaSize();

    ui.BufferPanel(viewportBuffer, newViewport);

    ui.StartFrame(Rect(Vec2f(0.0f, 0.0f), Vec2f(screen.x - 200.0f, 40.0f)), 5.0f, "MEMES");

    {
        if (ui.ImgButton(playButtonTexture, Rect(Vec2f(0.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f))
        {
            state = State::GAME;
            player.velocity = Vec3f(0.0f, 0.0f, 0.0f);
            player.position = cam.GetPosition();
        }
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(100.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(200.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(300.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(400.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(500.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(600.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
        ui.ImgButton(loadedTextures[1], Rect(Vec2f(700.0f, 0.0f), Vec2f(100.0f, 30.0f)), 2.0f);
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

                EditorModel newModel;
                newModel.model = new Model(graphics.CloneModel(loadedModels[i]));
                newModel.colMesh = new CollisionMesh(collisions.GetCollisionMeshFromMesh(newModel.model->m_TexturedMeshes[0].m_Mesh));
            
                draggingModel = new Model(graphics.CloneModel(loadedModels[i]));
            }
        }
    }
    ui.EndTab();

    ui.StartTab("Textures");
    for (int i = 0; i < loadedTextures.size(); ++i)
    {
        if (ui.ImgButton(loadedTextures[i], Rect(Vec2f(i * 40, 0.0f), Vec2f(40, 80)), 2.5f).clicking)
        {
            if (!draggingNewTexture)
            {
                draggingNewTexture = true;
                draggingTexture = loadedTextures[i];
            }
        }
    }
    ui.EndTab();

    ui.StartTab("Scripts");

    ui.EndTab();


    ui.EndFrame();


    ui.StartFrame(Rect(Vec2f(screen.x - 200.0f, 0.0f), Vec2f(200.0f, screen.y)), 20.0f, "Inspector");

    std::string posText;

    if (movingModelPtr)
    {
        Vec3f pos = movingModelPtr->GetTransform().GetPosition();
        std::string xText = "X: " + std::to_string(pos.x);
        std::string yText = "Y: " + std::to_string(pos.y);
        std::string zText = "Z: " + std::to_string(pos.z);

        text.DrawText(xText, &inspectorFont, Vec2f(screen.x - 180.0f, 20.0f), Vec3f(0.0f, 0.0f, 0.0f));
        text.DrawText(yText, &inspectorFont, Vec2f(screen.x - 180.0f, 35.0f), Vec3f(0.0f, 0.0f, 0.0f));
        text.DrawText(zText, &inspectorFont, Vec2f(screen.x - 180.0f, 50.0f), Vec3f(0.0f, 0.0f, 0.0f));

    }
    else
    {
        posText = "No model selected.";
        text.DrawText(posText, &inspectorFont, Vec2f(screen.x - 180.0f, 20.0f), Vec3f(0.0f, 0.0f, 0.0f));
    }


    ui.EndFrame();

    if (ui.ImgButton(cursorToolTexture, Rect(Vec2f(0.0f, 40.0f), Vec2f(100.0f, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::NORMAL;
    }
    if (ui.ImgButton(boxToolTexture, Rect(Vec2f(0.0f, 140.0f), Vec2f(100.0f, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::BOX;
    }

    if (ui.ImgButton(moveToolTexture, Rect(Vec2f(0.0f, 240.0f), Vec2f(100.0f, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::MOVE;
    }

    if (ui.ImgButton(vertexToolTexture, Rect(Vec2f(0.0f, 340.0f), Vec2f(100, 100.0f)), 20.0f))
    {
        toolMode = ToolMode::VERTEX;
    }

    if (input.IsKeyDown(Key::One))
    {
        toolMode = ToolMode::NORMAL;
    }
    if (input.IsKeyDown(Key::Two))
    {
        toolMode = ToolMode::BOX;
    }
    if (input.IsKeyDown(Key::Three))
    {
        toolMode = ToolMode::MOVE;
    }
    if (input.IsKeyDown(Key::Four))
    {
        toolMode = ToolMode::VERTEX;
    }

    std::string modeString;
    switch (toolMode)
    {
    case ToolMode::NORMAL:
        modeString = "Normal mode";
        break;
    case ToolMode::BOX:
        modeString = "Box mode";
        break;
    case ToolMode::MOVE:
        modeString = "Move mode";
        break;
    case ToolMode::VERTEX:
        modeString = "Vertex Edit Mode";
        break;
    }

    text.DrawText(modeString, &testFont, Vec2f(100.0f, 40.0f), Vec3f(0.0f, 0.0f, 0.0f));

    // END DRAW
}

void UpdateGame(ModuleManager& modules)
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

    if (cursorLocked)
    {
        MoveCamera(input, graphics, cam, 0.001f);
    }

    // Update player

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
        inputDir = Math::normalize(inputDir) * 0.1f;
    }

    player.velocity.x = inputDir.x;
    player.velocity.y = inputDir.y;

    player.velocity.z += -0.01f;
    //player.velocity += Vec3f(0.0f, 0.0f, -0.01f);

    //player.velocity = inputDir;
    
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

    if (input.IsKeyDown(Key::Space) && player.grounded)
    {
        player.velocity.z = 0.375f;
    }

    if (movement.rayCastHit.hit && movement.rayCastHit.hitDistance > Math::magnitude(player.velocity))
    {
        graphics.DebugDrawPoint(movement.rayCastHit.hitPoint); 
    }

    //if (movement.rayCastHit.hit && movement.rayCastHit.hitDistance < Math::magnitude(player.velocity))
    //{
    //    graphics.DebugDrawPoint(movement.rayCastHit.hitPoint, Vec3f(1.0f, 0.0f, 0.0f));
    //    player.position = movement.rayCastHit.hitPoint + (Vec3f(0.0f, 0.0f, 0.1f));
    //    //player.velocity = Vec3f(0.0f, 0.0f, 0.0f);
    //    //player.velocity.z = -player.velocity.z;
    //    player.velocity.z = 0.0f;
    //}
    //else
    //{
    //    player.position += player.velocity;
    //}

    player.cam->SetPosition(player.position + Vec3f(0.0f, 0.0f, 2.5f));

    // Drop models

    static Model* droppingModel = nullptr;

    static Vec3f modelSpeed = Vec3f(0.0f, 0.0f, 0.0f);

    if (input.GetMouseState().IsButtonDown(Mouse::LMB))
    {
        SceneRayCastHit modelHit = scene.RayCast(Ray(cam.GetPosition() , cam.GetDirection()), collisions);
        if (modelHit.rayCastHit.hit)
        {
            droppingModel = modelHit.hitModel;
            modelSpeed = Vec3f(0.0f, 0.0f, 0.0f);
        }
    }

    
    

    //if (droppingModel)
    //{
    //    modelSpeed.z = 0.01f;
    //    SceneRayCastHit modelMove = scene.RayCast(Ray(droppingModel->GetTransform().GetPosition(), Math::normalize(modelSpeed)), collisions);


    //    if (modelMove.rayCastHit.hit)
    //    {

    //        hitDist = modelMove.rayCastHit.hitDistance;
    //    }
    //    if (modelMove.rayCastHit.hit && modelMove.rayCastHit.hitDistance <= modelSpeed.z)
    //    {
    //        Vec3f newPos = modelMove.rayCastHit.hitPoint + (modelMove.rayCastHit.hitNormal * 0.01f);
    //        droppingModel->GetTransform().SetPosition(newPos);
    //        modelSpeed = Vec3f(0.0f, 0.0f, 0.0f);
    //        
    //        std::string debugString = "Hit something at " + modelMove.rayCastHit.hitPoint.toString() + ", moving to " + newPos.toString();
    //        Engine::DEBUGPrint(debugString);
    //    }
    //    else
    //    {
    //        droppingModel->GetTransform().Move(modelSpeed);
    //    }

    //}


    graphics.SetActiveFrameBuffer(viewportBuffer);
    {
        scene.SetCamera(&cam);
        scene.Draw(graphics);

    }
    graphics.ResetFrameBuffer();


    Rect screenRect;
    screenRect.size = Engine::GetClientAreaSize();

    ui.BufferPanel(viewportBuffer, screenRect);


    text.DrawText(std::to_string(hitDist), &testFont, Vec2f(0.0f, 0.0f), Vec3f(1.0f, 0.5f, 0.5f));
    text.DrawText(std::to_string(testHitDist), &testFont, Vec2f(0.0f, 24.0f), Vec3f(1.0f, 0.5f, 0.5f));
    text.DrawText(std::to_string(playerVel), &testFont, Vec2f(0.0f, 48.0f), Vec3f(1.0f, 0.5f, 0.5f));
}

void Initialize(ModuleManager& modules)
{

    GraphicsModule& graphics = *modules.GetGraphics();
    CollisionModule& collisions = *modules.GetCollision();
    TextModule& text = *modules.GetText();
    InputModule& input = *modules.GetInput();

    Rect viewportRect = GetViewportSizeFromScreenSize(Engine::GetClientAreaSize());

    previewPSpawn = graphics.CreateModel(TexturedMesh(
        graphics.LoadMesh("models/PlayerSpawn.obj"),
        graphics.LoadTexture("textures/PlayerSpawn.png")
    ));

    xAxisArrow = graphics.CreateModel(TexturedMesh(
        graphics.LoadMesh("models/ArrowSmooth.obj"),
        graphics.LoadTexture("textures/whiteTexture.png")
    ));
    
    yAxisArrow = graphics.CloneModel(xAxisArrow);
    zAxisArrow = graphics.CloneModel(xAxisArrow);

    xAxisArrow.GetTransform().SetScale(Vec3f(0.5f, 0.5f, 0.5f));
    yAxisArrow.GetTransform().SetScale(Vec3f(0.5f, 0.5f, 0.5f));
    zAxisArrow.GetTransform().SetScale(Vec3f(0.5f, 0.5f, 0.5f));

    xAxisArrow.GetTransform().SetRotation(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -M_PI_2));
    zAxisArrow.GetTransform().SetRotation(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), M_PI_2));

    xAxisArrow.SetTexture(graphics.LoadTexture("textures/red.png"));
    yAxisArrow.SetTexture(graphics.LoadTexture("textures/green.png"));
    zAxisArrow.SetTexture(graphics.LoadTexture("textures/blue.png"));

    playButtonTexture = graphics.LoadTexture("images/playButton.png");

    cursorToolTexture = graphics.LoadTexture("images/cursorTool.png");
    boxToolTexture = graphics.LoadTexture("images/boxTool.png");
    moveToolTexture = graphics.LoadTexture("images/moveTool.png");
    vertexToolTexture = graphics.LoadTexture("images/vertexTool.png");

    Vec2i screenSizeI = Engine::GetClientAreaSize();
    cam = Camera(Projection::Perspective);
    cam.SetScreenSize(screenSizeI);
    
    cam.SetPosition(Vec3f(0.0f, -8.0f, 6.0f));
    cam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), 1.3f));

    modelCam = Camera(Projection::Perspective);
    modelCam.SetScreenSize(Vec2f(100.0f, 100.0f));
    modelCam.SetPosition(Vec3f(0.0f, -2.5f, 2.5f));
    modelCam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), 1.3f));

    shadowCam = Camera(Projection::Orthographic);
    shadowCam.SetScreenSize(Vec2f(100.0f, 100.0f));
    shadowCam.SetNearPlane(0.0f);
    shadowCam.SetFarPlane(100.0f);
    shadowCam.SetPosition(Vec3f(0.0f, -30.0f, 6.0f));
    shadowCam.Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), 0.9f));
    shadowCam.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), 1.3f));

    viewportBuffer = graphics.CreateFBuffer(Vec2i(viewportRect.size), FBufferFormat::COLOUR);

    shadowBuffer = graphics.CreateFBuffer(Vec2i(8000, 8000), FBufferFormat::DEPTH);

    std::string shadowVertShader = R"(
    #version 400
    layout (location = 0) in vec3 aPos;

    uniform mat4 lightSpaceMatrix;
    uniform mat4 Transformation;

    void main()
    {
        gl_Position = lightSpaceMatrix * Transformation * vec4(aPos, 1.0);
    }   
    )";

    std::string shadowFragShader = R"(
    #version 400
    
    void main()
    {
    }
    )";

    shadowShader = graphics.m_Renderer.LoadShader(shadowVertShader, shadowFragShader);

    std::string depthVertShader = R"(
    #version 400

    in vec2 VertPosition;
    in vec2 VertUV;

    smooth out vec2 FragUV;

    void main()
    {
        gl_Position = vec4(VertPosition, 0.0, 1.0);
        FragUV = VertUV;    
    }

    )";

    std::string depthFragShader = R"(
    #version 400

    uniform sampler2D DepthMap;
    
    smooth in vec2 FragUV;
    
    out vec4 FragColour;

    void main()
    {             
        float depthValue = texture(DepthMap, FragUV).r;
        FragColour = vec4(vec3(depthValue), 1.0);
        //FragColour = vec4(1.0, 0.0, 0.0, 1.0);
    }  
    )";

    testDepthShader = graphics.m_Renderer.LoadShader(depthVertShader, depthFragShader);

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

    quadMesh = graphics.m_Renderer.LoadMesh(VertexBufferFormat({ VertAttribute::Vec2f, VertAttribute::Vec2f }), quadVertices, quadIndices);

    graphics.InitializeDebugDraw(viewportBuffer);

    graphics.SetCamera(&cam);
    
    graphics.SetRenderMode(RenderMode::FULLBRIGHT);

    tempWhiteTexture = graphics.LoadTexture("images/white.png", TextureMode::NEAREST, TextureMode::NEAREST);

    testFont = text.LoadFont("fonts/ARLRDBD.TTF", 30);
    inspectorFont = text.LoadFont("fonts/ARLRDBD.TTF", 15);

    loadedModels = LoadModels(graphics);
    
    loadedTextures = LoadTextures(graphics);

    Vec2i clientArea = Engine::GetClientAreaSize();

    Vec2i newCenter = Vec2i(viewportRect.location.x + viewportRect.size.x / 2, viewportRect.location.y + viewportRect.size.y / 2);
    //TODO(fraser): clean up mouse constrain/input code
    input.SetMouseCenter(newCenter);
    Engine::SetCursorCenter(newCenter);

    player.cam = &cam;
}

void Update(ModuleManager& modules)
{
    if (state == State::EDITOR)
    {
        UpdateEditor(modules);
    }
    else if (state == State::GAME)
    {
        UpdateGame(modules);
    }
    //if (inputs.mouse.leftMouseButton)
    //{
    //	if (holdingMouseLeft)
    //	{
    //		if (translatingX)
    //		{
    //			Line mouseLine;
    //			mouseLine.point = cam.position - xAxisOffset;
    //			mouseLine.direction = cam.direction;

    //			Line axisLine;
    //			axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			axisLine.direction = Vec3f(1.0f, 0.0f, 0.0f);

    //			Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

    //			editingMesh->SetPosition(pointAlongAxis, &renderer);

    //		}
    //		if (translatingY)
    //		{
    //			Line mouseLine;
    //			mouseLine.point = cam.position - yAxisOffset;
    //			mouseLine.direction = cam.direction;

    //			Line axisLine;
    //			axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			axisLine.direction = Vec3f(0.0f, 1.0f, 0.0f);

    //			Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

    //			editingMesh->SetPosition(pointAlongAxis, &renderer);
    //		}
    //		if (translatingZ)
    //		{
    //			Line mouseLine;
    //			mouseLine.point = cam.position - zAxisOffset;
    //			mouseLine.direction = cam.direction;

    //			Line axisLine;
    //			axisLine.point = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			axisLine.direction = Vec3f(0.0f, 0.0f, 1.0f);

    //			Vec3f pointAlongAxis = Math::ClosestPointsOnLines(mouseLine, axisLine).second;

    //			editingMesh->SetPosition(pointAlongAxis, &renderer);
    //		}
    //		if (rotatingX)
    //		{
    //			// Get intersection between mouse ray and x plane
    //			Plane xPlane;
    //			xPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			xPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);

    //			RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), xPlane);

    //			Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			if (intersection.hit)
    //			{
    //				Vec3f point = intersection.hitPoint - objectCenter;
    //				Vec3f unitVec = Vec3f(0.0f, 0.0f, 1.0f);
    //				float dot = Math::dot(point, unitVec);
    //				float det = Math::dot(xPlane.normal, (Math::cross(point, unitVec)));
    //				float angle = atan2(det, dot);
    //				
    //				float deltaAngle = xAngleLast - angle;

    //				xAngleLast = angle;

    //				editingMesh->RotateMeshAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), deltaAngle, &renderer);
    //				
    //				DebugLineSegment debugLine;
    //				debugLine.a = point + objectCenter;
    //				debugLine.b = objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);

    //				debugLine.a = unitVec + objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);
    //			}
    //		}
    //		if (rotatingY)
    //		{
    //			// Get intersection between mouse ray and y plane
    //			Plane yPlane;
    //			yPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			yPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);

    //			RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), yPlane);

    //			Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			if (intersection.hit)
    //			{
    //				Vec3f point = intersection.hitPoint - objectCenter;
    //				Vec3f unitVec = Vec3f(1.0f, 0.0f, 0.0f);
    //				float dot = Math::dot(point, unitVec);
    //				float det = Math::dot(yPlane.normal, (Math::cross(point, unitVec)));
    //				float angle = atan2(det, dot);

    //				float deltaAngle = yAngleLast - angle;

    //				yAngleLast = angle;

    //				editingMesh->RotateMeshAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), deltaAngle, &renderer);

    //				DebugLineSegment debugLine;
    //				debugLine.a = point + objectCenter;
    //				debugLine.b = objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);

    //				debugLine.a = unitVec + objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);
    //			}
    //		}
    //		if (rotatingZ)
    //		{
    //			// Get intersection between mouse ray and z plane
    //			Plane zPlane;
    //			zPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			zPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);

    //			RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), zPlane);

    //			Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //			if (intersection.hit)
    //			{
    //				Vec3f point = intersection.hitPoint - objectCenter;
    //				Vec3f unitVec = Vec3f(0.0f, 1.0f, 0.0f);
    //				float dot = Math::dot(point, unitVec);
    //				float det = Math::dot(zPlane.normal, (Math::cross(point, unitVec)));
    //				float angle = atan2(det, dot);

    //				float deltaAngle = zAngleLast - angle;

    //				zAngleLast = angle;

    //				editingMesh->RotateMeshAroundAxis(Vec3f(0.0f, 0.0f, 1.0f), deltaAngle, &renderer);
    //			
    //				DebugLineSegment debugLine;
    //				debugLine.a = point + objectCenter;
    //				debugLine.b = objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);

    //				debugLine.a = unitVec + objectCenter;
    //				renderer.DebugDrawLineSegment(debugLine);
    //			}
    //		}
    //	}
    //	else {
    //		holdingMouseLeft = true;

    //		Ray testRay;
    //		testRay.origin = cam.position;
    //		testRay.direction = cam.direction;

    //		EditorRayCastInfo rayCastInfo = editorScene.RayCast(testRay);

    //		if (rayCastInfo.hitMesh)
    //		{
    //			if (rayCastInfo.hitType == HitType::MESH)
    //			{
    //				editorScene.UnselectSelectedEditableMesh();
    //				editorScene.SetEditableMeshSelected(rayCastInfo.hitMesh);

    //				editingMesh = rayCastInfo.hitMesh;
    //			}
    //			if (rayCastInfo.hitType == HitType::TOOL)
    //			{
    //				if (rayCastInfo.toolType == ToolType::X_ARROW)
    //				{
    //					xAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
    //					translatingX = true;
    //				}
    //				if (rayCastInfo.toolType == ToolType::Y_ARROW)
    //				{
    //					yAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
    //					translatingY = true;
    //				}
    //				if (rayCastInfo.toolType == ToolType::Z_ARROW)
    //				{
    //					zAxisOffset = rayCastInfo.hitInfo.hitPoint - renderer.GetMeshPosition(rayCastInfo.hitMesh->m_Mesh);
    //					translatingZ = true;
    //				}
    //				if (rayCastInfo.toolType == ToolType::X_RING)
    //				{
    //					// Get intersection between mouse ray and x plane
    //					Plane xPlane;
    //					xPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					xPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
    //					
    //					RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), xPlane);
    //					
    //					Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					if (intersection.hit)
    //					{
    //						Vec3f point = intersection.hitPoint - objectCenter;
    //						Vec3f unitVec = Vec3f(0.0f, 0.0f, 1.0f);
    //						float dot = Math::dot(point, unitVec);
    //						float det = Math::dot(xPlane.normal, (Math::cross(point, unitVec)));
    //						float angle = atan2(det, dot);
    //						xAngleLast = angle;
    //						rotatingX = true;
    //					}
    //				}

    //				if (rayCastInfo.toolType == ToolType::Y_RING)
    //				{
    //					// Get intersection between mouse ray and y plane
    //					Plane yPlane;
    //					yPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					yPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);

    //					RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), yPlane);

    //					Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					if (intersection.hit)
    //					{
    //						Vec3f point = intersection.hitPoint - objectCenter;
    //						Vec3f unitVec = Vec3f(1.0f, 0.0f, 0.0f);
    //						float dot = Math::dot(point, unitVec);
    //						float det = Math::dot(yPlane.normal, (Math::cross(point, unitVec)));
    //						float angle = atan2(det, dot);
    //						yAngleLast = angle;
    //						rotatingY = true;
    //					}
    //				}
    //				if (rayCastInfo.toolType == ToolType::Z_RING)
    //				{
    //					// Get intersection between mouse ray and z plane
    //					Plane zPlane;
    //					zPlane.center = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					zPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);

    //					RayCastHit intersection = Collisions::RayCast(Ray(cam.position, cam.direction), zPlane);

    //					Vec3f objectCenter = renderer.GetMeshPosition(editingMesh->m_Mesh);
    //					if (intersection.hit)
    //					{
    //						Vec3f point = intersection.hitPoint - objectCenter;
    //						Vec3f unitVec = Vec3f(0.0f, 1.0f, 0.0f);
    //						float dot = Math::dot(point, unitVec);
    //						float det = Math::dot(zPlane.normal, (Math::cross(point, unitVec)));
    //						float angle = atan2(det, dot);
    //						zAngleLast = angle;
    //						rotatingZ = true;
    //					}
    //				}
    //				
    //			}

    //		}
    //	}
    //}
    //else {
    //	translatingX = false;
    //	translatingY = false;
    //	translatingZ = false;

    //	rotatingX = false;
    //	rotatingY = false;
    //	rotatingZ = false;

    //	holdingMouseLeft = false;
    //}

    //if (inputs.keysDown.alt && !holdingAlt)
    //{
    //	Engine::UnlockCursor();
    //	Engine::ShowCursor();
    //	cameraControlEnabled = false;

    //	holdingAlt = true;
    //}
    //else if (!inputs.keysDown.alt && holdingAlt)
    //{
    //	holdingAlt = false;
    //}
    //if (inputs.keysDown.space && !holdingSpace)
    //{
    //	Engine::LockCursor();
    //	Engine::HideCursor();
    //	cameraControlEnabled = true;
    //	
    //	holdingSpace = true;
    //}
    //else if (!inputs.keysDown.space && holdingSpace)
    //{
    //	holdingSpace = false;
    //}

    //if (inputs.keysDown.tab && !holdingTab)
    //{
    //	holdingTab = true;
    //}
    //else if (!inputs.keysDown.tab && holdingTab)
    //{
    //	holdingTab = false;
    //	//editorScene.CycleToolType();
    //}

    //renderer.ClearScreen();

    //editorScene.DrawScene();

    //renderer.SwapBuffer();
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
