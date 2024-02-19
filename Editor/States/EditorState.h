#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

#include <filesystem>

//--------------------
// Enums
//--------------------
enum class ToolMode : uint8_t
{
    Select,
    Geometry,
    Move,
    Vertex,
    Sculpt,
};

enum class GeometryMode : uint8_t
{
    Box,
    Plane
};

enum class MoveMode : uint8_t
{
    Translate,
    Rotate,
    Scale
};

enum class DraggingMode : uint8_t
{
    None,
    NewModel,
    NewTexture,
    NewBehaviour,
    NewPointLight,
};



class EditorState : public BaseState
{
    class CursorState
    {
    public:
        void Update(Scene& EditorScene);

        void CycleToolMode();
        void CycleGeometryMode();
        void CycleMoveMode();

        void SetToolMode(ToolMode InToolMode);

        void StartDraggingNewModel(Model NewModel);

        void DrawTransientModels();

    private:
        void UpdateSelectTool();
        void UpdateGeometryTool();
        void UpdateMoveTool();
        void UpdateVertexTool();
        void UpdateSculptTool();

        ToolMode Tool = ToolMode::Select;
        DraggingMode Dragging = DraggingMode::None;

        GeometryMode Geometry = GeometryMode::Box;
        MoveMode Move = MoveMode::Translate;

        Model* DraggingModelPtr = nullptr;
    };

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

    void DrawResourcesPanel();

private:
    //--------------------
    // Private member variables
    //--------------------
    CursorState Cursor;

    Scene EditorScene;
    Camera ViewportCamera;
    Camera ModelCamera;

    bool CursorLocked = false;

    std::filesystem::path CurrentResourceDirectoryPath;

    std::vector<Vec2f> RandomSizes;

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

};
