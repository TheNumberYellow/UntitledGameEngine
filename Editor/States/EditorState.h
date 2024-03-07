#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

#include <filesystem>

class EditorState;

//--------------------
// Enums
//--------------------
enum class ToolMode : uint8_t
{
    Select,
    Transform,
    Geometry,
    Vertex,
    Sculpt,
};

enum class TransformMode : uint8_t
{
    Translate,
    Rotate,
    Scale
};

enum class GeometryMode : uint8_t
{
    Box,
    Plane
};


enum class DraggingMode : uint8_t
{
    None,
    NewModel,
    NewTexture,
    NewBehaviour,
    NewPointLight,
};

class ISelectedObject
{
public:

    // Any editor specific drawing that needs to be done for this selected object
    virtual void Draw() {};

    // Any per-frame update that needs to be done while this object is selected
    virtual void Update() {};

    // Fill the UI panel with object-specific data
    virtual void DrawInspectorPanel() = 0;

    virtual Transform* GetTransform() = 0;
    virtual void DeleteObject() = 0;
};

class SelectedModel : public ISelectedObject
{
public:
    SelectedModel(Model* InModel, Scene* InScene);

    virtual void Draw() override;

    virtual void DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:

    Model* ModelPtr;
    Scene* ScenePtr;
};

class SelectedLight : public ISelectedObject
{
public:
    SelectedLight(Vec3f InPos);

    virtual void DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:

    PointLight* PointLightPtr;
    Transform Trans;
};

class CursorState
{
public:

    CursorState() {}

    CursorState(EditorState* InEditorState, Scene* InEditorScene)
        : EditorStatePtr(InEditorState)
        , EditorScenePtr(InEditorScene)
    {
    }

    void Update();

    void CycleToolMode();
    void CycleGeometryMode();
    void CycleMoveMode();

    void SetToolMode(ToolMode InToolMode);

    void StartDraggingNewModel(Model* NewModel);

    void DrawTransientModels();

    bool IsDraggingSomething();

private:
    void UpdateSelectTool();
    void UpdateGeometryTool();
    void UpdateMoveTool();
    void UpdateVertexTool();
    void UpdateSculptTool();

    ToolMode Tool = ToolMode::Select;
    DraggingMode Dragging = DraggingMode::None;

    TransformMode TransMode = TransformMode::Translate;
    GeometryMode GeoMode = GeometryMode::Box;

    Model* DraggingModelPtr = nullptr;
    
    ISelectedObject* SelectedObject = nullptr;

    EditorState* EditorStatePtr;
    Scene* EditorScenePtr;
};


class EditorState : public BaseState
{

public:

    void OnInitialized() override;
    void OnUninitialized() override;
    void OnEnter() override;
    void OnExit() override;
    void Update(float DeltaTime) override;
    void OnResize() override;

private:
    //--------------------
    // Private Member Functions
    //--------------------
    void UpdateEditor(float DeltaTime);
    void UpdateGame(float DeltaTime);

    Rect GetEditorSceneViewportRect();

    Ray GetMouseRay(Camera& cam, Vec2i mousePosition, Rect viewPort);

    void LoadEditorResources();

    std::vector<Model> LoadModels(GraphicsModule& graphics);
    std::vector<Material> LoadMaterials(GraphicsModule& graphics);

    void MoveCamera(Camera* Camera, float PixelToRadians, double DeltaTime);

    void DrawEditorUI();
    void DrawResourcesPanel();

private:
    //--------------------
    // Private member variables
    //--------------------
    friend class CursorState;
    CursorState Cursor;

    Scene EditorScene;
    Camera ViewportCamera;
    Camera ModelCamera;

    bool CursorLocked = false;

    std::filesystem::path CurrentResourceDirectoryPath;

    std::vector<float> RandomSizes;

    //--------------------
    // Editor Textures
    //--------------------
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

    Texture gridTexture;
    
    Material WhiteMaterial;

    //--------------------
    // Editor Fonts
    //--------------------
    Font DefaultFont;
    Font InspectorFont;

    //--------------------
    // Editor Models
    //--------------------
    Model xAxisArrow;
    Model yAxisArrow;
    Model zAxisArrow;

    Model xAxisRing;
    Model yAxisRing;
    Model zAxisRing;

    Model xScaleWidget;
    Model yScaleWidget;
    Model zScaleWidget;

    //--------------------
    // Resources loaded from folders
    //--------------------
    std::vector<Material> LoadedMaterials;
    std::vector<Model> LoadedModels;

    //--------------------
    // Editor Frame Buffers (Come back here when I change how framebuffers are used)
    //--------------------
    GBuffer ViewportBuffer;

    //--------------------
    // Constants
    //--------------------
    const Vec3f c_NiceBlue = Vec3f(0.0f, 150.f / 255.f, 255.f / 255.f);
    const Vec3f c_NiceLighterBlue = Vec3f(40.f / 255.f, 190.f / 255.f, 255.f / 255.f);
    const Vec3f c_NicePurple = Vec3f(207.f / 255.f, 159.f / 255.f, 255.f / 255.f);

};
