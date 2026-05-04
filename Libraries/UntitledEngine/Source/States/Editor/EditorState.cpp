#include "EditorState.h"

#ifdef USE_EDITOR

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
    LoadedHotspotTextures = LoadHotspotTextures(*Graphics);

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


    // Set up entity editor stuff
    EntityCamera = Camera(Projection::Perspective);

    EntityCamera.SetPosition(Vec3f(0.0f, -4.0f, 0.0f));
    //EntityEditorScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });


    // Set up material editor preview stuff
    MaterialPreviewCamera = Camera(Projection::Perspective);

    // Check if there's an editor config file, if so load settings from it
    std::filesystem::path EditorConfigPath = "EditorConfig";
    if (std::filesystem::exists(EditorConfigPath))
    {
        std::ifstream EditorConfigFile(EditorConfigPath);
        if (EditorConfigFile.is_open())
        {
            EditorConfigFile >> EditorConfig;

            if (EditorConfig.contains("LastOpenLevel"))
            {
                CurrentLevelName = EditorConfig["LastOpenLevel"].get<std::string>();
                EditorScene.Load(CurrentLevelName);
            }
            if (EditorConfig.contains("LastEditorCameraPos"))
            {
                std::vector<float> LastCameraPos = EditorConfig["LastEditorCameraPos"].get<std::vector<float>>();
                if (LastCameraPos.size() == 3)
                {
                    ViewportCamera.SetPosition(Vec3f(LastCameraPos[0], LastCameraPos[1], LastCameraPos[2]));
                }
            }
            if (EditorConfig.contains("LastEditorCameraDir"))
            {
                std::vector<float> LastCameraDir = EditorConfig["LastEditorCameraDir"].get<std::vector<float>>();
                if (LastCameraDir.size() == 3)
                {
                    ViewportCamera.SetDirection(Vec3f(LastCameraDir[0], LastCameraDir[1], LastCameraDir[2]));
                }
            }
        }
    }
}

void EditorState::OnUninitialized()
{
    Engine::DEBUGPrint("EditorState Uninitialized");
    // Save editor config file
    std::filesystem::path EditorConfigPath = "EditorConfig";
    std::ofstream EditorConfigFile(EditorConfigPath);
    if (EditorConfigFile.is_open())
    {
        EditorConfig["LastOpenLevel"] = CurrentLevelName;
        EditorConfig["LastEditorCameraPos"] = { ViewportCamera.GetPosition().x, ViewportCamera.GetPosition().y, ViewportCamera.GetPosition().z };
        EditorConfig["LastEditorCameraDir"] = { ViewportCamera.GetDirection().x, ViewportCamera.GetDirection().y, ViewportCamera.GetDirection().z };
        EditorConfigFile << EditorConfig.dump(4);
    }
}

void EditorState::OnEnter()
{
    Engine::UnlockCursor();
    Engine::ShowCursor();
}

void EditorState::OnExit()
{
    Engine::DEBUGPrint("EditorState Exited");
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

    if (Input->IsKeyDown(Key::Q))
    {
        EditorScene.GetCamera()->SetPosition(ViewportCamera.GetPosition());
        EditorScene.GetCamera()->SetDirection(ViewportCamera.GetDirection());

        //EditorScene.GetCamera()->SetCamMatrix(ViewportCamera.GetInvCamMatrix());
    }

    Vec2i ViewportSize = Engine::GetClientAreaSize();

    //Graphics->ResetFrameBuffer();

    UI->StartFrame("EditorFrame", PlacementSettings(PlacementType::RECT_ABSOLUTE, Rect(Vec2f(0.0f, 0.0f), ViewportSize)), 0.0f, c_FrameDark);
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
            DrawMaterialEditor();
        }
        UI->EndTab();       

        if (UI->StartTab("HotSpot Materials"))
        {
            DrawHotSpotMaterialEditor();
        }
        UI->EndTab();

        if (UI->StartTab("+"))
        {
            DrawNewTabScreen();
        }
        UI->EndTab();

        for (int i = ResourceTabs.size() - 1; i >= 0; i--)
        {
            ResourceTab tab = ResourceTabs[i];
            if (UI->StartTab(tab.ResourcePath.GetFileName()))
            {
                if (UI->TextButton("Close", PlacementSettings(Vec2f(80.0f, 40.0f)), 8.0f, c_NiceRed))
                {
                    ResourceTabs.erase(ResourceTabs.begin() + i);
                    continue;
                }
                
                UI->NewLine();

                DrawResourceTab(tab);
            }
            UI->EndTab();
        }
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

    ScaleRing = Graphics->CreateModel(
        *Registry->LoadStaticMesh("Assets/models/ScaleRing.obj"),
        Graphics->CreateMaterial(PurpleTexture)
    );

    // Load editor UI textures
    playButtonTexture = *Registry->LoadTexture("Assets/images/playButton.png");

    genericSelectToolTexture = *Registry->LoadTexture("Assets/images/cursorTool.png");
    vertexSelectToolTexture = *Registry->LoadTexture("Assets/images/vertSelectTool.png");
    faceSelectToolTexture = *Registry->LoadTexture("Assets/images/faceSelectTool.png");

    boxToolTexture = *Registry->LoadTexture("Assets/images/boxTool.png");
    planeToolTexture = *Registry->LoadTexture("Assets/images/planeTool.png");
    waterToolTexture = *Registry->LoadTexture("Assets/images/waterTool.png");

    translateToolTexture = *Registry->LoadTexture("Assets/images/translateTool.png");
    rotateToolTexture = *Registry->LoadTexture("Assets/images/rotateTool.png");
    scaleToolTexture = *Registry->LoadTexture("Assets/images/scaleTool.png");

    vertexToolTexture = *Registry->LoadTexture("Assets/images/vertexTool.png");
    sculptToolTexture = *Registry->LoadTexture("Assets/images/sculptTool.png");
    
    lightEntityTexture = *Registry->LoadTexture("Assets/images/lightTool.png");
    directionalLightEntityTexture = *Registry->LoadTexture("Assets/images/dirLight.png");
    spotLightEntityTexture = *Registry->LoadTexture("Assets/images/spotLight.png");
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

    bool useLazyLoad = true;

    Material newMaterial;

    std::filesystem::path ext = materialPath.extension();
    std::string extensionString = ext.string();

    std::string NormalMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".norm" + extensionString;
    std::string RoughnessMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".rough" + extensionString;
    std::string MetallicMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".metal" + extensionString;
    std::string AOMapString = materialPath.parent_path().generic_string() + "/" + materialPath.stem().generic_string() + ".ao" + extensionString;

    std::string fileName = materialPath.generic_string();

    Texture* newTexture = Registry->LoadTexture(fileName, useLazyLoad);

    if (std::filesystem::exists(NormalMapString))
    {
        if (std::filesystem::exists(RoughnessMapString))
        {
            if (std::filesystem::exists(MetallicMapString))
            {
                if (std::filesystem::exists(AOMapString))
                {
                    Texture* newNormal = Registry->LoadTexture(NormalMapString, useLazyLoad);
                    Texture* newRoughness = Registry->LoadTexture(RoughnessMapString, useLazyLoad);
                    Texture* newMetal = Registry->LoadTexture(MetallicMapString, useLazyLoad);
                    Texture* newAO = Registry->LoadTexture(AOMapString, useLazyLoad);
                    newMaterial = Graphics->CreateMaterial(newTexture, newNormal, newRoughness, newMetal, newAO);
                }
                else
                {
                    Texture* newNormal = Registry->LoadTexture(NormalMapString, useLazyLoad);
                    Texture* newRoughness = Registry->LoadTexture(RoughnessMapString, useLazyLoad);
                    Texture* newMetal = Registry->LoadTexture(MetallicMapString, useLazyLoad);
                    newMaterial = Graphics->CreateMaterial(newTexture, newNormal, newRoughness, newMetal);
                }
            }
            else
            {
                Texture* newNormal = Registry->LoadTexture(NormalMapString, useLazyLoad);
                Texture* newRoughness = Registry->LoadTexture(RoughnessMapString, useLazyLoad);
                newMaterial = Graphics->CreateMaterial(newTexture, newNormal, newRoughness);
            }
        }
        else
        {
            Texture* newNormal = Registry->LoadTexture(NormalMapString, useLazyLoad);
            newMaterial = Graphics->CreateMaterial(newTexture, newNormal);
        }
    }
    else
    {
        newMaterial = Graphics->CreateMaterial(newTexture);
    }

    return newMaterial;
}

std::vector<HotspotTexture> EditorState::LoadHotspotTextures(GraphicsModule& graphics)
{
    std::vector<HotspotTexture> LoadedHotspotTextures;
    std::string path = "Assets/hotspot_textures";

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::path ext = entry.path().extension();
        std::string extensionString = ext.string();
        if (extensionString == ".hs")
        {
            std::string fileName = entry.path().generic_string();
            HotspotTexture newHotspotTexture;
            newHotspotTexture.Load(fileName);
            LoadedHotspotTextures.push_back(newHotspotTexture);
        }
    }
    return LoadedHotspotTextures;
}

void EditorState::ReloadHotspotTextures(GraphicsModule& graphics)
{
    LoadedHotspotTextures.clear();
    LoadedHotspotTextures = LoadHotspotTextures(graphics);
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

    bool drawHeMeshDebug = Cursor.GetSelectMode() == SelectMode::FaceSelect || Cursor.GetSelectMode() == SelectMode::VertSelect || Cursor.GetToolMode() == ToolMode::Vertex;

    EditorScene.EditorDraw(*Graphics, ViewportBuffer, &ViewportCamera, true, drawHeMeshDebug);
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

    {
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

    if (UI->StartFrame("Tools", PlacementSettings(PlacementType::RECT_ABSOLUTE, ToolbarButtonRect), 0.0f, c_FrameLight))
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
        UI->CheckBox("Grid Snap", Cursor.ShouldSnapToGrid);
        UI->NewLine();

        UI->Text(std::to_string(Cursor.TransSnap));
        UI->NewLine();

        if (UI->TextButton("<"))
        {
            Cursor.DecrementTransSnap();
        }
        if (UI->TextButton(">"))
        {
            Cursor.IncrementTransSnap();
        }
        UI->NewLine();

        UI->CheckBox("Rot Snap", Cursor.ShouldSnapToRotationGrid);
        UI->NewLine();
        UI->CheckBox("Static Light", UseStaticLight);
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

    // Add directional light in direction of camera
    DirectionalLightRenderCommand dlCommand;
    if (UseStaticLight)
    {
        dlCommand.m_Direction = Vec3f(0.0f, 0.0f, -1.0f);
    }
    else
    {
        dlCommand.m_Direction = EntityCamera.GetDirection();
    }
    dlCommand.m_Colour = Vec3f(1.0f);
    dlCommand.m_ShadowBlurMult = 0.5f;

    Graphics->AddRenderCommand(dlCommand);

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
        ClearEntity();
    }
    if (UI->TextButton("Open", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
    {
        std::string EntityName;
        if (Engine::FileOpenDialog(EntityName, "Open Entity", "Entity", "*.ntt", "./Assets/entities"))
        {
            LoadEntity(EntityName);
        }
    }
    if (UI->TextButton("Save", Vec2f(40.0f, 40.0f), 8.0f, c_TopButton, Vec3f(1.0f, 1.0f, 1.0f)))
    {
        std::string EntityName;
        if (Engine::FileSaveDialog(EntityName, "Save Entity", "Entity", "*.ntt", "./Assets/entities"))
        {
            SaveEntity(EntityName);
        }
    }

    // Entity inspector panel
    Rect InspectorPanelRect = Rect(Vec2f(SceneViewportRect.location.x + SceneViewportRect.size.x, SceneViewportRect.location.y),
        Vec2f(ViewportSize.x - (SceneViewportRect.location.x + SceneViewportRect.size.x), ViewportSize.y - SceneViewportRect.location.y));

    UI->StartFrame("Entity Inspector", PlacementSettings(PlacementType::RECT_ABSOLUTE, InspectorPanelRect), 12.0f, c_NiceLighterBlue);
    {
        DrawEntityInspectorPanel();
    }
    UI->EndFrame();

    // Entity resource drawer stuff
    Vec2i ScreenSize = Engine::GetClientAreaSize();

    Rect ResourcePanelRect = Rect(Vec2f(ViewportRect.location.x, ViewportRect.location.y + ViewportRect.size.y), Vec2f(ViewportRect.size.x, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y)));
    UI->StartFrame("Resources", PlacementSettings(PlacementType::RECT_ABSOLUTE, ResourcePanelRect), 16.0f, c_FrameDark);
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
                Click materialClick = UI->ImgButton(Mat.m_Albedo->Path.GetFileNameNoExt(), *Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_ResourceButton);
                if (materialClick.clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        Material* MatPtr = &Mat;
                        Cursor.StartDraggingNewMaterial(MatPtr);
                    }

                }
                if (materialClick.clicked)
                {
                    Cursor.StopDragging();
                    Cursor.ApplyMaterialToSelectedObjects(Mat);
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
                EntityEditorScene.AddDirectionalLight(DirectionalLight());
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

void EditorState::ClearEntity()
{
    EntityEditorScene.Clear();
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

        ModelJson["Mesh"] = Mod.m_StaticMesh.Path.GetRelativePath();

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

void EditorState::LoadEntity(std::string Filename)
{
    std::ifstream File(Filename, std::ifstream::in);
    if (!File.is_open())
    {
        Engine::DEBUGPrint("Failed to load entity :(");
        return;
    }

    json EntityJson;
    File >> EntityJson;

    // Clear the current entity before loading a new one
    ClearEntity();

    auto& ModelsList = EntityJson["Models"];
    for (auto& ModelJson : ModelsList)
    {
        Model* Mod = new Model();
        AssetRegistry* Registry = AssetRegistry::Get();

        Mod->m_StaticMesh = *Registry->LoadStaticMesh(ModelJson["Mesh"].get<std::string>());
        Mat4x4f ModTrans;
        auto& OffsetTrans = ModelJson["OffsetTrans"];
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                ModTrans[i][j] = OffsetTrans[i * 4 + j];
            }
        }
        Mod->GetTransform().SetTransformMatrix(ModTrans);
        EntityEditorScene.AddModel(Mod);
    }
}

void EditorState::DrawMaterialEditor()
{



}

void EditorState::DrawHotSpotMaterialEditor()
{
    UIModule* UI = UIModule::Get();
    InputModule* Input = InputModule::Get();

    Vec2f FrameSize = UI->GetCurrentFrameSize();

    UI->StartFrame("Hotspot Material Editor", PlacementSettings(Vec2f(FrameSize.x * 0.75f, FrameSize.y * 0.75f)), 14.0f, c_NiceLighterBlue);
    {
        if (HasSelectedMaterial)
        {
            Vec2f hotspotMaterialFrameSize = UI->GetCurrentFrameSize();

            float smallerDim = std::min(hotspotMaterialFrameSize.x, hotspotMaterialFrameSize.y);
            
            smallerDim -= smallerDim * 0.05; // Padding from edges of frame

            Rect frameRect = UI->GetCurrentFrameRect();

            // Center the image in the frame and scale it to fit while maintaining aspect ratio
            Vec2f imageSize = GraphicsModule::Get()->m_Renderer.GetTextureSize(CurrentHotspotTexture.m_Material.m_Albedo->GetID());
            float imageAspect = imageSize.x / imageSize.y;
            float frameAspect = frameRect.size.x / frameRect.size.y;
            Rect imgRect;
            if (imageAspect > frameAspect)
            {
                imgRect.size.x = smallerDim;
                imgRect.size.y = smallerDim / imageAspect;
            }
            else
            {
                imgRect.size.y = smallerDim;
                imgRect.size.x = smallerDim * imageAspect;
            }

            if (Input->GetMouseState().GetMouseButtonState(MouseButton::RMB))
            {
                HotspotTextureOffset += Input->GetMouseState().GetDeltaMousePos();
            }

            int deltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();

            HotspotTextureZoom += deltaMouseWheel * 0.1f;

            imgRect.size.x *= HotspotTextureZoom;
            imgRect.size.y *= HotspotTextureZoom;


            // Center the image rect in the frame rect
            imgRect.location.x = HotspotTextureOffset.x + frameRect.location.x + (frameRect.size.x - imgRect.size.x) / 2.0f;
            imgRect.location.y = HotspotTextureOffset.y + frameRect.location.y + (frameRect.size.y - imgRect.size.y) / 2.0f;

            UI->ImgPanel(CurrentHotspotTexture.m_Material.m_Albedo->GetID(), PlacementSettings(PlacementType::RECT_ABSOLUTE, imgRect));

            Vec2f NormalizedPixelSnap;
            NormalizedPixelSnap.x = (float)HotspotPixelSnap / imageSize.x;
            NormalizedPixelSnap.y = (float)HotspotPixelSnap / imageSize.y;


            // If the user clicks on the frame, start dragging out a new hotspot rect if they didn't click on an existing one
            if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed && !DraggingNewHotspotRect)
            {

                Vec2f MousePos = Input->GetMouseState().GetMousePos();
                Rect frameRect = UI->GetCurrentFrameRect();
                if (frameRect.Contains(MousePos))
                {
                    // Clamp mouse position to the image rect
                    if (MousePos.x < imgRect.location.x) MousePos.x = imgRect.location.x;
                    if (MousePos.y < imgRect.location.y) MousePos.y = imgRect.location.y;
                    if (MousePos.x > imgRect.location.x + imgRect.size.x) MousePos.x = imgRect.location.x + imgRect.size.x;
                    if (MousePos.y > imgRect.location.y + imgRect.size.y) MousePos.y = imgRect.location.y + imgRect.size.y;

                    bool clickedOnExistingHotspot = false;
                    //for (auto& hotspotRect : HotspotRects)
                    //{
                    //    Rect hotspotRectAbsolute = Rect(imgRect.location + (hotspotRect.location * imgRect.size), hotspotRect.size * imgRect.size);
                    //    if (hotspotRectAbsolute.Contains(MousePos))
                    //    {
                    //        clickedOnExistingHotspot = true;
                    //        break;
                    //    }
                    //}
                    if (!clickedOnExistingHotspot)
                    {
                        DraggingNewHotspotRect = true;
                        NewHotspotRectStartPos = MousePos - imgRect.location;

                        // Store starting position as relative to img rect
                        //NewHotspotRectStartPos -= imgRect.location;
                        NewHotspotRectStartPos.x /= imgRect.size.x;
                        NewHotspotRectStartPos.y /= imgRect.size.y;

                        // Snap rect position
                        NewHotspotRectStartPos.x = Math::Round(NewHotspotRectStartPos.x, NormalizedPixelSnap.x);
                        NewHotspotRectStartPos.y = Math::Round(NewHotspotRectStartPos.y, NormalizedPixelSnap.y);
                    }
                }
            }
            // If the user releases the mouse button while dragging out a new hotspot rect, add it to the list of hotspot rects
            if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justReleased && DraggingNewHotspotRect)
            {
                Vec2f MousePos = Input->GetMouseState().GetMousePos();
                Vec2f NewHotspotRectEndPos = MousePos - imgRect.location;
                NewHotspotRectEndPos.x /= imgRect.size.x;
                NewHotspotRectEndPos.y /= imgRect.size.y;

                // If the mouse is outside the image rect, clamp the current position to the edge of the image rect
                if (NewHotspotRectEndPos.x < 0.0f) NewHotspotRectEndPos.x = 0.0f;
                if (NewHotspotRectEndPos.y < 0.0f) NewHotspotRectEndPos.y = 0.0f;
                if (NewHotspotRectEndPos.x > 1.0f) NewHotspotRectEndPos.x = 1.0f;
                if (NewHotspotRectEndPos.y > 1.0f) NewHotspotRectEndPos.y = 1.0f;

                // Snap rect position
                NewHotspotRectEndPos.x = Math::Round(NewHotspotRectEndPos.x, NormalizedPixelSnap.x);
                NewHotspotRectEndPos.y = Math::Round(NewHotspotRectEndPos.y, NormalizedPixelSnap.y);

                Rect NewHotspotRect = Rect(
                    Vec2f(std::min(NewHotspotRectStartPos.x, NewHotspotRectEndPos.x), std::min(NewHotspotRectStartPos.y, NewHotspotRectEndPos.y)),
                    Vec2f(std::abs(NewHotspotRectEndPos.x - NewHotspotRectStartPos.x), std::abs(NewHotspotRectEndPos.y - NewHotspotRectStartPos.y))
                );

                if (NewHotspotRect.size.x > 0.0001f && NewHotspotRect.size.y > 0.0001f)
                {
                    // Store rect as relative to img rect, with normalized coordinates (0 to 1)
/*                    NewHotspotRect.location.x /= imgRect.size.x;
                    NewHotspotRect.location.y /= imgRect.size.y;
                    NewHotspotRect.size.x /= imgRect.size.x;
                    NewHotspotRect.size.y /= imgRect.size.y;    */            

                    for (auto& hotspotRect : CurrentHotspotTexture.m_Hotspots)
                    {
                        NewHotspotRect.shrinkOverlap(hotspotRect);
                    }

                    CurrentHotspotTexture.m_Hotspots.push_back(NewHotspotRect);
                    DraggingNewHotspotRect = false;
                }
                else
                {
                    DraggingNewHotspotRect = false;
                }

            }
            // If the user is dragging out a new hotspot rect, draw a rect from the start position to the current mouse position
            if (DraggingNewHotspotRect)
            {
                Vec2f MousePos = Input->GetMouseState().GetMousePos();
                Vec2f NewHotspotRectCurrentPos = MousePos - imgRect.location;
                NewHotspotRectCurrentPos.x /= imgRect.size.x;
                NewHotspotRectCurrentPos.y /= imgRect.size.y;

                // If the mouse is outside the image rect, clamp the current position to the edge of the image rect
                if (NewHotspotRectCurrentPos.x < 0.0f) NewHotspotRectCurrentPos.x = 0.0f;
                if (NewHotspotRectCurrentPos.y < 0.0f) NewHotspotRectCurrentPos.y = 0.0f;
                if (NewHotspotRectCurrentPos.x > 1.0f) NewHotspotRectCurrentPos.x = 1.0f;
                if (NewHotspotRectCurrentPos.y > 1.0f) NewHotspotRectCurrentPos.y = 1.0f;

                // Snap rect position
                NewHotspotRectCurrentPos.x = Math::Round(NewHotspotRectCurrentPos.x, NormalizedPixelSnap.x);
                NewHotspotRectCurrentPos.y = Math::Round(NewHotspotRectCurrentPos.y, NormalizedPixelSnap.y);

                NewHotspotRect = Rect(
                    Vec2f(std::min(NewHotspotRectStartPos.x, NewHotspotRectCurrentPos.x), std::min(NewHotspotRectStartPos.y, NewHotspotRectCurrentPos.y)),
                    Vec2f(std::abs(NewHotspotRectCurrentPos.x - NewHotspotRectStartPos.x), std::abs(NewHotspotRectCurrentPos.y - NewHotspotRectStartPos.y))
                );

                // Store rect as relative to img rect, with normalized coordinates (0 to 1)
                //NewHotspotRect.location.x /= imgRect.size.x;
                //NewHotspotRect.location.y /= imgRect.size.y;
                //NewHotspotRect.size.x /= imgRect.size.x;
                //NewHotspotRect.size.y /= imgRect.size.y;

                for (auto& hotspotRect : CurrentHotspotTexture.m_Hotspots)
                {
                    NewHotspotRect.shrinkOverlap(hotspotRect);
                }
            }

            for (int i = CurrentHotspotTexture.m_Hotspots.size() - 1; i >= 0; i--)
            {
                Rect& hotspotRect = CurrentHotspotTexture.m_Hotspots[i];
                Rect hotspotRectAbsolute = Rect(imgRect.location + (hotspotRect.location * imgRect.size), hotspotRect.size * imgRect.size);
                UI->DrawBorder(PlacementSettings(PlacementType::RECT_ABSOLUTE, hotspotRectAbsolute), 4.0f, c_NiceBrightGreen);
                //if (UI->TextButton("Wee", PlacementSettings(PlacementType::RECT_ABSOLUTE, hotspotRectAbsolute), 0.0f))
                //{
                //    HotspotRects.erase(HotspotRects.begin() + i);
                //}
            }

            // If we're dragging out a new hotspot rect, draw it as well
            if (DraggingNewHotspotRect)
            {
                Rect hotspotRectAbsolute = Rect(imgRect.location + (NewHotspotRect.location * imgRect.size), NewHotspotRect.size * imgRect.size);
                UI->DrawBorder(PlacementSettings(PlacementType::RECT_ABSOLUTE, hotspotRectAbsolute), 4.0f, c_NicePurple);
                //UI->TextButton("Wee", PlacementSettings(PlacementType::RECT_ABSOLUTE, hotspotRectAbsolute), 0.0f);
            }
        }
        else
        {
            UI->Text("Select a Material to begin", PlacementType::FIT_BOTH, 12.0f);
        }
    }
    UI->EndFrame();

    UI->StartFrame("Hotspot Editor Settings", PlacementSettings(Vec2f(FrameSize.x * 0.25f, FrameSize.y * 0.75f)), 14.0f, MakeColour(102, 232, 12));
    {
        if (UI->TextButton("Save Hotspots", PlacementSettings(PlacementType::FIT_WIDTH, 40.0f), 8.0f, c_NiceBrightGreen))
        {
            if (CurrentHotspotTexture.m_Hotspots.empty() || !HasSelectedMaterial)
            {
                Engine::DEBUGPrint("No hotspots to save!");
            }
            else
            {
                std::string SavePath;
                if (Engine::FileSaveDialog(SavePath, "Save Hotspot Texture", "Hotspots", "*.hs", "./Assets/hotspot_textures"))
                {
                    SaveHotspotTexture(SavePath, CurrentHotspotTexture);
                }
                ReloadHotspotTextures(*GraphicsModule::Get());
            }
        }
        if (UI->TextButton("Load Hotspots", PlacementSettings(PlacementType::FIT_WIDTH, 40.0f), 8.0f, c_NiceBrightGreen))
        {
            std::string LoadPath;
            if (Engine::FileOpenDialog(LoadPath, "Load Hotspot Texture", "Hotspots", "*.hs", "./Assets/hotspot_textures"))
            {
                CurrentHotspotTexture = LoadHotspotTexture(LoadPath);
                HasSelectedMaterial = true;
            }
        }
        if (UI->TextButton("Clear Hotspots", PlacementSettings(PlacementType::FIT_WIDTH, 40.0f), 8.0f, c_NiceYellow))
        {
            CurrentHotspotTexture.m_Hotspots.clear();
        }
        if (HasSelectedMaterial)
        {
            UI->CheckBox("Allow Rotation", CurrentHotspotTexture.m_AllowRotation);
            UI->NewLine();
        }

        std::string PixelSnapString = "Hotspot Pixel Snap: " + std::to_string(HotspotPixelSnap);
        UI->Text(PixelSnapString);
        UI->NewLine();
        if (UI->TextButton("<"))
        {
            SelectedHotspotPixelSnapIndex--;
            if (SelectedHotspotPixelSnapIndex < 0)
            {
                SelectedHotspotPixelSnapIndex = 0;
            }
        }
        if (UI->TextButton(">"))
        {
            SelectedHotspotPixelSnapIndex++;
            if (SelectedHotspotPixelSnapIndex > HotspotPixelSnaps.size() - 1)
            {
                SelectedHotspotPixelSnapIndex = HotspotPixelSnaps.size() - 1;
            }
        }
        HotspotPixelSnap = HotspotPixelSnaps[SelectedHotspotPixelSnapIndex];

    }
    UI->EndFrame();

    //UI->NewLine();

    // Draw resource drawer for materials
    UI->StartFrame("Materials", PlacementType::FIT_BOTH, 12.0f, c_NiceYellow);
    {
        Vec2f frameSize = UI->GetCurrentFrameSize();

        for (auto& Mat : LoadedMaterials)
        {
            Click materialClick = UI->ImgButton(Mat.m_Albedo->Path.GetFileNameNoExt(), *Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_ResourceButton);
            if (materialClick.clicked)
            {
                CurrentHotspotTexture.m_Hotspots.clear();
                CurrentHotspotTexture.m_Material = Mat;
                HasSelectedMaterial = true;
            }
        }
    }
    UI->EndFrame();
}

void EditorState::SaveHotspotTexture(std::string TextureName, HotspotTexture& HotspotTex)
{
    HotspotTex.Save(TextureName);
}

HotspotTexture EditorState::LoadHotspotTexture(std::string TextureName)
{
    HotspotTexture newHotspotTex;
    newHotspotTex.Load(TextureName);
    return newHotspotTex;
}

void EditorState::DrawNewTabScreen()
{
    UIModule* UI = UIModule::Get();

    if (UI->TextButton("New Material", PlacementType::FIT_WIDTH, 4.0f, c_NicePurple));
    {

    }
    UI->NewLine();
    if (UI->TextButton("New Entity", PlacementType::FIT_WIDTH, 4.0f, c_NicePurple))
    {

    }
    UI->NewLine();
    if (UI->TextButton("New Hotspot Texture", PlacementType::FIT_WIDTH, 4.0f, c_NicePurple));
    {

    }
}

void EditorState::DrawEditorUI()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    // Left toolbar buttons
    Rect ToolbarButtonRect = Rect(Vec2f(0.0f, 60.0f), Vec2f(ViewportRect.location.x, ViewportRect.size.y));

    if (UI->StartFrame("Tools", PlacementSettings(PlacementType::RECT_ABSOLUTE, ToolbarButtonRect), 0.0f, c_FrameLight))
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
        case GeometryMode::Water:
            GeoModeTexture = waterToolTexture;
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

        if (UI->ImgButton("VertexTool", vertexToolTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Vertex ? SelectedColour : UnSelectedColour))
        {
            Cursor.SetToolMode(ToolMode::Vertex);
        }

        if (UI->ImgButton("SculptTool", sculptToolTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Sculpt ? SelectedColour : UnSelectedColour))
        {
            Cursor.SetToolMode(ToolMode::Sculpt);
        }

        UI->CheckBox("Grid Snap", Cursor.ShouldSnapToGrid);
        UI->NewLine();

        UI->Text(std::to_string(Cursor.TransSnap));
        UI->NewLine();

        if (UI->TextButton("<"))
        {
            Cursor.DecrementTransSnap();
        }
        if (UI->TextButton(">"))
        {
            Cursor.IncrementTransSnap();
        }
        UI->NewLine();

        UI->CheckBox("Rot Snap", Cursor.ShouldSnapToRotationGrid);
    }
    UI->EndFrame();

    DrawTopPanel();
    DrawToolSettingsPanel();
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

    UI->StartFrame("Top", PlacementSettings(PlacementType::RECT_ABSOLUTE, TopPanelRect), 0.0f, c_FrameDark);
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

            if (Engine::FileOpenDialog(FileName, "Load Level", "Level", "*.lvl", "./Assets/levels"))
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
            if (Engine::FileSaveDialog(FileName, "Save Level", "Level", "*.lvl", "./Assets/levels"))
            {
                EditorScene.Save(FileName);
                Engine::SetWindowTitleText(FileName);
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

void EditorState::DrawToolSettingsPanel()
{
    UIModule* UI = UIModule::Get();
    
    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect ToolSettingsRect = Rect(
        Vec2f(ScreenSize.x * 0.0f, ViewportRect.location.y + ViewportRect.size.y), 
        Vec2f(ScreenSize.x * 0.1f, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y))
    );

    UI->StartFrame("Tool Settings", PlacementSettings(PlacementType::RECT_ABSOLUTE, ToolSettingsRect), 8.0f, c_FrameDark, false);
    {
        Cursor.DrawToolSettingsPanel();
    }
    UI->EndFrame();
}

void EditorState::DrawDrawerSettingsPanel()
{
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect ModeSelectRect = Rect(
        Vec2f(ScreenSize.x * 0.1f, ViewportRect.location.y + ViewportRect.size.y), 
        Vec2f(ScreenSize.x * 0.05f, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y))
    );

    UI->StartFrame("Mode Select", PlacementSettings(PlacementType::RECT_ABSOLUTE, ModeSelectRect), 6.0f, c_FrameLight);
    {
        if (UI->TextButton("Content", PlacementType::FIT_WIDTH, 6.0f))
        {
            Drawer = DrawerMode::CONTENT;
        }
        if (UI->TextButton("Browser", PlacementType::FIT_WIDTH, 6.0f))
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

    Rect ResourcePanelRect = Rect(
        Vec2f(ScreenSize.x * 0.15f, ViewportRect.location.y + ViewportRect.size.y),
        Vec2f((ViewportRect.size.x + ViewportRect.location.x) - (ScreenSize.x * 0.15f), ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y)));

    if (Drawer == DrawerMode::CONTENT)
    {
        UI->StartFrame("Resources", PlacementSettings(PlacementType::RECT_ABSOLUTE,  ResourcePanelRect), 16.0f, c_FrameDark);
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
                    Click materialClick = UI->ImgButton(Mat.m_Albedo->Path.GetFileNameNoExt(), *Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_ResourceButton);
                    if (materialClick.clicking)
                    {
                        lastUsedMaterial = &Mat;
                        if (!Cursor.IsDraggingSomething())
                        {
                            Material* MatPtr = &Mat;
                            Cursor.StartDraggingNewMaterial(MatPtr);
                        }

                    }
                    if (materialClick.clicked)
                    {
                        Cursor.StopDragging();
                        Cursor.ApplyMaterialToSelectedObjects(Mat);
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

            UI->StartTab("HotSpot Textures", c_Tab);
            {
                for(auto& HotspotMat : LoadedHotspotTextures)
                {
                    Click materialClick = UI->ImgButton(HotspotMat.m_Material.m_Albedo->Path.GetFileNameNoExt(), *HotspotMat.m_Material.m_Albedo, Vec2f(80, 80), 5.0f, c_ResourceButton);
                    if (materialClick.clicking)
                    {
                        //lastUsedHotspotMaterial = &HotspotMat;
                        //if (!Cursor.IsDraggingSomething())
                        //{
                        //    Cursor.StartDraggingNewHotspotMaterial(&HotspotMat);
                        //}
                    }
                    if (materialClick.clicked)
                    {
                        Cursor.ApplyHotspotTextureToSelectedObjects(HotspotMat);
                        //Cursor.StopDragging();
                        //Cursor.ApplyHotspotMaterialToSelectedObjects(HotspotMat);
                    }
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
                if (UI->ImgButton("SpotLightEntity", spotLightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f, c_ResourceButton).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        SpotLight NewSpotLight = SpotLight();
                        SpotLight* SpotLightPtr = FocusedScene.AddSpotLight(NewSpotLight);
                        Cursor.StartDraggingNewSpotLight(SpotLightPtr);
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
        UI->StartFrame("Browser", PlacementSettings(PlacementType::RECT_ABSOLUTE, ResourcePanelRect), 16.0f, c_FrameDark);
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
                        FilePath resourcePath = entry.path().filename().generic_string();

                        if (resourcePath.GetExt() == ".png" || resourcePath.GetExt() == ".jpg")
                        {
                            ResourceTab newResourceTab;
                            newResourceTab.Type = TabType::TEXTURE;
                            newResourceTab.ResourcePath = entry.path().generic_string();

                            ResourceTabs.push_back(newResourceTab);
                        }

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

    UI->StartFrame("Settings", PlacementSettings(PlacementType::RECT_ABSOLUTE, InspectorPanelRect), 16.0f, c_Inspector);
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

void EditorState::DrawResourceTab(ResourceTab& Tab)
{
    UIModule* UI = UIModule::Get();

    if (Tab.Type == TabType::TEXTURE)
    {
        Texture* tex = AssetRegistry::Get()->LoadTexture(Tab.ResourcePath.GetFullPath());

        UI->ImgButton(Tab.ResourcePath.GetFileName(), tex->GetID(), Vec2f(500.0f, 500.0f), 10.0f, c_NiceYellow);
    }
}
#endif