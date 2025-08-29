#include "EditorState.h"

#include "States/Game/GameState.h"

#include "State/StateRegistry.h"

#include <fstream>
#include <filesystem>
#include <future>
#include <ctime>

static std::filesystem::path CurrentResourceDirectoryPath;

void EditorState::OnInitialized(ArgsList args)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    TextModule* Text = TextModule::Get();
    InputModule* Input = InputModule::Get();

    Engine::SetWindowTitleText(CurrentLevelName);

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
    //EditorScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });
    //EditorScene.SetCamera(&ViewportCamera);
    //EditorScene.SetCamera(&ModelCamera);

    Cursor = CursorState(this, &EditorScene);

    // Set up framebuffers the editor uses
    Rect ViewportRect = GetEditorSceneViewportRect();
    
    ViewportBuffer = Graphics->CreateGBuffer(ViewportRect.size);
    
    WidgetBuffer = Graphics->CreateFBuffer(ViewportRect.size);

    Vec2i NewCenter = Vec2i(ViewportRect.Center());
    //TODO(fraser): clean up mouse constrain/input code
    Input->SetMouseCenter(NewCenter);

    Graphics->InitializeDebugDraw(ViewportBuffer.FinalOutput);

    Graphics->SetRenderMode(RenderMode::DEFAULT);

    CurrentResourceDirectoryPath = std::filesystem::current_path();

    TestFont = TextModule::Get()->LoadFont("Assets/fonts/ARLRDBD.TTF", 30);

    CustomGameStateNames = StateRegistry::Get()->GetStateNames();

    for (int i = 0; i < 1000; i++)
    {
        RandomSizes.push_back(Math::RandomFloat(120.0f, 200.0f));
        Vec3f Colour = Vec3f(Math::RandomFloat(0.0f, 1.0f), Math::RandomFloat(0.0f, 1.0f), Math::RandomFloat(0.0f, 1.0f));
        RandomColours.push_back(Colour);
    }

    he::HalfEdgeMesh* newHeMesh = new he::HalfEdgeMesh();
    newHeMesh->MakeQuad();

    EditorScene.AddHalfEdgeMesh(newHeMesh);


    // Set up entity editor stuff
    EntityCamera = Camera(Projection::Perspective);

    EntityCamera.SetPosition(Vec3f(0.0f, -4.0f, 0.0f));
    //EntityEditorScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });


    // Set up material editor preview stuff
    MaterialPreviewCamera = Camera(Projection::Perspective);

    

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

void EditorState::Update(double DeltaTime)
{
#if 0
    UIModule* UI = UIModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    //Graphics->ResetFrameBuffer();

    UI->StartFrame("Test", Rect(Vec2f(50.0f, 50.0f), Vec2f(600.0f, 400.0f)), 16.0f, c_LightGoldenRodYellow);
    {
        UI->StartFrame("Inner Test", Vec2f(400.0f, 150.0f), 16.0f, c_VegasGold);
        {
            for (int i = 0; i < 1000; ++i)
            {
                UI->TextButton("", Vec2f(20.0f, 20.0f), 4.0f, RandomColours[i]);
            }
        }
        UI->EndFrame();

        for (int i = 0; i < 1000; ++i)
        {
            UI->TextButton("", Vec2f(80.0f, 80.0f), 4.0f, RandomColours[i]);
        }
    }
    UI->EndFrame();

    UI->StartFrame("Test 2", Rect(Vec2f(50.0f, 500.0f), Vec2f(600.0f, 400.0f)), 16.0f, c_DarkOrange);
    {
        for (int i = 0; i < 1000; ++i)
        {
            UI->TextButton("", Vec2f(80.0f, 80.0f), 4.0f, RandomColours[i]);
        }
    }
    UI->EndFrame();

#else    
    UpdateEditor(DeltaTime);
#endif
}

void EditorState::OnResize()
{
    GraphicsModule* graphics = GraphicsModule::Get();
    InputModule* input = InputModule::Get();

    Rect ViewportRect = GetEditorSceneViewportRect();

    ViewportCamera.SetScreenSize(ViewportRect.size);
    graphics->ResizeGBuffer(ViewportBuffer, ViewportRect.size);
    input->SetMouseCenter(ViewportRect.Center());

    EntityCamera.SetScreenSize(ViewportRect.size);
}

void EditorState::UpdateEditor(double DeltaTime)
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

    if (Input->IsKeyDown(Key::Q))
    {
        //EditorScene.SetDirectionalLight(DirectionalLight{ ViewportCamera.GetDirection(), EditorScene.m_DirLight.colour });
        //EntityEditorScene.SetDirectionalLight(DirectionalLight{ EntityCamera.GetDirection(), EntityEditorScene.m_DirLight.colour });
    }

    if (Engine::IsWindowFocused())
    {
        // Update tools
        // TODO(Fraser): This is heavily reliant on order since Update() calls UI functions - another reason to make the UI module use deferred render commands
        Cursor.Update(DeltaTime);
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
    else
    {
        Cursor.ResetAllState();
    }

    if (Input->IsKeyDown(Key::E))
    {
        EditorScene.GetCamera()->SetPosition(ViewportCamera.GetPosition());
        EditorScene.GetCamera()->SetDirection(ViewportCamera.GetDirection());

        //EditorScene.GetCamera()->SetCamMatrix(ViewportCamera.GetInvCamMatrix());
    }


    Vec2i ViewportSize = Engine::GetClientAreaSize();

    //Graphics->ResetFrameBuffer();

    UI->StartFrame("EditorFrame", Rect(Vec2f(0.0f, 0.0f), ViewportSize), 0.0f, c_FrameDark);
    {
        if (UI->StartTab("Level"))
        {
            DrawLevelEditor(Graphics, UI, DeltaTime);
        }
        UI->EndTab();

        if (UI->StartTab("Project"))
        {
            DrawProjectSettingsEditor();
        }
        UI->EndTab();

        if (UI->StartTab("Entities"))
        {
            DrawEntityEditor();
        }
        UI->EndTab();

        if (UI->StartTab("Materials"))
        {

        }
        UI->EndTab();       
    }
    UI->EndFrame();

}

void EditorState::UpdateGame(double DeltaTime)
{
}

Rect EditorState::GetEditorSceneViewportRect()
{
    Vec2f ViewportSize = Engine::GetClientAreaSize();

    Rect SceneViewportRect = Rect(  Vec2f(80.0f, 60.0f), 
                                    Vec2f(ViewportSize.x - 520.0f, ViewportSize.y * 0.7f - 60.0f));
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
    Texture* RedTexture = Registry->LoadTexture("Assets/textures/red.png");
    Texture* GreenTexture = Registry->LoadTexture("Assets/textures/green.png");
    Texture* BlueTexture = Registry->LoadTexture("Assets/textures/blue.png");
    Texture* PurpleTexture = Registry->LoadTexture("Assets/textures/purple.png");

    WhiteMaterial = Graphics->CreateMaterial(Registry->LoadTexture("Assets/images/white.png"));

    // Create translation gizmo models
    xAxisArrow = Graphics->CreateModel(
        *Registry->LoadStaticMesh("Assets/models/ArrowSmooth.obj"),
        Graphics->CreateMaterial(RedTexture)
    );

    yAxisArrow = Graphics->CloneModel(xAxisArrow);
    zAxisArrow = Graphics->CloneModel(xAxisArrow);
    
    xAxisArrow.GetTransform().SetScale(Vec3f(0.2f, 1.0f, 0.2f));
    yAxisArrow.GetTransform().SetScale(Vec3f(0.2f, 1.0f, 0.2f));
    zAxisArrow.GetTransform().SetScale(Vec3f(0.2f, 1.0f, 0.2f));

    xAxisArrow.GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -(float)M_PI_2));
    zAxisArrow.GetTransform().Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), (float)M_PI_2));

    yAxisArrow.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zAxisArrow.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    TranslateBall = Graphics->CreateModel(
        *Registry->LoadStaticMesh("Assets/models/Buckyball.obj"),
        Graphics->CreateMaterial(PurpleTexture)
    );

    // Create rotation gizmo models
    xAxisRing = Graphics->CreateModel(
        *Registry->LoadStaticMesh("Assets/models/RotationHoop.obj"),
        Graphics->CreateMaterial(RedTexture)
    );

    yAxisRing = Graphics->CloneModel(xAxisRing);
    zAxisRing = Graphics->CloneModel(xAxisRing);

    xAxisRing.GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -(float)M_PI_2));
    zAxisRing.GetTransform().Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), (float)M_PI_2));

    yAxisRing.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zAxisRing.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Create scaling gizmo models
    xScaleWidget = Graphics->CreateModel(
        *Registry->LoadStaticMesh("Assets/models/ScaleWidget.obj"),
        Graphics->CreateMaterial(RedTexture)
    );

    yScaleWidget = Graphics->CloneModel(xScaleWidget);
    zScaleWidget = Graphics->CloneModel(xScaleWidget);

    xScaleWidget.GetTransform().Rotate(Quaternion(Vec3f(0.0f, 1.0f, 0.0f), (float)M_PI_2));
    yScaleWidget.GetTransform().Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), -(float)M_PI_2));

    xScaleWidget.GetTransform().SetScale(Vec3f(0.4f, 0.4f, 0.8f));
    yScaleWidget.GetTransform().SetScale(Vec3f(0.4f, 0.4f, 0.8f));
    zScaleWidget.GetTransform().SetScale(Vec3f(0.4f, 0.4f, 0.8f));

    yScaleWidget.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zScaleWidget.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Load editor UI textures
    playButtonTexture = *Registry->LoadTexture("Assets/images/playButton.png");

    genericSelectToolTexture = *Registry->LoadTexture("Assets/images/cursorTool.png");
    vertexSelectToolTexture = *Registry->LoadTexture("Assets/images/vertSelectTool.png");
    faceSelectToolTexture = *Registry->LoadTexture("Assets/images/faceSelectTool.png");

    boxToolTexture = *Registry->LoadTexture("Assets/images/boxTool.png");
    planeToolTexture = *Registry->LoadTexture("Assets/images/planeTool.png");

    translateToolTexture = *Registry->LoadTexture("Assets/images/translateTool.png");
    rotateToolTexture = *Registry->LoadTexture("Assets/images/rotateTool.png");
    scaleToolTexture = *Registry->LoadTexture("Assets/images/scaleTool.png");

    vertexToolTexture = *Registry->LoadTexture("Assets/images/vertexTool.png");
    sculptToolTexture = *Registry->LoadTexture("Assets/images/sculptTool.png");
    
    lightEntityTexture = *Registry->LoadTexture("Assets/images/lightTool.png");
    directionalLightEntityTexture = *Registry->LoadTexture("Assets/images/dirLight.png");
    cameraEntityTexture = *Registry->LoadTexture("Assets/images/cameraButton.png");
    brainEntityTexture = *Registry->LoadTexture("Assets/images/brainTool.png");
    billboardEntityTexture = *Registry->LoadTexture("Assets/images/billboardTool.png");

    // Load editor fonts
    DefaultFont = Text->LoadFont("Assets/fonts/ARLRDBD.TTF", 30);
    InspectorFont = Text->LoadFont("Assets/fonts/ARLRDBD.TTF", 15);
}

std::vector<Model> EditorState::LoadModels(GraphicsModule& graphics)
{
    AssetRegistry* Registry = AssetRegistry::Get();
    
    std::vector<Model> LoadedModels;

    std::string path = "Assets/models";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        if (ext.string() == ".obj")
        {
            std::string fileName = entry.path().generic_string();

            StaticMesh newMesh = *Registry->LoadStaticMesh(fileName);
            
            // For now we load all models with a temporary white texture
            Model newModel = graphics.CreateModel(newMesh, WhiteMaterial);

            LoadedModels.push_back(newModel);
        }
    }

    return LoadedModels;
}

std::vector<Material> EditorState::LoadMaterials(GraphicsModule& graphics)
{
    std::clock_t start;
    double duration;

    start = std::clock();

    AssetRegistry* Registry = AssetRegistry::Get();

    std::vector<Material> LoadedMaterials;

    std::string path = "Assets/textures";

    std::vector<std::future<void>> textureFutures;

    int i = 0;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {


        std::filesystem::path ext = entry.path().extension();
        std::string extensionString = ext.string();
        if (extensionString == ".png" || extensionString == ".jpg")
        {
            if (i > 1)
            {
                break;
            }
            i++;
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


            Material newMaterial = LoadMaterial(entry);

            LoadedMaterials.push_back(newMaterial);
        }
    }

    duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;

    Engine::DEBUGPrint("Took " + std::to_string(duration) + " seconds to load all textures.");

    return LoadedMaterials;

}

Material EditorState::LoadMaterial(std::filesystem::path materialPath)
{
    AssetRegistry* Registry = AssetRegistry::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    Material newMaterial;

    std::filesystem::path ext = materialPath.extension();
    std::string extensionString = ext.string();

    std::string NormalMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".norm" + extensionString;
    std::string RoughnessMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".rough" + extensionString;
    std::string MetallicMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".metal" + extensionString;
    std::string AOMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".ao" + extensionString;

    std::string fileName = materialPath.generic_string();

    Texture* newTexture = Registry->LoadTexture(fileName, true);

    if (std::filesystem::exists(NormalMapString))
    {
        if (std::filesystem::exists(RoughnessMapString))
        {
            if (std::filesystem::exists(MetallicMapString))
            {
                if (std::filesystem::exists(AOMapString))
                {
                    Texture* newNormal = Registry->LoadTexture(NormalMapString, true);
                    Texture* newRoughness = Registry->LoadTexture(RoughnessMapString, true);
                    Texture* newMetal = Registry->LoadTexture(MetallicMapString, true);
                    Texture* newAO = Registry->LoadTexture(AOMapString, true);
                    newMaterial = Graphics->CreateMaterial(newTexture, newNormal, newRoughness, newMetal, newAO);
                }
                else
                {
                    Texture* newNormal = Registry->LoadTexture(NormalMapString, true);
                    Texture* newRoughness = Registry->LoadTexture(RoughnessMapString, true);
                    Texture* newMetal = Registry->LoadTexture(MetallicMapString, true);
                    newMaterial = Graphics->CreateMaterial(newTexture, newNormal, newRoughness, newMetal);
                }
            }
            else
            {
                Texture* newNormal = Registry->LoadTexture(NormalMapString, true);
                Texture* newRoughness = Registry->LoadTexture(RoughnessMapString, true);
                newMaterial = Graphics->CreateMaterial(newTexture, newNormal, newRoughness);
            }
        }
        else
        {
            Texture* newNormal = Registry->LoadTexture(NormalMapString, true);
            newMaterial = Graphics->CreateMaterial(newTexture, newNormal);
        }
    }
    else
    {
        newMaterial = Graphics->CreateMaterial(newTexture);
    }

    return newMaterial;
}

void EditorState::MoveCamera(Camera* Camera, float PixelToRadians, double DeltaTime)
{
    InputModule* Input = InputModule::Get();

    const float CamSpeed = 10.0f;

    float Speed = CamSpeed * (float)DeltaTime;

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

void EditorState::DrawLevelEditor(GraphicsModule* Graphics, UIModule* UI, double DeltaTime)
{
    InputModule* Input = InputModule::Get();

    Cursor.SetScene(&EditorScene);
    Cursor.SetCamera(&ViewportCamera);
    
    Vec2i ViewportSize = Engine::GetClientAreaSize();

    // Keyboard hotkeys for switching tools
    if (Input->GetKeyState(Key::One).justPressed)
    {
        // If already in select tool, cycle sub-tool
        if (Cursor.GetToolMode() == ToolMode::Select)
        {
            Cursor.CycleSelectMode();
        }
        else
        {
            Cursor.SetToolMode(ToolMode::Select);
        }
    }
    if (Input->GetKeyState(Key::Two).justPressed)
    {
        // If already in transform tool, cycle sub-tool
        if (Cursor.GetToolMode() == ToolMode::Transform)
        {
            Cursor.CycleTransformMode();
        }
        else
        {
            Cursor.SetToolMode(ToolMode::Transform);
        }
    }
    if (Input->GetKeyState(Key::Three).justPressed)
    {
        // If already in geometry tool, cycle sub-tool
        if (Cursor.GetToolMode() == ToolMode::Geometry)
        {
            Cursor.CycleGeometryMode();
        }
        else
        {
            Cursor.SetToolMode(ToolMode::Geometry);
        }
    }
    if (Input->GetKeyState(Key::Four).justPressed)
    {
        Cursor.SetToolMode(ToolMode::Sculpt);
    }

    if (Input->GetKeyState(Key::Five).justPressed)
    {
    }

    EditorScene.EditorDraw(*Graphics, ViewportBuffer, &ViewportCamera);
    //EditorScene.Update(DeltaTime);

    Graphics->SetActiveFrameBuffer(WidgetBuffer);
    {
        Graphics->SetCamera(&ViewportCamera);

        Graphics->SetRenderMode(RenderMode::FULLBRIGHT);

        Cursor.DrawTransientModels();
    }
    Graphics->ResetFrameBuffer();

    UI->BufferPanel(ViewportBuffer.FinalOutput, GetEditorSceneViewportRect());

    UI->BufferPanel(WidgetBuffer, GetEditorSceneViewportRect());

    TextModule::Get()->DrawText("Frame Time: " + std::to_string(DeltaTime), &TestFont, GetEditorSceneViewportRect().location);

    PrevFrameTimeCount++;
    PrevFrameTimeSum += DeltaTime;

    if (PrevFrameTimeSum > 0.5f)
    {
        PrevAveFPS = (int)round(1.0f / (PrevFrameTimeSum / PrevFrameTimeCount));
        PrevFrameTimeCount = 0;
        PrevFrameTimeSum -= 0.5f;
    }

    TextModule::Get()->DrawText("FPS: " + std::to_string(PrevAveFPS), &TestFont, GetEditorSceneViewportRect().location + Vec2f(0.0f, 30.0f));

    DrawEditorUI();
}

void EditorState::DrawProjectSettingsEditor()
{
    UIModule* UI = UIModule::Get();
    
    UI->StartFrame("Project Build Settings", Vec2f(420.0f, 400.0f), 20.0f, c_NiceLighterBlue);


    std::string GameTypeButtonString;
    if (CurrentGameType == GameType::SINGLEPLAYER) GameTypeButtonString = "Singleplayer";
    else if (CurrentGameType == GameType::MULTIPLAYER) GameTypeButtonString = "Multiplayer";

    if (UI->TextButton(GameTypeButtonString, Vec2f(380.0f, 60.0f), 10.0f, c_Button))
    {
        if (CurrentGameType == GameType::SINGLEPLAYER) CurrentGameType = GameType::MULTIPLAYER;
        else if (CurrentGameType == GameType::MULTIPLAYER) CurrentGameType = GameType::SINGLEPLAYER;
    }
    UI->NewLine(40.0f);

    std::string GameStateUsingString = "Using custom game state: ";
    if (UsingCustomGameState) GameStateUsingString += " True";
    else GameStateUsingString += " False";

    if (UI->TextButton(GameStateUsingString, Vec2f(UsingCustomGameState ? 190.0f : 380.0f, 60.0f), 10.0f, c_Button))
    {
        UsingCustomGameState = !UsingCustomGameState;
    }

    if (UsingCustomGameState)
    {
        if (UI->TextButton(CustomGameStateNames[CurrentCustomStateIndex], Vec2f(190.0f, 60.0f), 10.0f, c_NiceBrightGreen))
        {
            CurrentCustomStateIndex++;
            if (CurrentCustomStateIndex >= CustomGameStateNames.size())
            {
                CurrentCustomStateIndex = 0;
            }
        }
    }

    UI->NewLine(40.0f);

    if (UI->TextButton("Build", Vec2f(380.0f, 60.0f), 10.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
    {
        std::string BuildCommand = "BuildGame.bat " + CurrentLevelName + " " + GameTypeButtonString;
        
        if (UsingCustomGameState) BuildCommand += " " + CustomGameStateNames[CurrentCustomStateIndex];

        Engine::RunCommand(BuildCommand);
    }
    UI->EndFrame();
}

void EditorState::DrawEntityEditor()
{
    Cursor.SetScene(&EntityEditorScene);
    Cursor.SetCamera(&EntityCamera);

    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();
    InputModule* Input = InputModule::Get();

    //EntityEditorScene.SetDirectionalLight(DirectionalLight{ EntityCamera.GetDirection(), EntityEditorScene.m_DirLight.colour });

    // Keyboard hotkeys for switching tools
    if (Input->GetKeyState(Key::One).justPressed)
    {
        // If already in select tool, cycle sub-tool
        if (Cursor.GetToolMode() == ToolMode::Select)
        {
            Cursor.CycleSelectMode();
        }
        else
        {
            Cursor.SetToolMode(ToolMode::Select);
        }
    }
    if (Input->GetKeyState(Key::Two).justPressed)
    {
        // If already in transform tool, cycle sub-tool
        if (Cursor.GetToolMode() == ToolMode::Transform)
        {
            Cursor.CycleTransformMode();
        }
        else
        {
            Cursor.SetToolMode(ToolMode::Transform);
        }
    }

    //EditorScene.EditorDraw(*Graphics, ViewportBuffer, &ViewportCamera);
    //

    //// Do render commands

    //DirectionalLight EntityLight;
    //EntityLight.colour = MakeColour(255, 255, 255);
    //EntityLight.direction = Vec3f(0.0f, 0.0f, -1.0f);

    ////BillboardRenderCommand billboardRC;
    ////billboardRC.m_Position = Vec3f(0.0f, 0.0f, 0.0f);
    ////billboardRC.m_Colour = Vec3f(1.0f, 1.0f, 1.0f);
    ////billboardRC.m_Size = 1.0f;
    ////billboardRC.m_Texture = WhiteMaterial.m_Albedo.Id;

    ////Graphics->AddRenderCommand(billboardRC);

    Rect ViewportRect = GetEditorSceneViewportRect();

    // Left toolbar buttons
    Rect ToolbarButtonRect = Rect(Vec2f(0.0f, 60.0f), Vec2f(ViewportRect.location.x, ViewportRect.size.y));

    if (UI->StartFrame("Tools", ToolbarButtonRect, 0.0f, c_FrameLight))
    {
        Vec3f SelectedColour = c_SelectedButton;
        Vec3f UnSelectedColour = c_Button;

        Texture SelectModeTexture = genericSelectToolTexture;
        switch (Cursor.GetSelectMode())
        {
        case SelectMode::GenericSelect:
            SelectModeTexture = genericSelectToolTexture;
            break;
        case SelectMode::FaceSelect:
            SelectModeTexture = faceSelectToolTexture;
            break;
        case SelectMode::VertSelect:
            SelectModeTexture = vertexSelectToolTexture;
            break;
        default:
            break;
        }

        if (UI->ImgButton("SelectTool", SelectModeTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Select ? SelectedColour : UnSelectedColour))
        {
            if (Cursor.GetToolMode() == ToolMode::Select)
            {
                Cursor.CycleSelectMode();
            }
            else
            {
                Cursor.SetToolMode(ToolMode::Select);
            }
        }

        Texture TransModeTexture = translateToolTexture;
        switch (Cursor.GetTransMode())
        {
        case TransformMode::Translate:
            TransModeTexture = translateToolTexture;
            break;
        case TransformMode::Rotate:
            TransModeTexture = rotateToolTexture;
            break;
        case TransformMode::Scale:
            TransModeTexture = scaleToolTexture;
            break;
        default:
            break;
        }

        if (UI->ImgButton("TransformTool", TransModeTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Transform ? SelectedColour : UnSelectedColour))
        {
            if (Cursor.GetToolMode() == ToolMode::Transform)
            {
                Cursor.CycleTransformMode();
            }
            else
            {
                Cursor.SetToolMode(ToolMode::Transform);
            }
        }
    }
    UI->EndFrame();


    // Begin entity camera stuff
    int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();

    EntityCamDistance -= ((float)DeltaMouseWheel * 0.25f);
    EntityCamDistance = Math::Clamp(EntityCamDistance, 0.5f, 10.0f);

    if (Input->GetMouseState().GetMouseButtonState(MouseButton::RMB).pressed)
    {
        Vec2f DeltaMouse = Input->GetMouseState().GetDeltaMousePos();

        EntityCamXAxis -= DeltaMouse.x * 0.005f;
        EntityCamYAxis -= DeltaMouse.y * 0.005f;

        EntityCamYAxis = Math::ClampRadians(EntityCamYAxis, -M_PI_2 + 0.001f, M_PI_2 - 0.001f);
    }

    Quaternion Rotation = Quaternion::FromEuler(EntityCamYAxis, 0.0f, EntityCamXAxis);
    Vec3f NegDistance = Vec3f(0.0f, -EntityCamDistance, 0.0f);

    Vec3f NewCamPos = (NegDistance * Rotation) + EntityCamCenterPoint;
    Vec3f NewCamDir = (EntityCamCenterPoint - NewCamPos).GetNormalized();

    EntityCamera.SetPosition(NewCamPos);
    EntityCamera.SetDirection(NewCamDir);

    // End entity camera stuff

    // Render entity in viewport
    // TEMP (draw a ball)
    //StaticMeshRenderCommand command;
    //command.m_Material = TranslateBall.m_TexturedMeshes[0].m_Material;
    //command.m_Mesh = TranslateBall.m_TexturedMeshes[0].m_Mesh.Id;
    //command.m_TransMat = Mat4x4f();

    //Graphics->AddRenderCommand(command);

    //Graphics->Render(ViewportBuffer, EntityCamera, EntityLight);

    EntityEditorScene.EditorDraw(*Graphics, ViewportBuffer, &EntityCamera, false);

    Rect SceneViewportRect = GetEditorSceneViewportRect();
    Graphics->SetActiveFrameBuffer(WidgetBuffer);
    {
        Graphics->SetCamera(&EntityCamera);

        Graphics->SetRenderMode(RenderMode::FULLBRIGHT);

        Cursor.DrawTransientModels();
    }
    Graphics->ResetFrameBuffer();

    UI->BufferPanel(ViewportBuffer.FinalOutput, SceneViewportRect);
    UI->BufferPanel(WidgetBuffer, SceneViewportRect);


    Vec2f ViewportSize = Engine::GetClientAreaSize();
    
    // Save/Load/etc. toolbar
    if (UI->TextButton("New", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
    {

    }
    if (UI->TextButton("Open", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
    {

    }
    if (UI->TextButton("Save", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
    {
        std::string EntityName;
        if (Engine::FileSaveDialog(EntityName, "Save Entity", "Entity", "ntt"))
        {
            SaveEntity(EntityName);
        }
    }

    // Entity inspector panel
    Rect InspectorPanelRect = Rect(Vec2f(SceneViewportRect.location.x + SceneViewportRect.size.x, SceneViewportRect.location.y),
        Vec2f(ViewportSize.x - (SceneViewportRect.location.x + SceneViewportRect.size.x), ViewportSize.y - SceneViewportRect.location.y));

    UI->StartFrame("Entity Inspector", InspectorPanelRect, 12.0f, c_NiceLighterBlue);
    {
        DrawEntityInspectorPanel();
    }
    UI->EndFrame();

    // Entity resource drawer stuff
    Vec2i ScreenSize = Engine::GetClientAreaSize();

    Rect ResourcePanelRect = Rect(Vec2f(ViewportRect.location.x, ViewportRect.location.y + ViewportRect.size.y), Vec2f(ViewportRect.size.x, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y)));
    UI->StartFrame("Resources", ResourcePanelRect, 16.0f, c_FrameDark);
    {
        if (UI->StartTab("Models", c_Tab))
        {
            for (auto& AModel : LoadedModels)
            {
                if (UI->TextButton(AModel.m_StaticMesh.Path.GetFileNameNoExt(), Vec2f(120.0f, 40.0f), 10.0f, c_ResourceButton))
                {
                    EntityEditorScene.AddModel(new Model(AModel));
                }
            }
        }
        UI->EndTab();
        
        if (UI->StartTab("Materials", c_Tab))
        {
            for (auto& Mat : LoadedMaterials)
            {
                if (UI->ImgButton(Mat.m_Albedo->Path.GetFileNameNoExt(), *Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_ResourceButton).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        Material* MatPtr = &Mat;
                        Cursor.StartDraggingNewMaterial(MatPtr);
                    }

                }
            }
        }
        UI->EndTab();
    
        if (UI->StartTab("Objects", c_Tab))
        {
            if (UI->ImgButton("LightEntity", lightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
            {
                EntityEditorScene.AddPointLight(PointLight());
            }
            if (UI->ImgButton("DirectionalLightEntity", directionalLightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
            {

            }
            if (UI->ImgButton("CameraEntity", cameraEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
            {
            }
            if (UI->ImgButton("BrainEntity", brainEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
            {
            }
            if (UI->ImgButton("BillboardEntity", billboardEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
            {
            }
        }
        UI->EndTab();
    }
    UI->EndFrame();

    // End Entity resource drawer stuff

    //DrawResourcesPanel(EntityEditorScene);
}

void EditorState::SaveEntity(std::string EntityName)
{
    std::ofstream File(EntityName, std::ofstream::out | std::ofstream::trunc);
    
    if (!File.is_open())
    {
        Engine::DEBUGPrint("Failed to save entity :(");
        return;
    }

    json EntityJson;

    auto& SceneModelMap = EntityEditorScene.m_Models;

    json ModelsList;

    for (auto& it : SceneModelMap)
    {
        Model& Mod = *it.second;
        
        json ModelJson;

        ModelJson["Mesh"] = Mod.m_StaticMesh.Path.GetFullPath();

        Mat4x4f ModTrans = Mod.GetTransform().GetTransformMatrix();
        ModelJson["OffsetTrans"] = {
            ModTrans[0].x, ModTrans[0].y, ModTrans[0].z, ModTrans[0].w,
            ModTrans[1].x, ModTrans[1].y, ModTrans[1].z, ModTrans[1].w,
            ModTrans[2].x, ModTrans[2].y, ModTrans[2].z, ModTrans[2].w,
            ModTrans[3].x, ModTrans[3].y, ModTrans[3].z, ModTrans[3].w,
        };

        ModelsList.push_back(ModelJson);
    }

    EntityJson["Models"] = ModelsList;

    // Uncomment to beautify json - makes it easier to debug but makes files much larger
    File << std::setw(4) << EntityJson;

    // Uncomment to normalness
    //File << EntityJson;
}

void EditorState::DrawMaterialEditor()
{



}

void EditorState::DrawEditorUI()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    // Left toolbar buttons
    Rect ToolbarButtonRect = Rect(Vec2f(0.0f, 60.0f), Vec2f(ViewportRect.location.x, ViewportRect.size.y));

    if (UI->StartFrame("Tools", ToolbarButtonRect, 0.0f, c_FrameLight))
    {
        Vec3f SelectedColour = c_SelectedButton;
        Vec3f UnSelectedColour = c_Button;

        Texture SelectModeTexture = genericSelectToolTexture;
        switch (Cursor.GetSelectMode())
        {
        case SelectMode::GenericSelect:
            SelectModeTexture = genericSelectToolTexture;
            break;
        case SelectMode::FaceSelect:
            SelectModeTexture = faceSelectToolTexture;
            break;
        case SelectMode::VertSelect:
            SelectModeTexture = vertexSelectToolTexture;
            break;
        default:
            break;
        }

        if (UI->ImgButton("SelectTool", SelectModeTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Select ? SelectedColour : UnSelectedColour))
        {
            if (Cursor.GetToolMode() == ToolMode::Select)
            {
                Cursor.CycleSelectMode();
            }
            else
            {
                Cursor.SetToolMode(ToolMode::Select);
            }
        }

        Texture TransModeTexture = translateToolTexture;
        switch (Cursor.GetTransMode())
        {
        case TransformMode::Translate:
            TransModeTexture = translateToolTexture;
            break;
        case TransformMode::Rotate:
            TransModeTexture = rotateToolTexture;
            break;
        case TransformMode::Scale:
            TransModeTexture = scaleToolTexture;
            break;
        default:
            break;
        }

        if (UI->ImgButton("TransformTool", TransModeTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Transform ? SelectedColour : UnSelectedColour))
        {
            if (Cursor.GetToolMode() == ToolMode::Transform)
            {
                Cursor.CycleTransformMode();
            }
            else
            {
                Cursor.SetToolMode(ToolMode::Transform);
            }
        }

        Texture GeoModeTexture = boxToolTexture;
        switch (Cursor.GetGeoMode())
        {
        case GeometryMode::Box:
            GeoModeTexture = boxToolTexture;
            break;
        case GeometryMode::Plane:
            GeoModeTexture = planeToolTexture;
            break;
        case GeometryMode::HalfEdge:
            GeoModeTexture = playButtonTexture;
            break;
        default:
            break;
        }

        if (UI->ImgButton("GeometryTool", GeoModeTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Geometry ? SelectedColour : UnSelectedColour))
        {
            if (Cursor.GetToolMode() == ToolMode::Geometry)
            {
                Cursor.CycleGeometryMode();
            }
            else
            {
                Cursor.SetToolMode(ToolMode::Geometry);
            }
        }

        if (UI->ImgButton("SculptTool", sculptToolTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Sculpt ? SelectedColour : UnSelectedColour))
        {
            Cursor.SetToolMode(ToolMode::Sculpt);
        }

    }
    UI->EndFrame();

    DrawTopPanel();
    DrawDrawerSettingsPanel();
    DrawResourcesPanel(EditorScene);
    DrawInspectorPanel();
}

void EditorState::DrawTopPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect TopPanelRect = Rect(Vec2f(0.0f, 20.0f),
        Vec2f((float)ScreenSize.x, ViewportRect.location.y - 20.0f));

    //Vec2f TopPanelButtonSize = Vec2f(ViewportRect.location.y, ViewportRect.location.y);

    UI->StartFrame("Top", TopPanelRect, 0.0f, c_FrameDark);
    {
        if (UI->ImgButton("PlayButton", playButtonTexture, Vec2f(40.0f, 40.0f), 8.0f, c_TopButton))
        {
            Cursor.UnselectAll();
            Cursor.ResetAllState();
            GameState* NewGameState = new GameState();
            
            //NewGameState->LoadScene(CurrentLevelName);
            NewGameState->LoadScene(EditorScene);
            //NewGameState->LoadScene();

            Machine->PushState(NewGameState);
        }
        if (UI->TextButton("New", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
        {
            Cursor.UnselectAll();
            EditorScene.Clear();
            BehaviourRegistry::Get()->ClearAllAttachedBehaviours();
        }
        if (UI->TextButton("Open", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
        {
            Cursor.UnselectAll();
            std::string FileName;
            if (Engine::FileOpenDialog(FileName))
            {
                std::filesystem::path LevelPath = FileName;
                EditorScene.Load(FileName);
            
                std::filesystem::path CurrentDir = std::filesystem::current_path();

                LevelPath = std::filesystem::relative(LevelPath, CurrentDir);

                CurrentLevelName = LevelPath.string();

                std::replace(CurrentLevelName.begin(), CurrentLevelName.end(), '\\', '/');

                Engine::SetWindowTitleText(CurrentLevelName);
            }
        }
        if (UI->TextButton("Save", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
        {
            std::string FileName;
            if (Engine::FileSaveDialog(FileName, "Save Level", "Level", "lvl"))
            {
                EditorScene.Save(FileName);
            }
        }

        NetworkModule* Network = NetworkModule::Get();
        
        static std::string ipString = "";

        UI->TextEntry("IP", ipString, Vec2f(240.0f, 40.0f));

        if (UI->TextButton("Host", Vec2f(80.0f, 40.0f), 8.0f))
        {
            Network->StartServer();
        }

        if (UI->TextButton("Connect", Vec2f(80.0f, 40.0f), 8.0f))
        {
            Network->StartClient(ipString);
        }

        if (UI->TextButton("Ping", Vec2f(80.0f, 40.0f), 8.0f))
        {
            Network->ClientPing();
            Network->ServerPing();
        }
    }
    UI->EndFrame();
}

void EditorState::DrawDrawerSettingsPanel()
{
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect ModeSelectRect = Rect(
        Vec2f(0.0f, ViewportRect.location.y + ViewportRect.size.y), 
        Vec2f(ViewportRect.location.x, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y))
    );

    UI->StartFrame("Mode Select", ModeSelectRect, 6.0f, c_FrameLight);
    {
        if (UI->TextButton("Content", Vec2f(ModeSelectRect.size.x - 16.0f, 40.0f), 6.0f))
        {
            Drawer = DrawerMode::CONTENT;
        }
        if (UI->TextButton("Browser", Vec2f(ModeSelectRect.size.x - 16.0f, 40.0f), 6.0f))
        {
            Drawer = DrawerMode::BROWSER;
        }

    }
    UI->EndFrame();
}

void EditorState::DrawResourcesPanel(Scene& FocusedScene)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    // Draw meshes
    Graphics->SetCamera(&ModelCamera);

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect ResourcePanelRect = Rect(Vec2f(ViewportRect.location.x, ViewportRect.location.y + ViewportRect.size.y), Vec2f(ViewportRect.size.x, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y)));

    if (Drawer == DrawerMode::CONTENT)
    {
        UI->StartFrame("Resources", ResourcePanelRect, 16.0f, c_FrameDark);
        {
            UI->StartTab("Models", c_Tab);
            {
                int index = 0;
                for (auto& AModel : LoadedModels)
                {
                    if (UI->TextButton(AModel.m_StaticMesh.Path.GetFileNameNoExt(), Vec2f(120.0f, 40.0f), 10.0f, c_ResourceButton).clicking)
                    {
                        if (!Cursor.IsDraggingSomething())
                        {
                            Model* AddedModel = FocusedScene.AddModel(new Model(AModel));
                            Cursor.StartDraggingNewModel(AddedModel);
                        }
                    }
                    index++;
                }
            }
            UI->EndTab();

            UI->StartTab("Materials", c_Tab);
            {
                for (auto& Mat : LoadedMaterials)
                {
                    if (UI->ImgButton(Mat.m_Albedo->Path.GetFileNameNoExt(), *Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_ResourceButton).clicking)
                    {
                        lastUsedMaterial = &Mat;
                        if (!Cursor.IsDraggingSomething())
                        {
                            Material* MatPtr = &Mat;
                            Cursor.StartDraggingNewMaterial(MatPtr);
                        }

                    }
                }

                if (lastUsedMaterial)
                {
                    UI->ImgButton(lastUsedMaterial->m_Albedo->Path.GetFileNameNoExt(), *lastUsedMaterial->m_Albedo, Vec2f(40, 40), 0.0f);
                    UI->ImgButton(lastUsedMaterial->m_Normal->Path.GetFileNameNoExt(), *lastUsedMaterial->m_Normal, Vec2f(40, 40), 0.0f);
                    UI->ImgButton(lastUsedMaterial->m_Roughness->Path.GetFileNameNoExt(), *lastUsedMaterial->m_Roughness, Vec2f(40, 40), 0.0f);
                    UI->ImgButton(lastUsedMaterial->m_Metallic->Path.GetFileNameNoExt(), *lastUsedMaterial->m_Metallic, Vec2f(40, 40), 0.0f);
                    UI->ImgButton(lastUsedMaterial->m_AO->Path.GetFileNameNoExt(), *lastUsedMaterial->m_AO, Vec2f(40, 40), 0.0f);
                }
            }
            UI->EndTab();

            UI->StartTab("Objects", c_Tab);
            {
                if (UI->ImgButton("LightEntity", lightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        PointLight NewLight = PointLight();

                        PointLight* PointLightPtr = FocusedScene.AddPointLight(NewLight);
                        Cursor.StartDraggingNewPointLight(PointLightPtr);
                    }
                }
                if (UI->ImgButton("DirectionalLightEntity", directionalLightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        DirectionalLight NewDirLight = DirectionalLight();
                        DirectionalLight* DirLightPtr = FocusedScene.AddDirectionalLight(NewDirLight);
                        Cursor.StartDraggingNewDirectionalLight(DirLightPtr);
                    }
                }
                if (UI->ImgButton("CameraEntity", cameraEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
                {
                }
                if (UI->ImgButton("BrainEntity", brainEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
                {
                }
                if (UI->ImgButton("BillboardEntity", billboardEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton))
                {
                }
            }
            UI->EndTab();

            if (UI->StartTab("Entities", c_Tab))
            {

            }
            UI->EndTab();

            if (UI->StartTab("Behaviours", c_Tab))
            {
                auto BehaviourMap = BehaviourRegistry::Get()->GetBehaviours();

                for (auto Behaviour : BehaviourMap)
                {
                    if (UI->TextButton(Behaviour.first, Vec2f(120, 40), 10.0f, c_ResourceButton).clicking)
                    {
                        if (!Cursor.IsDraggingSomething())
                        {
                            Cursor.StartDraggingNewBehaviour(Behaviour.first);
                        }
                    }
                }
            }
            UI->EndTab();

            if (UI->StartTab("1000 Buttons", c_Tab))
            {
                for (int i = 0; i < 1000; ++i)
                {
                    UI->TextButton("", Vec2f(80.0f, 80.0f), 4.0f, RandomColours[i]);
                }
            }
            UI->EndTab();
        }
        UI->EndFrame();
    }
    else if (Drawer == DrawerMode::BROWSER)
    {
        UI->StartFrame("Browser", ResourcePanelRect, 16.0f, c_FrameDark);
        {
            if (UI->TextButton("..", Vec2f(120.0f, 40.0f), 8.0f, c_NiceYellow))
            {
                CurrentResourceDirectoryPath = CurrentResourceDirectoryPath.parent_path();
            }

            for (const auto& entry : std::filesystem::directory_iterator(CurrentResourceDirectoryPath))
            {
                if (entry.is_directory())
                {
                    if (UI->TextButton(entry.path().filename().generic_string(), Vec2f(120.0f, 80.0f), 8.0f, c_FrameLight))
                    {
                        CurrentResourceDirectoryPath = entry.path();
                        break;
                    }
                }
                else
                {
                    if (UI->TextButton(entry.path().filename().generic_string(), Vec2f(120.0f, 80.0f), 8.0f, c_ResourceButton))
                    {

                    }
                }
            }
        }
        UI->EndFrame();
    }
}

void EditorState::DrawInspectorPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect InspectorPanelRect = Rect( Vec2f(ViewportRect.location.x + ViewportRect.size.x, ViewportRect.location.y), 
                                    Vec2f(ScreenSize.x - (ViewportRect.location.x + ViewportRect.size.x), ScreenSize.y - ViewportRect.location.y));

    UI->StartFrame("Settings", InspectorPanelRect, 16.0f, c_Inspector);
    {
        if (UI->StartTab("Inspector"))
        {
            Cursor.DrawInspectorPanel();
        }
        UI->EndTab();

        if (UI->StartTab("Scene"))
        {
            EditorScene.DrawSettingsPanel();
        }
        UI->EndTab();

    }
    UI->EndFrame();

}

void EditorState::DrawEntityInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    UI->TextButton("Add Component", Vec2f(120.0f, 60.0f), 12.0f, c_NiceYellow, c_FrameDark);
}
