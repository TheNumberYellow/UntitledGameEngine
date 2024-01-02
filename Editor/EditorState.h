#pragma once
#include "State/BaseState.h"

#include "GameEngine.h"

//--------------------
// Enums
//--------------------

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
    void CycleMoveMode();
    void CycleGemoetryMode();

    Rect GetEditorSceneViewportRect();

    std::vector<Model> LoadModels(GraphicsModule& graphics);
    std::vector<Material> LoadMaterials(GraphicsModule& graphics);

private:
    //--------------------
    // Private member variables
    //--------------------
    Scene EditorScene;
    Camera ViewportCamera;
    Camera ModelCamera;

    ToolMode toolMode = ToolMode::SELECT;
    GeometryMode geometryMode = GeometryMode::BOX;
    MoveMode moveMode = MoveMode::TRANSLATE;

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
    // Editor Frame Buffers (Come back here if I change how framebuffers are used)
    //--------------------
    GBuffer ViewportBuffer;




};
