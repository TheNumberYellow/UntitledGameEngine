#pragma once
#include "State/BaseState.h"

#include "HalfEdge/HalfEdge.h"

#include "GameEngine.h"

#include "CursorState.h"
#include <iostream>
//#include <filesystem>

class EditorState;

enum class GameType
{
    SINGLEPLAYER,
    MULTIPLAYER
};

enum class DrawerMode
{
    CONTENT,
    BROWSER
};

class EditorState : public BaseState
{
    //--------------------
    // BaseState Implementation
    //--------------------
public:
    void OnInitialized(ArgsList) override;
    void OnUninitialized() override;
    void OnEnter() override;
    void OnExit() override;
    void Update(double DeltaTime) override;
    void OnResize() override;

    //--------------------
    // Private Member Functions
    //--------------------
private:
    void UpdateEditor(double DeltaTime);

    void UpdateGame(double DeltaTime);

    Rect GetEditorSceneViewportRect();

    Ray GetMouseRay(Camera& cam, Vec2i mousePosition, Rect viewPort);

    void LoadEditorResources();

    std::vector<Model> LoadModels(GraphicsModule& graphics);

    std::vector<Material> LoadMaterials(GraphicsModule& graphics);
    Material LoadMaterial(std::filesystem::path materialPath);

    void MoveCamera(Camera* Camera, float PixelToRadians, double DeltaTime);

    void DrawLevelEditor(GraphicsModule* Graphics, UIModule* UI, double DeltaTime);
    void DrawProjectSettings();

    void DrawEditorUI();

    void DrawTopPanel();
    void DrawDrawerSettingsPanel();
    void DrawResourcesPanel();
    void DrawInspectorPanel();

    //--------------------
    // Private member variables
    //--------------------
private:
    friend class CursorState;
    CursorState Cursor;

    Scene EditorScene;
    Camera ViewportCamera;
    Camera ModelCamera;

    bool CursorLocked = false;

    std::vector<float> RandomSizes;
    std::vector<Vec3f> RandomColours;

    DrawerMode Drawer = DrawerMode::CONTENT;

    std::string CurrentLevelName = "Untitled_Level";

    GameType CurrentGameType = GameType::SINGLEPLAYER;
    
    std::vector<std::string> CustomGameStateNames;
    int CurrentCustomStateIndex = 0;
    bool UsingCustomGameState = false;

    //--------------------
    // Editor Textures
    //--------------------
    Texture playButtonTexture;

    Texture modelSelectToolTexture;
    Texture vertexSelectToolTexture;

    Texture boxToolTexture;
    Texture planeToolTexture;

    Texture translateToolTexture;
    Texture rotateToolTexture;
    Texture scaleToolTexture;

    Texture vertexToolTexture;

    Texture sculptToolTexture;

    Texture gridTexture;
    
    Texture lightEntityTexture;
    Texture directionalLightEntityTexture;
    Texture cameraEntityTexture;
    Texture brainEntityTexture;
    Texture billboardEntityTexture;

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

    Model TranslateBall;

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

    Framebuffer_ID WidgetBuffer;

    //--------------------
    // Constants
    //--------------------
    const Vec3f c_NiceBlue = Vec3f(0.0f, 150.f / 255.f, 255.f / 255.f);
    const Vec3f c_NiceLighterBlue = Vec3f(40.f / 255.f, 190.f / 255.f, 255.f / 255.f);
    const Vec3f c_NicePurple = Vec3f(207.f / 255.f, 159.f / 255.f, 255.f / 255.f);
    const Vec3f c_NicePink = Vec3f(248.f / 255.f, 131.f / 255.f, 121.f / 255.f);
    const Vec3f c_NiceRed = Vec3f(238.f / 255.f, 75.f / 255.f, 43.f / 255.f);
    const Vec3f c_NiceBrightBlue = Vec3f(125.f / 255.f, 249.f / 255.f, 255.f / 255.f);
    const Vec3f c_NiceGreen = Vec3f(11.f / 255.f, 218.f / 255.f, 81.f / 255.f);
    const Vec3f c_NiceBrightGreen = Vec3f(152.f / 255.f, 251.f / 255.f, 152.f / 255.f);
    const Vec3f c_NiceYellow = Vec3f(255.f / 255.f, 191.5f / 255.f, 0.f / 255.f);

    const Colour c_VegasGold = Colour(197.f / 255.f, 179.f / 255.f, 88.f / 255.f);
    const Colour c_LightGoldenRodYellow = Colour(250.f / 255.f, 250.f / 255.f, 210.f / 255.f);
    const Colour c_GoldenYellow = Colour(255.f / 255.f, 223.f / 255.f, 0.0f);
    const Colour c_DarkOrange = Colour(255.f / 255.f, 140.f / 255.f, 0.0f);

    // Test Palette
    const Colour c_Black = Colour(21.f / 255.f, 21.f / 255.f, 21.f / 255.f);
    const Colour c_Maroon = Colour(169.f / 255.f, 29.f / 255.f, 58.f / 255.f);
    const Colour c_Red = Colour(199.f / 255.f, 54.f / 255.f, 89.f / 255.f);
    const Colour c_Grey = Colour(238.f / 255.f, 238.f / 255.f, 238.f / 255.f);

    // Another palette
    const Colour c_Navy = Colour(1.f / 255.f, 32.f / 255.f, 78.f / 255.f);
    const Colour c_Teal = Colour(2.f / 255.f, 131.f / 255.f, 145.f / 255.f);
    const Colour c_Beige = Colour(246.f / 255.f, 220.f / 255.f, 172.f / 255.f);
    const Colour c_Orange = Colour(254.f / 255.f, 174.f / 255.f, 111.f / 255.f);

    // Another another palette
    const Colour c_RetroGrey = MakeColour(238, 238, 238);
    const Colour c_RetroDarkGrey = MakeColour(104, 109, 118);
    const Colour c_RetroBlack = MakeColour(55, 58, 64);
    const Colour c_RetroOrange = MakeColour(220, 95, 0);
    const Colour c_RetroBlue = MakeColour(38, 54, 139);

    const Colour c_TopButton = c_RetroBlue;
    const Colour c_FrameDark = c_RetroBlack;
    const Colour c_FrameLight = c_RetroDarkGrey;
    const Colour c_Button = c_RetroGrey;
    const Colour c_SelectedButton = c_RetroOrange;
    const Colour c_Tab = c_RetroOrange;
    const Colour c_Inspector = c_RetroGrey;
    const Colour c_ResourceButton = c_RetroGrey;

    // TEMP
    int PrevFrameTimeCount = 0;
    double PrevFrameTimeSum = 0.0f;
    int PrevAveFPS = 0;

    Font TestFont;
};
