#pragma once

#ifdef USE_EDITOR

#include "Scene/Scene.h"
#include "Interfaces/EditorClickable_i.h"

class EditorState;
class DirectionalLight;
class Model;
class PointLight;
class Material;

//--------------------
// Enums
//--------------------
enum class ToolMode : uint8_t
{
    Select,
    Transform,
    Geometry,
    Vertex,
    Sculpt
};

enum class SelectMode : uint8_t
{
    GenericSelect,
    FaceSelect,
    EdgeSelect,
    VertSelect
};

enum class TransformMode : uint8_t
{
    Translate,
    Rotate,
    Scale,
};

enum class GeometryMode : uint8_t
{
    Box,
    Cylinder,
    Plane,
    Water
};

enum class CylinderState : uint8_t
{
    NotCreating,
    AwaitingFirstClick,
    AwaitingSecondClick,
    AwaitingConfirmation
};

enum class VertexMode : uint8_t
{
    Slice,
};

enum class SliceState : uint8_t
{
    NotSlicing,
    AwaitingFirstClick,
    AwaitingSecondClick,
    AwaitingConfirmation
};

enum class SliceOption : uint8_t
{
    KeepBoth,
    KeepFront,
    KeepBack
};

enum class DraggingMode : uint8_t
{
    None,
    NewModel,
    NewTexture,
    NewBehaviour,
    NewPointLight,
    NewDirectionalLight,
    NewSpotLight,
    NewDecal
};

enum class EditingAxis : uint8_t
{
    None,
    X,
    Y,
    Z,
    Omni,
};

struct OffsetInfo
{
    Vec3f Offset;
    Quaternion RotationDiff;
    float InitialScale;
};

struct EditorClickContext
{

};

class CursorState
{
public:

    CursorState() {}

    CursorState(EditorState* InEditorState, Scene* InEditorScene);

    void SetScene(Scene* InScene);
    void SetCamera(Camera* InCamera);

    void Update(double DeltaTime);

    void ResetAllState();
    void UnselectAll();

    void CycleToolMode();

    void CycleSelectMode();
    void CycleGeometryMode();
    void CycleTransformMode();

    void SetToolMode(ToolMode InToolMode);

    ToolMode GetToolMode();

    SelectMode GetSelectMode();
    TransformMode GetTransMode();
    GeometryMode GetGeoMode();

    void StartDraggingNewModel(Model* NewModel);
    void StartDraggingNewPointLight(PointLight* NewPointLight);
    void StartDraggingNewDirectionalLight(DirectionalLight* NewDirLight);
    void StartDraggingNewSpotLight(SpotLight* NewSpotLight);
    void StartDraggingNewDecal(Decal* NewDecal);
    void StartDraggingNewMaterial(Material* NewMaterial);
    void StartDraggingNewBehaviour(std::string NewBehaviourName);

    void StopDragging();

    void DrawTransientModels();

    void DrawToolSettingsPanel();
    void DrawInspectorPanel();

    bool IsDraggingSomething();

private:
    void UpdateSelectTool();
    void UpdateTransformTool();
    void UpdateGeometryTool();
    void UpdateVertexTool();
    void UpdateSculptTool(double DeltaTime);

    void UpdateGenericSelectTool();
    void UpdateHalfEdgeSelectTool();

    void UpdateTranslateTool();
    void UpdateRotateTool();
    void UpdateScaleTool();

    void UpdateBoxTool();
    void UpdateCylinderTool();
    void UpdatePlaneTool();
    void UpdateWaterTool();

    void UpdateSliceTool();
    void PreviewSlice(he::HalfEdgeMesh* TargetMesh, Plane SlicePlane);

    void UpdateSelectedObjects();
    void DrawSelectedObjects();
    void DrawSelectedInspectorPanels();
    void DeleteSelectedObjects();
    void UnselectSelectedObjects();

    void AddToSelectedObjects(ISelectedObject* NewSelectedObject);
    
    // Temp public
public:
    void RecalculateProxyAndObjectOffsets();
private:

    void UpdateSelectedTransformsBasedOnProxy();
    void RotateSelectedTransforms(Quaternion Rotation);

    RayCastHit SceneRayCastWithScenePlane(Ray mouseRay);

    ISelectedObject* ClickCastGeneric(Ray mouseRay);
    std::vector<ISelectedObject*> ClickCastHalfEdgeMesh(Ray mouseRay);

    bool ClickCastApplyMaterial(Ray mouseRay, Material* material);

    void ApplyMaterialToSelectedObjects(Material& material);
    void ApplyHotspotTextureToSelectedObjects(HotspotTexture& hotspotTexture);

    ToolMode Tool = ToolMode::Select;
    DraggingMode Dragging = DraggingMode::None;

    SelectMode Select = SelectMode::GenericSelect;

    TransformMode TransMode = TransformMode::Translate;
    GeometryMode GeoMode = GeometryMode::Box;
    VertexMode VertMode = VertexMode::Slice;

    Model* DraggingModelPtr = nullptr;
    PointLight* DraggingPointLightPtr = nullptr;
    DirectionalLight* DraggingDirectionalLightPtr = nullptr;
    SpotLight* DraggingSpotLightPtr = nullptr;
    Decal* DraggingDecalPtr = nullptr;

    Material* DraggingMaterialPtr = nullptr;
    std::string DraggingBehaviourName;

    Transform SelectedProxyTransform;
    std::vector<std::pair<OffsetInfo, ISelectedObject*>> SelectedObjects;

    // Transform mode state + models
    EditingAxis Axis = EditingAxis::None;

    // Translate
    Vec3f ObjectRelativeHitPoint;
    Vec3f InitialObjectPosition;
    float ObjectDistanceAtHit;

    std::vector<float> TransSnaps =
    {
        0.01f,
        0.05f,
        0.1f,
        0.5f,
        1.0f
    };
    

    int TransSnapIndex = 4;
    float TransSnap = TransSnaps[TransSnapIndex];

    bool ShouldSnapToGrid = true;
    bool ShouldSnapToRotationGrid = true;

    void IncrementTransSnap();
    void DecrementTransSnap();

    // Rotate
    Quaternion ObjectInitialRotation;
    float InitialAngle;
    float RotSnap = M_PI_2 / 8.0f;
    bool RotateIndividually = false;

    // Scale
    float InitialDistFromObjectCenter;
    float InitialScale;
    float ScaleWidgetModifier = 1.0f;
    bool ScaleIndividually = false;

    // Models
    Model* XAxisTrans;
    Model* YAxisTrans;
    Model* ZAxisTrans;

    Model* TransBall;

    Model* XAxisRot;
    Model* YAxisRot;
    Model* ZAxisRot;

    Model* XAxisScale;
    Model* YAxisScale;
    Model* ZAxisScale;

    Model* ScaleRing;

    // Geometry mode state

    // Box tool state
    bool IsCreatingNewBox = false;
    Vec3f NewBoxStartPoint;
    float NewBoxHeight = 1.0f;
    AABB BoxBeingCreated;
    float GeoPlaceSnap = 1.0f;

    // Plane tool state
    bool IsCreatingNewPlane = false;
    Vec3f NewPlaneStartPoint;
    Vec3f NewPlaneMin, NewPlaneMax;
    int NewPlaneSubdivisions = 1;

    // Cylinder tool state
    //bool IsCreatingNewCylinder = false;
    CylinderState CurrentCylinderState = CylinderState::NotCreating;
    Vec3f NewCylinderStartPoint;
    float NewCylinderHeight = 1.0f;
    float NewCylinderRadius = 1.0f;
    Cylinder CylinderBeingCreated;

    int NumCylinderSegments = 16;

    // Water tool state
    bool IsCreatingNewWater = false;

    // Vertex mode state
    SliceState CurrentSliceState = SliceState::NotSlicing;
    SliceOption CurrentSliceOption = SliceOption::KeepBoth;
    Vec3f SliceStartPoint;
    Vec3f SliceEndPoint;
    Plane SliceHitPlane;
    he::HalfEdgeMesh* SliceTargetMesh = nullptr;
    bool SliceAddCaps = true;

    // Sculpt mode state
    float SculptSpeed = 3.0f;
    float SculptRadius = 1.0f;

    friend class EditorState;

    EditorClickContext ClickContext;

    EditorState* EditorStatePtr;
    Scene* EditorScenePtr;
    Camera* CameraPtr;
};

#endif