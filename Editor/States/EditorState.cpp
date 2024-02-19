#include "EditorState.h"

#include <filesystem>

void EditorState::CursorState::Update(Scene& EditorScene)
{
    InputModule* Input = InputModule::Get();

    if (Dragging == DraggingMode::NewModel)
    {
        if (Input->GetMouseState().IsButtonDown(Mouse::LMB))
        {
        }
        else
        {
            // Mouse button released, place model in scene
            EditorScene.AddModel(*DraggingModelPtr);

            DraggingModelPtr = nullptr;

            Dragging = DraggingMode::None;
        }
    }
}

void EditorState::CursorState::CycleToolMode()
{
    switch (Tool)
    {
    case ToolMode::Select:
        Tool = ToolMode::Geometry;
        break;
    case ToolMode::Geometry:
        Tool = ToolMode::Move;
        break;
    case ToolMode::Move:
        Tool = ToolMode::Vertex;
        break;
    case ToolMode::Vertex:
        Tool = ToolMode::Sculpt;
        break;
    case ToolMode::Sculpt:
        Tool = ToolMode::Select;
        break;
    default:
        Engine::FatalError("Invalid editor cursor tool mode.");
        break;
    }
}

void EditorState::CursorState::CycleGeometryMode()
{
    switch (Geometry)
    {
    case GeometryMode::Box:
        Geometry = GeometryMode::Plane;
        break;
    case GeometryMode::Plane:
        Geometry = GeometryMode::Box;
        break;
    default:
        Engine::FatalError("Invalid editor cursor geometry mode.");
        break;
    }
}

void EditorState::CursorState::CycleMoveMode()
{
    switch (Move)
    {
    case MoveMode::Translate:
        Move = MoveMode::Rotate;
        break;
    case MoveMode::Rotate:
        Move = MoveMode::Scale;
        break;
    case MoveMode::Scale:
        Move = MoveMode::Translate;
        break;
    default:
        Engine::FatalError("Invalid editor cursor move mode.");
        break;
    }
}

void EditorState::CursorState::SetToolMode(ToolMode InToolMode)
{
    Tool = InToolMode;
}

void EditorState::CursorState::StartDraggingNewModel(Model NewModel)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewModel;

    DraggingModelPtr = new Model(NewModel);
}

void EditorState::CursorState::DrawTransientModels()
{

}

void EditorState::CursorState::UpdateSelectTool()
{

}

void EditorState::CursorState::UpdateGeometryTool()
{
}

void EditorState::CursorState::UpdateMoveTool()
{
}

void EditorState::CursorState::UpdateVertexTool()
{
}

void EditorState::CursorState::UpdateSculptTool()
{
}

void EditorState::OnInitialized()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    TextModule* Text = TextModule::Get();
    InputModule* Input = InputModule::Get();

    LoadEditorResources();

    // Load user resources
    LoadedModels = LoadModels(*Graphics);
    LoadedMaterials = LoadMaterials(*Graphics);

    // Create editor viewport camera
    ViewportCamera = Camera(Projection::Perspective);
    ViewportCamera.SetScreenSize(GetEditorSceneViewportRect().size);

    // TODO: Hook up viewport camera to editor "player"? (Might want to allow entities to parented together more generically?)

    // Create model camera
    ModelCamera = Camera(Projection::Perspective);
    ModelCamera.SetScreenSize(Vec2f(100.0f, 100.0f));
    ModelCamera.SetPosition(Vec3f(0.0f, -2.5f, 2.5f));
    ModelCamera.Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), -0.7f));
    
    // Set up empty editor scene
    EditorScene.Init(*Graphics, *Collisions);
    EditorScene.SetDirectionalLight(DirectionalLight{ Vec3f(0.0f, 0.0f, -1.0f), Vec3f(1.0f, 1.0f, 1.0f) });
    EditorScene.SetCamera(&ViewportCamera);

    // Set up framebuffers the editor uses
    Rect ViewportRect = GetEditorSceneViewportRect();
    
    ViewportBuffer = Graphics->CreateGBuffer(ViewportRect.size);
        
    Vec2i NewCenter = Vec2i(ViewportRect.Center());
    //TODO(fraser): clean up mouse constrain/input code
    Input->SetMouseCenter(NewCenter);

    Graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

    CurrentResourceDirectoryPath = std::filesystem::current_path();

    for (int i = 0; i < 100; i++)
    {
        RandomSizes.push_back(Vec2f(Math::RandomFloat(40.0f, 100.0f), Math::RandomFloat(40.0f, 100.0f)));
    }
}

void EditorState::OnUninitialized()
{
}

void EditorState::OnEnter()
{
}

void EditorState::OnExit()
{
}

void EditorState::Update(float DeltaTime)
{
    UpdateEditor(DeltaTime);
}

void EditorState::OnResize()
{
    GraphicsModule* graphics = GraphicsModule::Get();
    InputModule* input = InputModule::Get();

    Rect ViewportRect = GetEditorSceneViewportRect();

    ViewportCamera.SetScreenSize(ViewportRect.size);
    graphics->ResizeGBuffer(ViewportBuffer, ViewportRect.size);
    input->SetMouseCenter(ViewportRect.Center());
}

void EditorState::UpdateEditor(float DeltaTime)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    TextModule* Text = TextModule::Get();
    UIModule* UI = UIModule::Get();
    InputModule* Input = InputModule::Get();

    if (CursorLocked)
    {
        MoveCamera(&ViewportCamera, 0.001f, DeltaTime);
    }

    if (Engine::IsWindowFocused())
    {
        // Update tools
        Cursor.Update(EditorScene);

        if (Input->GetKeyState(Key::Alt).justPressed)
        {
            if (CursorLocked)
            {
                Engine::UnlockCursor();
                Engine::ShowCursor();
                CursorLocked = false;
            }
            else {
                Engine::LockCursor();
                Engine::HideCursor();
                CursorLocked = true;
            }
        }
    }


    Vec2i ViewportSize = Engine::GetClientAreaSize();

    EditorScene.Draw(*Graphics, ViewportBuffer);

    Graphics->ResetFrameBuffer();
    UI->BufferPanel(ViewportBuffer.FinalOutput, GetEditorSceneViewportRect());

    DrawResourcesPanel();
}

void EditorState::UpdateGame(float DeltaTime)
{
}

Rect EditorState::GetEditorSceneViewportRect()
{
    Vec2f ViewportSize = Engine::GetClientAreaSize();

    Rect SceneViewportRect = Rect(  Vec2f(ViewportSize.x * 0.1f, ViewportSize.y * 0.0f), 
                                    Vec2f(ViewportSize.x * 0.8f, ViewportSize.y * 0.7f));
    return SceneViewportRect;
}

Ray EditorState::GetMouseRay(Camera& cam, Vec2i mousePosition, Rect viewPort)
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

void EditorState::LoadEditorResources()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    TextModule* Text = TextModule::Get();

    AssetRegistry* Registry = AssetRegistry::Get();

    // Load textures needed for editor gizmos
    Texture RedTexture = *Registry->LoadTexture("textures/red.png");
    Texture GreenTexture = *Registry->LoadTexture("textures/green.png");
    Texture BlueTexture = *Registry->LoadTexture("textures/blue.png");

    WhiteMaterial = Graphics->CreateMaterial(*Registry->LoadTexture("images/white.png"));

    // Create translation gizmo models
    xAxisArrow = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ArrowSmooth.obj"),
        Graphics->CreateMaterial(RedTexture)
    ));

    yAxisArrow = Graphics->CloneModel(xAxisArrow);
    zAxisArrow = Graphics->CloneModel(xAxisArrow);

    yAxisArrow.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zAxisArrow.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Create rotation gizmo models
    xAxisRing = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/RotationHoop.obj"),
        Graphics->CreateMaterial(RedTexture)
    ));

    yAxisRing = Graphics->CloneModel(xAxisRing);
    zAxisRing = Graphics->CloneModel(xAxisRing);

    yAxisRing.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zAxisRing.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Create scaling gizmo models
    xScaleWidget = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ScaleWidget.obj"),
        Graphics->CreateMaterial(RedTexture)
    ));

    yScaleWidget = Graphics->CloneModel(xScaleWidget);
    zScaleWidget = Graphics->CloneModel(xScaleWidget);

    yScaleWidget.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zScaleWidget.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Load editor UI textures
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

    // Load editor fonts
    DefaultFont = Text->LoadFont("fonts/ARLRDBD.TTF", 30);
    InspectorFont = Text->LoadFont("fonts/ARLRDBD.TTF", 15);
}

std::vector<Model> EditorState::LoadModels(GraphicsModule& graphics)
{
    AssetRegistry* Registry = AssetRegistry::Get();
    
    std::vector<Model> LoadedModels;

    std::string path = "models";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        if (ext.string() == ".obj")
        {
            std::string fileName = entry.path().generic_string();

            StaticMesh newMesh = *Registry->LoadStaticMesh(fileName);
            
            // For now we load all models with a temporary white texture
            Model newModel = graphics.CreateModel(TexturedMesh(newMesh, WhiteMaterial));

            LoadedModels.push_back(newModel);
        }
    }

    return LoadedModels;
}

std::vector<Material> EditorState::LoadMaterials(GraphicsModule& graphics)
{
    AssetRegistry* Registry = AssetRegistry::Get();

    std::vector<Material> LoadedMaterials;

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
                            LoadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal, newRoughness, newMetal, newAO));
                        }
                        else
                        {
                            Texture newNormal = *Registry->LoadTexture(NormalMapString);
                            Texture newRoughness = *Registry->LoadTexture(RoughnessMapString);
                            Texture newMetal = *Registry->LoadTexture(MetallicMapString);
                            LoadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal, newRoughness, newMetal));
                        }
                    }
                    else
                    {
                        Texture newNormal = *Registry->LoadTexture(NormalMapString);
                        Texture newRoughness = *Registry->LoadTexture(RoughnessMapString);
                        LoadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal, newRoughness));
                    }
                }
                else
                {
                    Texture newNormal = *Registry->LoadTexture(NormalMapString);
                    LoadedMaterials.push_back(graphics.CreateMaterial(newTexture, newNormal));
                }
            }
            else
            {
                LoadedMaterials.push_back(graphics.CreateMaterial(newTexture));
            }

        }
    }

    return LoadedMaterials;

}

void EditorState::MoveCamera(Camera* Camera, float PixelToRadians, double DeltaTime)
{
    InputModule* Input = InputModule::Get();

    const float CamSpeed = 10.0f;

    float Speed = CamSpeed * DeltaTime;

    if (Input->IsKeyDown(Key::Shift))
    {
        Speed *= 5.0f;
    }

    if (Input->IsKeyDown(Key::W))
    {
        Camera->Move(Camera->GetDirection() * Speed);
    }
    if (Input->IsKeyDown(Key::S))
    {
        Camera->Move(-Camera->GetDirection() * Speed);
    }
    if (Input->IsKeyDown(Key::D))
    {
        Camera->Move(Math::normalize(Camera->GetPerpVector()) * Speed);
    }
    if (Input->IsKeyDown(Key::A))
    {
        Camera->Move(-Math::normalize(Camera->GetPerpVector()) * Speed);
    }

    if (Input->IsKeyDown(Key::Space))
    {
        Camera->Move(Vec3f(0.0f, 0.0f, Speed));
    }
    if (Input->IsKeyDown(Key::Ctrl))
    {
        Camera->Move(Vec3f(0.0f, 0.0f, -Speed));
    }

    Camera->RotateCamBasedOnDeltaMouse(Input->GetMouseState().GetDeltaMousePos(), PixelToRadians);
}

void EditorState::DrawResourcesPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    // Draw meshes
    Graphics->SetCamera(&ModelCamera);

    Vec2i ScreenSize = Engine::GetClientAreaSize();

    Rect ResourcePanelRect = Rect(Vec2f(ScreenSize.x * 0.1f, ScreenSize.y * 0.7f), Vec2f(ScreenSize.x * 0.8f, ScreenSize.y * 0.3f));

    UI->StartFrame("Resources", ResourcePanelRect, 16.0f);
    {
        UI->StartTab("Models");
        {
            for (auto& AModel : LoadedModels)
            {
                if (UI->TextButton(AModel.m_TexturedMeshes[0].m_Mesh.Path.GetFileNameNoExt(), Vec2f(160.0f, 40.0f), 8.0f).clicking)
                {
                    Cursor.StartDraggingNewModel(AModel);
                }
            }
        }
        UI->EndTab();

        UI->StartTab("Materials");
        {
            for (auto& Material : LoadedMaterials)
            {
                UI->TextButton(Material.m_Albedo.Path.GetFileNameNoExt(), Vec2f(160.0f, 40.0f), 8.0f);
            }
        }
        UI->EndTab();

        UI->StartTab("Other other");
        {
            for (int i = 0; i < 100; ++i)
            {
                UI->TextButton("Button #" + std::to_string(i), RandomSizes[i], 10.0f);
            }
        }
        UI->EndTab();

        //for (int i = 0; i < 10; ++i)
        //{
        //    UI->TextButton("Button #" + std::to_string(i), Rect(Vec2f(0.0f, 0.0f), Vec2f(80.0f, 80.0f)), 4.0f);
        //}
    }
    UI->EndFrame();


}