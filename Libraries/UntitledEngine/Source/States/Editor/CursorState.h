#pragma once

#include "Scene.h"
#include "Interfaces/EditorClickable_i.h"

class EditorState;
class Brush;
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

class SelectedVertex : public ISelectedObject
{
public:
    SelectedVertex(Vec3f* InVertPtr, Brush* InBrushPtr);

    virtual void Draw() override;

    virtual void Update() override;

    virtual bool DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;

    virtual void DeleteObject() override;

private:

    virtual bool IsEqual(const ISelectedObject& Other) const override;

    Vec3f* VertPtr = nullptr;
    Brush* BrushPtr = nullptr;
    Transform Trans;
};

class SelectedDirectionalLight : public ISelectedObject
{
public:
    SelectedDirectionalLight(DirectionalLight* InDirLight);

    virtual void Draw() override;

    virtual void Update() override;

    virtual bool DrawInspectorPanel() override;

    virtual Transform* GetTransform() override;
    virtual void DeleteObject() override;

private:

    bool IsEqual(const ISelectedObject& other) const override;
    
    DirectionalLight* DirLightPtr;
    Transform Trans;

};

class CursorState
{
public:

    CursorState() {}

    CursorState(EditorState* InEditorState, Scene* InEditorScene);

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
    void UpdateSculptTool(double DeltaTime);
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
    void RecalculateProxyAndObjectOffsets();

    void UpdateSelectedTransformsBasedOnProxy();
    void RotateSelectedTransforms(Quaternion Rotation);

    ISelectedObject* ClickCast(Ray mouseRay);

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
    float RotSnap = M_PI_2 / 8.0f;

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

