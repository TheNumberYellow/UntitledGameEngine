#include "EditorState.h"

#include <filesystem>

void EditorState::OnInitialized()
{
    GraphicsModule* graphics = GraphicsModule::Get();
    CollisionModule* collisions = CollisionModule::Get();
    TextModule* text = TextModule::Get();
    InputModule* input = InputModule::Get();

    AssetRegistry* Registry = AssetRegistry::Get();  

    // Load textures needed for editor gizmos
    Texture redTexture = *Registry->LoadTexture("textures/red.png");
    Texture greenTexture = *Registry->LoadTexture("textures/green.png");
    Texture blueTexture = *Registry->LoadTexture("textures/blue.png");

    WhiteMaterial = graphics->CreateMaterial(*Registry->LoadTexture("images/white.png"));

    // Create translation gizmo models
    xAxisArrow = graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ArrowSmooth.obj"),
        graphics->CreateMaterial(redTexture)
    ));

    yAxisArrow = graphics->CloneModel(xAxisArrow);
    zAxisArrow = graphics->CloneModel(xAxisArrow);

    yAxisArrow.SetMaterial(graphics->CreateMaterial(greenTexture));
    zAxisArrow.SetMaterial(graphics->CreateMaterial(blueTexture));

    // Create rotation gizmo models
    xAxisRing = graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/RotationHoop.obj"),
        graphics->CreateMaterial(redTexture)
    ));

    yAxisRing = graphics->CloneModel(xAxisRing);
    zAxisRing = graphics->CloneModel(xAxisRing);
    
    yAxisRing.SetMaterial(graphics->CreateMaterial(greenTexture));
    zAxisRing.SetMaterial(graphics->CreateMaterial(blueTexture));

    // Create scaling gizmo models
    xScaleWidget = graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ScaleWidget.obj"),
        graphics->CreateMaterial(redTexture)
    ));

    yScaleWidget = graphics->CloneModel(xScaleWidget);
    zScaleWidget = graphics->CloneModel(xScaleWidget);
    
    yScaleWidget.SetMaterial(graphics->CreateMaterial(greenTexture));
    zScaleWidget.SetMaterial(graphics->CreateMaterial(blueTexture));

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
    DefaultFont = text->LoadFont("fonts/ARLRDBD.TTF", 30);
    InspectorFont = text->LoadFont("fonts/ARLRDBD.TTF", 15);

    // Load user resources
    LoadedModels = LoadModels(*graphics);
    LoadedMaterials = LoadMaterials(*graphics);

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
    EditorScene.Init(*graphics, *collisions);
    EditorScene.SetDirectionalLight(DirectionalLight{ Vec3f(0.0f, 0.0f, 1.0f), Vec3f(1.0f, 1.0f, 1.0f) });

    // Set up framebuffers the editor uses
    ViewportBuffer = graphics->CreateGBuffer(GetEditorSceneViewportRect().size);

    Rect ViewportRect = GetEditorSceneViewportRect();
    Vec2i NewCenter = Vec2i((int)(ViewportRect.location.x + ViewportRect.size.x / 2.0f), (int)(ViewportRect.location.y + ViewportRect.size.y / 2.0f));
    //TODO(fraser): clean up mouse constrain/input code
    input->SetMouseCenter(NewCenter);
    Engine::SetCursorCenter(NewCenter);
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

}

void EditorState::OnResize()
{

}

void EditorState::CycleMoveMode()
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

void EditorState::CycleGemoetryMode()
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

Rect EditorState::GetEditorSceneViewportRect()
{
    Vec2f ViewportSize = Engine::GetClientAreaSize();

    Rect SceneViewportRect = Rect(Vec2f(100.0f, 40.0f), ViewportSize - Vec2f(300, 240));
    return SceneViewportRect;
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
