#pragma once

#include "Scene.h"

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
    Brush,
};

enum class SelectMode : uint8_t
{
    ModelSelect,
    VertexSelect
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
    Plane,
};

enum class DraggingMode : uint8_t
{
    None,
    NewModel,
    NewTexture,
    NewBehaviour,
    NewPointLight,
    NewDirectionalLight,
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

    virtual bool operator==(const ISelectedObject& Other) = 0;

protected:
    const Vec3f c_SelectedBoxColour = Vec3f(0.f / 255.f, 255.f / 255.f, 255.f / 255.f);
    const Vec3f c_InspectorColour = Vec3f(175.f / 255.f, 225.f / 255.f, 175.f / 255.f);
};
//
class SelectedModel : public ISelectedObject
{
public:
    SelectedModel(Model* InModel, Scene* InScene);

    virtual void Draw() override;

    virtual void DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

    virtual bool operator==(const ISelectedObject& Other) override;

private:

    Model* ModelPtr;
    Scene* ScenePtr;
};

class SelectedVertex : public ISelectedObject
{
public:
    SelectedVertex(Vec3f* InVertPtr, Brush* InBrushPtr, Scene* InScene);

    void Draw() override;

    void Update() override;

    void DrawInspectorPanel() override;

    Transform* GetTransform() override;

    void DeleteObject() override;

    bool operator==(const ISelectedObject& Other) override;

private:

    Vec3f* VertPtr = nullptr;
    Brush* BrushPtr = nullptr;
    Scene* ScenePtr = nullptr;
    Transform Trans;
};

class SelectedLight : public ISelectedObject
{
public:
    SelectedLight(PointLight* InPointLight, Scene* InScene);

    virtual void Draw() override;

    virtual void Update() override;

    virtual void DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

    virtual bool operator==(const ISelectedObject& Other) override;

private:

    PointLight* PointLightPtr;
    Scene* ScenePtr;
    Transform Trans;
};

class SelectedDirectionalLight : public ISelectedObject
{
public:
    SelectedDirectionalLight(DirectionalLight* InDirLight, Scene* InScene);

    virtual void Draw() override;

    virtual void Update() override;

    virtual void DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

    virtual bool operator==(const ISelectedObject& Other) override;

private:

    DirectionalLight* DirLightPtr;
    Scene* ScenePtr;
    Transform Trans;
};

class CursorState
{
public:

    CursorState() {}

    CursorState(EditorState* InEditorState, Scene* InEditorScene);

    void Update(float DeltaTime);

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
    void StartDraggingNewMaterial(Material* NewMaterial);
    void StartDraggingNewBehaviour(std::string NewBehaviourName);

    void DrawTransientModels();

    void DrawInspectorPanel();

    bool IsDraggingSomething();

private:
    void UpdateSelectTool();
    void UpdateTransformTool();
    void UpdateGeometryTool();
    void UpdateVertexTool();
    void UpdateSculptTool(float DeltaTime);
    void UpdateBrushTool();

    void UpdateModelSelectTool();
    void UpdateVertexSelectTool();

    void UpdateTranslateTool();
    void UpdateRotateTool();
    void UpdateScaleTool();

    void UpdateBoxTool();
    void UpdatePlaneTool();

    void UpdateSelectedObjects();
    void DrawSelectedObjects();
    void DrawSelectedInspectorPanels();
    void DeleteSelectedObjects();
    void UnselectSelectedObjects();

    void AddToSelectedObjects(ISelectedObject* NewSelectedObject);

    void UpdateSelectedTransformsBasedOnProxy();
    void RotateSelectedTransforms(Quaternion Rotation);

    //RayCastHit EditorRayCast()

    ToolMode Tool = ToolMode::Select;
    DraggingMode Dragging = DraggingMode::None;

    SelectMode Select = SelectMode::ModelSelect;

    TransformMode TransMode = TransformMode::Translate;
    GeometryMode GeoMode = GeometryMode::Box;

    Model* DraggingModelPtr = nullptr;
    PointLight* DraggingPointLightPtr = nullptr;

    Material* DraggingMaterialPtr = nullptr;
    std::string DraggingBehaviourName;

    Transform SelectedProxyTransform;
    std::vector<std::pair<OffsetInfo, ISelectedObject*>> SelectedObjects;

    // Transform mode state + models
    EditingAxis Axis = EditingAxis::None;

    // Translate
    Vec3f ObjectRelativeHitPoint;
    float ObjectDistanceAtHit;
    float TransSnap = 0.5f;

    // Rotate
    Quaternion ObjectInitialRotation;
    float InitialAngle;

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

    // Geometry mode state
    bool IsCreatingNewBox = false;
    Vec3f NewBoxStartPoint;
    float NewBoxHeight = 1.0f;
    AABB BoxBeingCreated;
    float GeoPlaceSnap = 1.0f;

    bool IsCreatingNewPlane = false;
    Vec3f NewPlaneStartPoint;
    Vec3f NewPlaneMin, NewPlaneMax;
    int NewPlaneSubdivisions = 1;

    // Sculpt mode state
    float SculptSpeed = 3.0f;
    float SculptRadius = 1.0f;

    EditorState* EditorStatePtr;
    Scene* EditorScenePtr;
};

