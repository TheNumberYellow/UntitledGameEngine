#include "CursorState.h"

#ifdef USE_EDITOR

#include "Modules/GraphicsModule.h"
#include "EditorState.h"
#include "Graphics/Model.h"
#include "Graphics/PointLight.h"
#include "Graphics/Material.h"

CursorState::CursorState(EditorState* InEditorState, Scene* InEditorScene)
    : EditorStatePtr(InEditorState)
    , EditorScenePtr(InEditorScene)
{
    XAxisTrans = &EditorStatePtr->xAxisArrow;
    YAxisTrans = &EditorStatePtr->yAxisArrow;
    ZAxisTrans = &EditorStatePtr->zAxisArrow;

    TransBall = &EditorStatePtr->TranslateBall;

    XAxisRot = &EditorStatePtr->xAxisRing;
    YAxisRot = &EditorStatePtr->yAxisRing;
    ZAxisRot = &EditorStatePtr->zAxisRing;

    XAxisScale = &EditorStatePtr->xScaleWidget;
    YAxisScale = &EditorStatePtr->yScaleWidget;
    ZAxisScale = &EditorStatePtr->zScaleWidget;

    ScaleRing = &EditorStatePtr->ScaleRing;
}

void CursorState::SetScene(Scene* InScene)
{
    EditorScenePtr = InScene;
}

void CursorState::SetCamera(Camera* InCamera)
{
    CameraPtr = InCamera;
}

void CursorState::Update(double DeltaTime)
{
    InputModule* Input = InputModule::Get();
    UIModule* UI = UIModule::Get();

    if (Dragging == DraggingMode::NewModel)
    {
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            Vec3f NewDraggingModelPos;
            if (Input->GetKeyState(Key::Ctrl).pressed)
            {
                SceneRayCastHit SceneHit = EditorScenePtr->RayCast(MouseRay, { DraggingModelPtr });

                if (SceneHit.rayCastHit.hit)
                {
                    NewDraggingModelPos = SceneHit.rayCastHit.hitPoint;
                    Quaternion q = Math::VecDiffToQuat(SceneHit.rayCastHit.hitNormal, Vec3f(0.0f, 0.0f, 1.0f));
                    q = Math::normalize(q);

                    DraggingModelPtr->GetTransform().SetRotation(q);
                }
                else
                {
                    NewDraggingModelPos = MouseRay.point + MouseRay.direction * 8.0f;
                    DraggingModelPtr->GetTransform().SetRotation(Quaternion());
                }
            }
            else
            {
                NewDraggingModelPos = MouseRay.point + MouseRay.direction * 8.0f;
                DraggingModelPtr->GetTransform().SetRotation(Quaternion());
            }

            DraggingModelPtr->GetTransform().SetPosition(NewDraggingModelPos);

            int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();

            Vec3f ScalingIncrement = Vec3f(0.1f, 0.1f, 0.1f);

            Vec3f PrevScale = DraggingModelPtr->GetTransform().GetScale();
            if (DeltaMouseWheel > 0)
            {
                DraggingModelPtr->GetTransform().SetScale(PrevScale + ScalingIncrement);
            }
            else if (DeltaMouseWheel < 0)
            {
                DraggingModelPtr->GetTransform().SetScale(PrevScale - ScalingIncrement);
            }
        }
        else
        {
            DraggingModelPtr = nullptr;

            Dragging = DraggingMode::None;
        }
    }
    else if (Dragging == DraggingMode::NewPointLight)
    {
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            DraggingPointLightPtr->position = MouseRay.point + MouseRay.direction * 8.0f;

            float LightScalingIncrement = 0.1f;

            int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
            if (DeltaMouseWheel > 0)
            {
                DraggingPointLightPtr->intensity += LightScalingIncrement;
            }
            else if (DeltaMouseWheel < 0)
            {
                DraggingPointLightPtr->intensity -= LightScalingIncrement;
            }
        }
        else
        {
            DraggingPointLightPtr = nullptr;

            Dragging = DraggingMode::None;
        }
    }
    else if (Dragging == DraggingMode::NewDirectionalLight)
    {
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            DraggingDirectionalLightPtr->position = MouseRay.point + MouseRay.direction * 8.0f;
        }
        else
        {
            DraggingDirectionalLightPtr = nullptr;

            Dragging = DraggingMode::None;
        }
    }
    else if (Dragging == DraggingMode::NewSpotLight)
    {
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());
            DraggingSpotLightPtr->position = MouseRay.point + MouseRay.direction * 8.0f;
        }
        else
        {
            DraggingSpotLightPtr = nullptr;

            Dragging = DraggingMode::None;
        }
    }
    else if (Dragging == DraggingMode::NewTexture)
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            UI->ImgPanel(*DraggingMaterialPtr->m_Albedo, Rect(Vec2f(MousePos), Vec2f(40.0f, 40.0f)));
        }
        else
        {
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            //ISelectedObject* hitSelectedObject = ClickCastGeneric(MouseRay);

            //if (hitSelectedObject)
            //{
            //    hitSelectedObject->SetScene(EditorScenePtr);
            //    hitSelectedObject->ApplyMaterial(*DraggingMaterialPtr);
            //    delete hitSelectedObject;
            //}

            if (ClickCastApplyMaterial(MouseRay, DraggingMaterialPtr))
            {
                DraggingMaterialPtr = nullptr;
                Dragging = DraggingMode::None;
            }
            else
            {
                DraggingMaterialPtr = nullptr;
                Dragging = DraggingMode::None;
            }
        }
    }
    else if (Dragging == DraggingMode::NewBehaviour)
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {

        }
        else
        {
            auto SceneHit = EditorScenePtr->RayCast(MouseRay);

            if (SceneHit.rayCastHit.hit)
            {
                BehaviourRegistry::Get()->AttachNewBehaviour(DraggingBehaviourName, SceneHit.hitModel);
            }

            Dragging = DraggingMode::None;
        }
    }

    switch (Tool)
    {
    case ToolMode::Select:
    {
        UpdateSelectTool();
        break;
    }
    case ToolMode::Transform:
        UpdateTransformTool();
        break;
    case ToolMode::Geometry:
        UpdateGeometryTool();
        break;
    case ToolMode::Sculpt:
        UpdateSculptTool(DeltaTime);
        break;

    default:
    {
        break;
    }
    }

    UpdateSelectedTransformsBasedOnProxy();

    if (!SelectedObjects.empty())
    {
        // Update widget positions
        Transform ObjTrans = SelectedProxyTransform;

        float DistFromCam = Math::magnitude(ObjTrans.GetPosition() - CameraPtr->GetPosition());

        XAxisTrans->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(DistFromCam / 20.0f * 5.0f, 0.0f, 0.0f));
        YAxisTrans->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, DistFromCam / 20.0f * 5.0f, 0.0f));
        ZAxisTrans->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 0.0f, DistFromCam / 20.0f * 5.0f));

        TransBall->GetTransform().SetPosition(ObjTrans.GetPosition());

        XAxisTrans->GetTransform().SetScale(Vec3f(DistFromCam / 40.0f, DistFromCam / 16.0f, DistFromCam / 40.0f));
        YAxisTrans->GetTransform().SetScale(Vec3f(DistFromCam / 40.0f, DistFromCam / 16.0f, DistFromCam / 40.0f));
        ZAxisTrans->GetTransform().SetScale(Vec3f(DistFromCam / 40.0f, DistFromCam / 16.0f, DistFromCam / 40.0f));

        TransBall->GetTransform().SetScale(DistFromCam / 32.0f);

        XAxisRot->GetTransform().SetPosition(ObjTrans.GetPosition());
        YAxisRot->GetTransform().SetPosition(ObjTrans.GetPosition());
        ZAxisRot->GetTransform().SetPosition(ObjTrans.GetPosition());

        XAxisRot->GetTransform().SetScale(DistFromCam / 6.0f);
        YAxisRot->GetTransform().SetScale(DistFromCam / 6.0f);
        ZAxisRot->GetTransform().SetScale(DistFromCam / 6.0f);

        XAxisScale->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(5.0f, 0.0f, 0.0f));
        YAxisScale->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 5.0f, 0.0f));
        ZAxisScale->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 0.0f, 5.0f));

        ScaleRing->GetTransform().SetPosition(ObjTrans.GetPosition());
        ScaleRing->GetTransform().SetScale((DistFromCam / 6.0f) * ScaleWidgetModifier);
        ScaleRing->GetTransform().SetRotation(Math::VecDiffToQuat(Math::normalize(CameraPtr->GetPosition() - ObjTrans.GetPosition()), Vec3f(0.0f, 0.0f, 1.0f)));

        UpdateSelectedObjects();
        DrawSelectedObjects();

        if (Input->GetKeyState(Key::Delete))
        {
            DeleteSelectedObjects();
        }
        else if (Input->GetKeyState(Key::Escape))
        {
            UnselectSelectedObjects();
        }
        else if (Input->GetKeyState(Key::C) && Input->GetKeyState(Key::Ctrl).pressed)
        {
            // Copy selected objects to clipboard
        }
    }
}

void CursorState::ResetAllState()
{
    Dragging = DraggingMode::None;

    DraggingModelPtr = nullptr;
    DraggingPointLightPtr = nullptr;
    DraggingDirectionalLightPtr = nullptr;
    DraggingMaterialPtr = nullptr;
    DraggingBehaviourName = "";

    //for (ISelectedObject* SelectedObj : SelectedObjects)
    //{
    //    delete SelectedObj;
    //}
    //SelectedObjects.clear();

    Axis = EditingAxis::None;

    IsCreatingNewBox = false;
}

void CursorState::UnselectAll()
{
    UnselectSelectedObjects();
}

void CursorState::CycleToolMode()
{
    ResetAllState();
    switch (Tool)
    {
    case ToolMode::Select:
        Tool = ToolMode::Transform;
        break;
    case ToolMode::Transform:
        Tool = ToolMode::Geometry;
        break;
    case ToolMode::Geometry:
        UnselectAll();
        Tool = ToolMode::Vertex;
        break;
    case ToolMode::Vertex:
        UnselectAll();
        Tool = ToolMode::Sculpt;
        break;
    case ToolMode::Sculpt:
        UnselectAll();
        Tool = ToolMode::Select;
        break;
    default:
        Engine::FatalError("Invalid editor cursor tool mode.");
        break;
    }
}

void CursorState::CycleSelectMode()
{
    ResetAllState();
    UnselectAll();
    switch (Select)
    {
    case SelectMode::GenericSelect:
        Select = SelectMode::FaceSelect;
        break;
    case SelectMode::FaceSelect:
        Select = SelectMode::VertSelect;
        break;
    case SelectMode::VertSelect:
        Select = SelectMode::GenericSelect;
        break;
    default:
        Engine::FatalError("Invalid editor select tool mode.");
        break;
    }
}

void CursorState::CycleGeometryMode()
{
    ResetAllState();
    switch (GeoMode)
    {
    case GeometryMode::Box:
        GeoMode = GeometryMode::Plane;
        break;
    case GeometryMode::Plane:
        GeoMode = GeometryMode::HalfEdge;
        break;
    case GeometryMode::HalfEdge:
        GeoMode = GeometryMode::Water;
        break;
    case GeometryMode::Water:
        GeoMode = GeometryMode::Box;
        break;
    default:
        Engine::FatalError("Invalid editor cursor geometry mode.");
        break;
    }
}

void CursorState::CycleTransformMode()
{
    ResetAllState();
    switch (TransMode)
    {
    case TransformMode::Translate:
        TransMode = TransformMode::Rotate;
        break;
    case TransformMode::Rotate:
        TransMode = TransformMode::Scale;
        break;
    case TransformMode::Scale:
        TransMode = TransformMode::Translate;
        break;
    default:
        Engine::FatalError("Invalid editor cursor move mode.");
        break;
    }
}

void CursorState::SetToolMode(ToolMode InToolMode)
{
    ResetAllState();
    
    if (InToolMode == ToolMode::Geometry
        || InToolMode == ToolMode::Sculpt
        )
    {
        UnselectAll();
    }

    Tool = InToolMode;
}

ToolMode CursorState::GetToolMode()
{
    return Tool;
}

SelectMode CursorState::GetSelectMode()
{
    return Select;
}

TransformMode CursorState::GetTransMode()
{
    return TransMode;
}

GeometryMode CursorState::GetGeoMode()
{
    return GeoMode;
}

void CursorState::StartDraggingNewModel(Model* NewModel)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewModel;

    DraggingModelPtr = NewModel;
}

void CursorState::StartDraggingNewPointLight(PointLight* NewPointLight)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewPointLight;

    DraggingPointLightPtr = NewPointLight;
}

void CursorState::StartDraggingNewDirectionalLight(DirectionalLight* NewDirLight)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewDirectionalLight;

    DraggingDirectionalLightPtr = NewDirLight;
}

void CursorState::StartDraggingNewSpotLight(SpotLight* NewSpotLight)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }
    Dragging = DraggingMode::NewSpotLight;
    DraggingSpotLightPtr = NewSpotLight;
}

void CursorState::StartDraggingNewMaterial(Material* NewMaterial)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewTexture;

    DraggingMaterialPtr = NewMaterial;
}

void CursorState::StartDraggingNewBehaviour(std::string NewBehaviourName)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewBehaviour;

    DraggingBehaviourName = NewBehaviourName;
}

void CursorState::StopDragging()
{
    Dragging = DraggingMode::None;
    DraggingModelPtr = nullptr;
    DraggingPointLightPtr = nullptr;
    DraggingDirectionalLightPtr = nullptr;
    DraggingSpotLightPtr = nullptr;
    DraggingMaterialPtr = nullptr;
    DraggingBehaviourName = "";
}

void CursorState::DrawTransientModels()
{
    if (SelectedObjects.empty())
    {
        return;
    }

    GraphicsModule* Graphics = GraphicsModule::Get();

    if (Tool == ToolMode::Transform)
    {
        if (TransMode == TransformMode::Translate)
        {
            Graphics->Draw(*XAxisTrans);
            Graphics->Draw(*YAxisTrans);
            Graphics->Draw(*ZAxisTrans);
            Graphics->Draw(*TransBall);
        }
        else if (TransMode == TransformMode::Rotate)
        {
            Graphics->Draw(*XAxisRot);
            Graphics->Draw(*YAxisRot);
            Graphics->Draw(*ZAxisRot);
        }
        else if (TransMode == TransformMode::Scale)
        {
            //Graphics->Draw(*XAxisScale);
            //Graphics->Draw(*YAxisScale);
            //Graphics->Draw(*ZAxisScale);

            Graphics->Draw(*ScaleRing);
        }
    }
}

void CursorState::DrawToolSettingsPanel()
{
    UIModule* UI = UIModule::Get();

    Colour titleTextColour = MakeColour(255, 255, 255);
    Colour textColour = MakeColour(200, 200, 200);
    Colour darkTextColour = MakeColour(100, 100, 100);

    Colour selectedColour = MakeColour(100, 200, 255);
    Colour unselectedColour = MakeColour(200, 200, 200);

    switch (Tool)
    {
    case ToolMode::Select:
        UI->Text("Select Tool Settings", PlacementType::FIT_WIDTH, titleTextColour);
        UI->Text("Select Mode", PlacementType::FIT_WIDTH, textColour);
        UI->NewLine();
        if (UI->TextButton("Generic", PlacementType::FIT_WIDTH, 4.0f, Select == SelectMode::GenericSelect ? selectedColour : unselectedColour, darkTextColour))
        {
            Select = SelectMode::GenericSelect;
        }
        if (UI->TextButton("Face", PlacementType::FIT_WIDTH, 4.0f, Select == SelectMode::FaceSelect ? selectedColour : unselectedColour, darkTextColour))
        {
            Select = SelectMode::FaceSelect;
        }
        if (UI->TextButton("Vertex", PlacementType::FIT_WIDTH, 4.0f, Select == SelectMode::VertSelect ? selectedColour : unselectedColour, darkTextColour))
        {
            Select = SelectMode::VertSelect;
        }
        break;
    case ToolMode::Transform:
        UI->Text("Transform Tool Settings", PlacementType::FIT_WIDTH, titleTextColour);
        UI->Text("Transform Mode", PlacementType::FIT_WIDTH, textColour);
        UI->NewLine();
        if (UI->TextButton("Translate", PlacementType::FIT_WIDTH, 4.0f, TransMode == TransformMode::Translate ? selectedColour : unselectedColour, darkTextColour))
        {
            TransMode = TransformMode::Translate;
        }
        if (UI->TextButton("Rotate", PlacementType::FIT_WIDTH, 4.0f, TransMode == TransformMode::Rotate ? selectedColour : unselectedColour, darkTextColour))
        {
            TransMode = TransformMode::Rotate;
        }
        if (UI->TextButton("Scale", PlacementType::FIT_WIDTH, 4.0f, TransMode == TransformMode::Scale ? selectedColour : unselectedColour, darkTextColour))
        {
            TransMode = TransformMode::Scale;
        }
        break;
    case ToolMode::Geometry:
        UI->Text("Geometry Tool Settings", PlacementType::FIT_WIDTH, titleTextColour);
        UI->Text("Geometry Mode", PlacementType::FIT_WIDTH, textColour);
        UI->NewLine();
        if (UI->TextButton("Box", PlacementType::FIT_WIDTH, 4.0f, GeoMode == GeometryMode::Box ? selectedColour : unselectedColour, darkTextColour))
        {
            GeoMode = GeometryMode::Box;
        }
        if (UI->TextButton("Plane", PlacementType::FIT_WIDTH, 4.0f, GeoMode == GeometryMode::Plane ? selectedColour : unselectedColour, darkTextColour))
        {
            GeoMode = GeometryMode::Plane;
        }
        if (UI->TextButton("Half-Edge", PlacementType::FIT_WIDTH, 4.0f, GeoMode == GeometryMode::HalfEdge ? selectedColour : unselectedColour, darkTextColour))
        {
            GeoMode = GeometryMode::HalfEdge;
        }
        if (UI->TextButton("Water", PlacementType::FIT_WIDTH, 4.0f, GeoMode == GeometryMode::Water ? selectedColour : unselectedColour, darkTextColour))
        {
            GeoMode = GeometryMode::Water;
        }
        break;
    case ToolMode::Vertex:
        UI->Text("Vertex Tool Settings", PlacementType::FIT_WIDTH, titleTextColour);
        break;
    case ToolMode::Sculpt:
        UI->Text("Sculpt Tool Settings", PlacementType::FIT_WIDTH, titleTextColour);
        UI->FloatDragger("Sculpt Radius", PlacementType::FIT_WIDTH, SculptRadius, 0.1f, 0.0f, 100.0f);
        UI->FloatDragger("Sculpt Speed", PlacementType::FIT_WIDTH, SculptSpeed, 0.1f, 0.0f, 100.0f);
        break;
    default:

        break;
    }
}

void CursorState::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    if (SelectedObjects.empty())
    {
    }
    else
    {
        DrawSelectedInspectorPanels();
    }
}

bool CursorState::IsDraggingSomething()
{
    return Dragging != DraggingMode::None;
}

void CursorState::UpdateSelectTool()
{
    switch (Select)
    {
    case SelectMode::GenericSelect:
        UpdateGenericSelectTool();
        break;
    case SelectMode::FaceSelect:
    case SelectMode::VertSelect:
        UpdateHalfEdgeSelectTool();
        break;
    default:
        break;
    }
}

void CursorState::UpdateTransformTool()
{
    switch (TransMode)
    {
    case TransformMode::Translate:
        UpdateTranslateTool();
        break;
    case TransformMode::Rotate:
        UpdateRotateTool();
        break;
    case TransformMode::Scale:
        UpdateScaleTool();
        break;
    default:
        break;
    }
}

void CursorState::UpdateGeometryTool()
{
    switch (GeoMode)
    {
    case GeometryMode::Box:
        UpdateBoxTool();
        break;
    case GeometryMode::Plane:
        UpdatePlaneTool();
        break;
    case GeometryMode::HalfEdge:
        UpdateHalfEdgeTool();
        break;
    case GeometryMode::Water:
        UpdateWaterTool();
        break;
    default:
        break;
    }
}

void CursorState::UpdateVertexTool()
{
}

void CursorState::UpdateSculptTool(double DeltaTime)
{
    InputModule* Input = InputModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    KeyState ClickState = Input->GetMouseState().GetMouseButtonState(MouseButton::LMB);

    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Vec2i MousePos = Input->GetMouseState().GetMousePos();

    if (!EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
    {
        return;
    }

    if (Input->GetMouseState().GetDeltaMouseWheel() > 0 || Input->GetKeyState(Key::Plus).justPressed)
    {
        SculptRadius += 0.1f;
    }
    else if (Input->GetMouseState().GetDeltaMouseWheel() < 0 || Input->GetKeyState(Key::Minus).justPressed)
    {
        SculptRadius -= 0.1f;
        if (SculptRadius < 1.0f)
        {
            SculptRadius = 1.0f;
        }
    }

    Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

    SceneRayCastHit FinalHit = EditorScenePtr->RayCast(MouseRay);

    if (FinalHit.rayCastHit.hit)
    {
        // TODO: some sort of polymorphic? solution to various hit results
        if (FinalHit.hitModel)
        {
            if (FinalHit.hitModel->Type == ModelType::PLANE)
            {
                Vec3f HitPoint = FinalHit.rayCastHit.hitPoint;
                Graphics->DebugDrawSphere(HitPoint, SculptRadius, Vec3f(0.6f, 0.3f, 0.9f));

                if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB) || Input->GetMouseState().GetMouseButtonState(MouseButton::RMB))
                {
                    Model* PlaneModel = FinalHit.hitModel;
                    Vec3f ModelSpaceVertPos = HitPoint * Math::inv(PlaneModel->GetTransform().GetTransformMatrix());

                    float VerticalDir = Input->GetMouseState().GetMouseButtonState(MouseButton::RMB) ? -SculptSpeed : SculptSpeed;

                    StaticMesh_ID Mesh = PlaneModel->m_StaticMesh.Id;

                    std::vector<Vertex*> Vertices = Graphics->m_Renderer.MapMeshVertices(Mesh);

                    for (auto& Vert : Vertices)
                    {
                        float Dist = Math::magnitude(Vert->position - ModelSpaceVertPos);
                        //if (Dist < radius)
                        //{
                        float Strength = Math::SmoothStep(Dist, SculptRadius, 0.5f) * VerticalDir * (SculptRadius * 0.25f);
                    
                        //Vec3f hitNorm = FinalHit.rayCastHit.hitNormal;
                        //Vert->position += Strength * hitNorm * (float)DeltaTime;
                        Vert->position += Strength * Vec3f(0.0f, 0.0f, 1.0f) * (float)DeltaTime;
                        //}
                    }

                    Graphics->m_Renderer.UnmapMeshVertices(Mesh);
                    Collisions->InvalidateMeshCollisionData(Mesh);
                    Graphics->RecalculateTerrainModelNormals(*PlaneModel);
                }
                else if (Input->GetMouseState().GetMouseButtonState(MouseButton::MIDDLE))
                {
                    Model* PlaneModel = FinalHit.hitModel;
                    Vec3f ModelSpaceVertPos = HitPoint * Math::inv(PlaneModel->GetTransform().GetTransformMatrix());

                    StaticMesh_ID Mesh = PlaneModel->m_StaticMesh.Id;

                    std::vector<Vertex*> Vertices = Graphics->m_Renderer.MapMeshVertices(Mesh);

                    std::vector<Vertex*> VerticesInRange;
                    float AverageElevation = 0.0f;

                    for (auto& Vert : Vertices)
                    {
                        float Dist = Math::magnitude(Vert->position - ModelSpaceVertPos);
                        if (Dist < SculptRadius)
                        {
                            VerticesInRange.push_back(Vert);
                            AverageElevation += Vert->position.z;
                        }
                    }
                    AverageElevation /= VerticesInRange.size();

                    for (auto& InRangeVert : VerticesInRange)
                    {
                        float Dist = Math::magnitude(InRangeVert->position - ModelSpaceVertPos);
                        float Strength = Math::SmoothStep(Dist, SculptRadius, 0.0f);

                        float Diff = AverageElevation - InRangeVert->position.z;

                        InRangeVert->position.z += SculptSpeed * Diff * Strength * (float)DeltaTime;
                    }

                    Graphics->m_Renderer.UnmapMeshVertices(Mesh);
                    Collisions->InvalidateMeshCollisionData(Mesh);
                    Graphics->RecalculateTerrainModelNormals(*PlaneModel);

                }
            }
        }
    }
}

void CursorState::UpdateGenericSelectTool()
{
    InputModule* Input = InputModule::Get();

    Vec2i MousePos = Input->GetMouseState().GetMousePos();

    if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
    {
        Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

        ISelectedObject* hitSelectedObject = ClickCastGeneric(MouseRay);

        bool HoldingShift = Input->GetKeyState(Key::Shift).pressed;

        if (hitSelectedObject)
        {
            hitSelectedObject->SetScene(EditorScenePtr);

            if (!HoldingShift)
            {
                UnselectSelectedObjects();
            }

            AddToSelectedObjects(hitSelectedObject);
        }
        else if (!HoldingShift)
        {
            UnselectSelectedObjects();
        }
    }
}

void CursorState::UpdateHalfEdgeSelectTool()
{
    InputModule* Input = InputModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    auto& HalfEdgeMeshVec = EditorScenePtr->GetHalfEdgeMeshes();
    
    Vec2i MousePos = Input->GetMouseState().GetMousePos();

    if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
    {
        Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

        ISelectedObject* closestSelectedObject = nullptr;
        RayCastHit closestHit;

        for (auto& M : HalfEdgeMeshVec)
        {
            if (Select == SelectMode::VertSelect)
            {
                ISelectedObject* newSelectedObject = nullptr;

                RayCastHit hit = M->ClickCastVerts(MouseRay, newSelectedObject);
                if (hit.hit && hit.hitDistance < closestHit.hitDistance)
                {
                    if (closestSelectedObject)
                    {
                        delete closestSelectedObject;
                        closestSelectedObject = nullptr;
                    }
                    closestHit = hit;
                    closestSelectedObject = newSelectedObject;
                }
                else
                {
                    delete newSelectedObject;
                }
            }
            else if (Select == SelectMode::FaceSelect)
            {
                ISelectedObject* newSelectedObject = nullptr;
                RayCastHit hit = M->ClickCastFaces(MouseRay, newSelectedObject);
                if (hit.hit && hit.hitDistance < closestHit.hitDistance)
                {
                    if (closestSelectedObject)
                    {
                        delete closestSelectedObject;
                        closestSelectedObject = nullptr;
                    }
                    closestHit = hit;
                    closestSelectedObject = newSelectedObject;
                }
                else
                {
                    delete newSelectedObject;
                }
            }
        }

        bool HoldingShift = Input->GetKeyState(Key::Shift).pressed;

        if (closestSelectedObject)
        {
            closestSelectedObject->SetScene(EditorScenePtr);

            if (!HoldingShift)
            {
                UnselectSelectedObjects();
            }

            AddToSelectedObjects(closestSelectedObject);
        }
        else if (!HoldingShift)
        {
            UnselectSelectedObjects();
        }
    }
}

void CursorState::UpdateTranslateTool()
{
    InputModule* Input = InputModule::Get();
    if (!Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed
        || SelectedObjects.empty())
    {
        Axis = EditingAxis::None;
        UpdateSelectTool();
        return;
    }

    Vec2i MousePos = Input->GetMouseState().GetMousePos();
    Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

    CollisionModule* Collisions = CollisionModule::Get();

    Vec3f SelectedPos = SelectedProxyTransform.GetPosition();
    //Vec3f SelectedPos = SelectedObjects[0]->GetTransform()->GetPosition();

    if (Axis == EditingAxis::None && Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed)
    {
        RayCastHit ClosestHit;
        if (RayCastHit XHit = Collisions->RayCast(MouseRay, *XAxisTrans); XHit.hit)
        {
            ClosestHit = XHit;
            Axis = EditingAxis::X;
            ObjectRelativeHitPoint = XHit.hitPoint - SelectedPos;
        }
        if (RayCastHit YHit = Collisions->RayCast(MouseRay, *YAxisTrans); YHit.hit && YHit.hitDistance < ClosestHit.hitDistance)
        {
            ClosestHit = YHit;
            Axis = EditingAxis::Y;
            ObjectRelativeHitPoint = YHit.hitPoint - SelectedPos;
        }
        if (RayCastHit ZHit = Collisions->RayCast(MouseRay, *ZAxisTrans); ZHit.hit && ZHit.hitDistance < ClosestHit.hitDistance)
        {
            ClosestHit = ZHit;
            Axis = EditingAxis::Z;
            ObjectRelativeHitPoint = ZHit.hitPoint - SelectedPos;
        }
        if (RayCastHit OmniHit = Collisions->RayCast(MouseRay, *TransBall); OmniHit.hit && OmniHit.hitDistance < ClosestHit.hitDistance)
        {
            ClosestHit = OmniHit;
            Axis = EditingAxis::Omni;
            ObjectRelativeHitPoint = OmniHit.hitPoint - SelectedPos;
            ObjectDistanceAtHit = Math::magnitude(OmniHit.hitPoint - CameraPtr->GetPosition());
        }
        InitialObjectPosition = SelectedPos;
    }
    else if (Axis == EditingAxis::Omni)
    {
        int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
        if (DeltaMouseWheel > 0)
        {
            ObjectDistanceAtHit += 0.5f;
        }
        else if (DeltaMouseWheel < 0)
        {
            ObjectDistanceAtHit -= 0.5f;
        }

        if (ObjectDistanceAtHit < 1.0f) ObjectDistanceAtHit = 1.0f;

        Vec3f NewObjPos = (MouseRay.point - ObjectRelativeHitPoint) +
            (MouseRay.direction * ObjectDistanceAtHit);

        if (ShouldSnapToGrid)
        {
            NewObjPos.x = Math::Round(NewObjPos.x, TransSnap);
            NewObjPos.y = Math::Round(NewObjPos.y, TransSnap);
            NewObjPos.z = Math::Round(NewObjPos.z, TransSnap);
        }

        SelectedProxyTransform.SetPosition(NewObjPos);

    }
    else if (Axis != EditingAxis::None)
    {
        Vec3f SlidingAxis;
        switch (Axis)
        {
        case EditingAxis::X:
            SlidingAxis = Vec3f(1.0f, 0.0f, 0.0f);
            break;
        case EditingAxis::Y:
            SlidingAxis = Vec3f(0.0f, 1.0f, 0.0f);
            break;
        case EditingAxis::Z:
            SlidingAxis = Vec3f(0.0f, 0.0f, 1.0f);
            break;
        default:
            break;
        }

        Line MouseLine(MouseRay.point - ObjectRelativeHitPoint, MouseRay.direction);
        Line AxisLine(SelectedPos, SlidingAxis);

        Vec3f PointAlongAxis = Math::ClosestPointsOnLines(MouseLine, AxisLine).second;

        // Snap based on distance from initial selection position
        Vec3f ProxyToInitialObjectPosition = PointAlongAxis - InitialObjectPosition;
        
        if (ShouldSnapToGrid)
        {
            PointAlongAxis.x = Math::Round(ProxyToInitialObjectPosition.x, TransSnap) + InitialObjectPosition.x;
            PointAlongAxis.y = Math::Round(ProxyToInitialObjectPosition.y, TransSnap) + InitialObjectPosition.y;
            PointAlongAxis.z = Math::Round(ProxyToInitialObjectPosition.z, TransSnap) + InitialObjectPosition.z;
        }

        if (Input->IsKeyDown(Key::Shift))
        {
            // Snap to geometry along the axis

            CollisionModule* Collisions = CollisionModule::Get();
            
            Vec3f SelectionPos = SelectedProxyTransform.GetPosition();

            // Determine which direction along the axis we should snap to based on camera angle
            Vec3f SnappingDirection = Math::dot(CameraPtr->GetDirection(), SlidingAxis) > 0 ? SlidingAxis : -SlidingAxis;

            // Ignore the selected objects in the raycast so we don't accidentally snap to ourselves
            std::vector<Model*> IgnoredModels;
            std::vector<he::HalfEdgeMesh*> IgnoredMeshes;
            for (auto& SelectedObj : SelectedObjects)
            {
                // TODO: Unify mesh/model collisions
                if (SelectedModel* SModel = dynamic_cast<SelectedModel*>(SelectedObj.second))
                {
                    IgnoredModels.push_back(SModel->ModelPtr);
                }
                if (he::SelectedHalfEdgeMesh* SHMesh = dynamic_cast<he::SelectedHalfEdgeMesh*>(SelectedObj.second))
                {
                    IgnoredMeshes.push_back(SHMesh->m_HalfEdgeMesh);
                }
                if (he::SelectedHalfEdgeFace* SHFace = dynamic_cast<he::SelectedHalfEdgeFace*>(SelectedObj.second))
                {
                    IgnoredMeshes.push_back(SHFace->m_HalfEdgeMesh);
                }
                if (he::SelectedHalfEdgeVertex* SHVert = dynamic_cast<he::SelectedHalfEdgeVertex*>(SelectedObj.second))
                {
                    IgnoredMeshes.push_back(SHVert->m_HalfEdgeMesh);
                }
            }

            SceneRayCastHit Hit = EditorScenePtr->RayCast(Ray(SelectionPos + (SnappingDirection * -0.001f), SnappingDirection), IgnoredModels, IgnoredMeshes);

            if (Hit.rayCastHit.hit)
            {
                PointAlongAxis = Hit.rayCastHit.hitPoint;
            }
        }

        SelectedProxyTransform.SetPosition(PointAlongAxis);
    }

    if (Axis == EditingAxis::None)
    {
        UpdateSelectTool();
    }
}

void CursorState::UpdateRotateTool()
{
    InputModule* Input = InputModule::Get();
    if (!Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed
        || SelectedObjects.empty())
    {
        Axis = EditingAxis::None;
        UpdateSelectTool();
        return;
    }

    CollisionModule* Collisions = CollisionModule::Get();

    Vec2i MousePos = Input->GetMouseState().GetMousePos();
    Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

    if (Axis == EditingAxis::None && Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed)
    {
        Plane AxisPlane;
        Vec3f PerpUnitVec;

        AxisPlane.center = SelectedProxyTransform.GetPosition();

        RayCastHit ClosestHit;
        if (RayCastHit XHit = Collisions->RayCast(MouseRay, *XAxisRot); XHit.hit)
        {
            ClosestHit = XHit;
            Axis = EditingAxis::X;

            AxisPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
            PerpUnitVec = Vec3f(0.0f, 0.0f, 1.0f);
        }
        if (RayCastHit YHit = Collisions->RayCast(MouseRay, *YAxisRot); YHit.hit && YHit.hitDistance < ClosestHit.hitDistance)
        {
            ClosestHit = YHit;
            Axis = EditingAxis::Y;

            AxisPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);
            PerpUnitVec = Vec3f(1.0f, 0.0f, 0.0f);
        }
        if (RayCastHit ZHit = Collisions->RayCast(MouseRay, *ZAxisRot); ZHit.hit && ZHit.hitDistance < ClosestHit.hitDistance)
        {
            ClosestHit = ZHit;
            Axis = EditingAxis::Z;

            AxisPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);
            PerpUnitVec = Vec3f(0.0f, 1.0f, 0.0f);
        }

        if (ClosestHit.hit)
        {
            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, AxisPlane);

            Vec3f ObjectCenter = SelectedProxyTransform.GetPosition();
            Vec3f HitPoint = PlaneHit.hitPoint;
            Vec3f AngleVec = HitPoint - ObjectCenter;
            float Dot = Math::dot(AngleVec, PerpUnitVec);
            float Det = Math::dot(AxisPlane.normal, (Math::cross(AngleVec, PerpUnitVec)));

            InitialAngle = atan2(Det, Dot);
            ObjectInitialRotation = SelectedProxyTransform.GetRotation();

            Engine::DEBUGPrint("Initial Angle: " + std::to_string(InitialAngle));
        }
    }
    else if (Axis != EditingAxis::None)
    {
        Plane AxisPlane;
        Vec3f PerpUnitVec;

        AxisPlane.center = SelectedProxyTransform.GetPosition();

        if (Axis == EditingAxis::X)
        {
            AxisPlane.normal = Vec3f(1.0f, 0.0f, 0.0f);
            PerpUnitVec = Vec3f(0.0f, 0.0f, 1.0f);
        }
        else if (Axis == EditingAxis::Y)
        {
            AxisPlane.normal = Vec3f(0.0f, 1.0f, 0.0f);
            PerpUnitVec = Vec3f(1.0f, 0.0f, 0.0f);
        }
        else if (Axis == EditingAxis::Z)
        {
            AxisPlane.normal = Vec3f(0.0f, 0.0f, 1.0f);
            PerpUnitVec = Vec3f(0.0f, 1.0f, 0.0f);
        }

        RayCastHit PlaneHit = Collisions->RayCast(MouseRay, AxisPlane);

        Vec3f ObjectCenter = SelectedProxyTransform.GetPosition();
        Vec3f HitPoint = PlaneHit.hitPoint;
        Vec3f AngleVec = HitPoint - ObjectCenter;
        float Dot = Math::dot(AngleVec, PerpUnitVec);
        float Det = Math::dot(AxisPlane.normal, (Math::cross(AngleVec, PerpUnitVec)));

        float CurrentAngle = atan2(Det, Dot);

        float DeltaAngle = InitialAngle - CurrentAngle;

        if (ShouldSnapToGrid)
        {
            DeltaAngle = Math::Round(DeltaAngle, RotSnap);
        }

        Quaternion axisQuat = Quaternion(AxisPlane.normal, DeltaAngle);

        SelectedProxyTransform.SetRotation(axisQuat * ObjectInitialRotation);
    }

    if (Axis == EditingAxis::None)
    {
        UpdateSelectTool();
    }
}

void CursorState::UpdateScaleTool()
{
    InputModule* Input = InputModule::Get();
    if (!Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed
        || SelectedObjects.empty())
    {
        // Reset scale widget modifier when not actively scaling to prevent it getting stuck at a smaller/larger size after a scale operation
        ScaleWidgetModifier = 1.0f;
        Axis = EditingAxis::None;
        UpdateSelectTool();
        return;
    }

    CollisionModule* Collisions = CollisionModule::Get();

    Vec3f SelectedPos = SelectedProxyTransform.GetPosition();

    Vec2i MousePos = Input->GetMouseState().GetMousePos();
    Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

    if (Axis == EditingAxis::None && Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed)
    {
        RayCastHit ClosestHit;

        if (RayCastHit RingHit = Collisions->RayCast(MouseRay, *ScaleRing); RingHit.hit)
        {
            ClosestHit = RingHit;
            Axis = EditingAxis::Omni;
            ObjectRelativeHitPoint = RingHit.hitPoint - SelectedPos;

            // Temp
            InitialScale = SelectedProxyTransform.GetScale().x;

            Line MouseLine(MouseRay.point, MouseRay.direction);

            Vec3f ClosestPointFromMouseRayToObjCenter = Math::ClosestPointOnLineToPoint(MouseLine, SelectedPos);
            InitialDistFromObjectCenter = (ClosestPointFromMouseRayToObjCenter - SelectedPos).Magnitude();
        }

    }
    else if (Axis == EditingAxis::Omni)
    {
        Line MouseLine(MouseRay.point, MouseRay.direction);

        Vec3f ClosestPointFromMouseRayToObjCenter = Math::ClosestPointOnLineToPoint(MouseLine, SelectedPos);

        float DistFromObjectCenter = (ClosestPointFromMouseRayToObjCenter - SelectedPos).Magnitude();

        float ScaleFactor = DistFromObjectCenter / InitialDistFromObjectCenter;

        SelectedProxyTransform.SetScale(InitialScale * ScaleFactor);
        ScaleWidgetModifier = ScaleFactor;
        //ScaleRing->GetTransform().SetScale(DistDiff + 1.0f);
    }

    if (Axis == EditingAxis::None)
    {
        UpdateSelectTool();
    }

}

void CursorState::UpdateBoxTool()
{
    InputModule* Input = InputModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    KeyState ClickState = Input->GetMouseState().GetMouseButtonState(MouseButton::LMB);

    if (Dragging != DraggingMode::None)
    {
        IsCreatingNewBox = false;
    }

    if (IsCreatingNewBox)
    {
        if (ClickState.justReleased)
        {
            IsCreatingNewBox = false;

            if (BoxBeingCreated.XSize() <= 0.0001f
                || BoxBeingCreated.YSize() <= 0.0001f
                || BoxBeingCreated.ZSize() < 0.0001f)
            {
                // Box too smol
            }
            else
            {
                // Create the box model, add to scene
                EditorScenePtr->AddModel(new Model(Graphics->CreateBoxModel(BoxBeingCreated)));
            }

        }
        else
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(NewBoxStartPoint, Vec3f(0.0f, 0.0f, 1.0f)));

            if (PlaneHit.hit)
            {
                int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
                if (DeltaMouseWheel > 0 || Input->GetKeyState(Key::Plus).justPressed)
                {
                    NewBoxHeight += GeoPlaceSnap;
                }
                else if (DeltaMouseWheel < 0 || Input->GetKeyState(Key::Minus).justPressed)
                {
                    NewBoxHeight -= GeoPlaceSnap;
                }

                if (NewBoxHeight < GeoPlaceSnap) NewBoxHeight = GeoPlaceSnap;

                Vec3f HitPoint = PlaneHit.hitPoint;

                HitPoint.x = Math::Round(HitPoint.x, GeoPlaceSnap);
                HitPoint.y = Math::Round(HitPoint.y, GeoPlaceSnap);
                HitPoint.z = Math::Round(HitPoint.z, GeoPlaceSnap);

                float minX = std::min(HitPoint.x, NewBoxStartPoint.x);
                float minY = std::min(HitPoint.y, NewBoxStartPoint.y);

                float maxX = std::max(HitPoint.x, NewBoxStartPoint.x);
                float maxY = std::max(HitPoint.y, NewBoxStartPoint.y);

                BoxBeingCreated.min = Vec3f(minX, minY, HitPoint.z);
                BoxBeingCreated.max = Vec3f(maxX, maxY, HitPoint.z + NewBoxHeight);

                Graphics->DebugDrawAABB(BoxBeingCreated, Vec3f(0.1f, 1.0f, 0.3f));
            }
        }
    }
    else
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        if (ClickState.justPressed && Dragging == DraggingMode::None && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
        {
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
            RayCastHit SceneHit = EditorScenePtr->RayCast(MouseRay).rayCastHit;

            RayCastHit FinalHit = PlaneHit.hitDistance < SceneHit.hitDistance ? PlaneHit : SceneHit;

            if (FinalHit.hit)
            {
                Vec3f HitPoint = FinalHit.hitPoint;

                HitPoint.x = Math::Round(HitPoint.x, GeoPlaceSnap);
                HitPoint.y = Math::Round(HitPoint.y, GeoPlaceSnap);
                HitPoint.z = Math::Round(HitPoint.z, GeoPlaceSnap);

                IsCreatingNewBox = true;
                NewBoxStartPoint = HitPoint;
            }
        }
    }

}

void CursorState::UpdatePlaneTool()
{
    InputModule* Input = InputModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    KeyState ClickState = Input->GetMouseState().GetMouseButtonState(MouseButton::LMB);

    if (Dragging != DraggingMode::None)
    {
        IsCreatingNewPlane = false;
    }

    if (IsCreatingNewPlane)
    {
        if (ClickState.justReleased)
        {
            Model* NewPlane = new Model(Graphics->CreatePlaneModel(Vec2f(NewPlaneMin.x, NewPlaneMin.y), Vec2f(NewPlaneMax.x, NewPlaneMax.y), NewPlaneMin.z, NewPlaneSubdivisions));
            EditorScenePtr->AddModel(NewPlane);
            IsCreatingNewPlane = false;
        }
        else
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane{ NewPlaneStartPoint, Vec3f(0.0f, 0.0f, 1.0f) });

            int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
            if (DeltaMouseWheel > 0)
            {
                NewPlaneSubdivisions += 1;
            }
            else if (DeltaMouseWheel < 0)
            {
                NewPlaneSubdivisions -= 1;
                if (NewPlaneSubdivisions < 1)
                {
                    NewPlaneSubdivisions = 1;
                }
            }

            if (Input->GetKeyState(Key::Plus).justPressed)
            {
                NewPlaneSubdivisions += 1;
            }
            if (Input->GetKeyState(Key::Minus).justPressed)
            {
                NewPlaneSubdivisions -= 1;
                if (NewPlaneSubdivisions < 1)
                {
                    NewPlaneSubdivisions = 1;
                }
            }

            Vec3f OriginalHitPoint = PlaneHit.hitPoint;

            PlaneHit.hitPoint.x = Math::Round(PlaneHit.hitPoint.x, GeoPlaceSnap);
            PlaneHit.hitPoint.y = Math::Round(PlaneHit.hitPoint.y, GeoPlaceSnap);
            PlaneHit.hitPoint.z = Math::Round(PlaneHit.hitPoint.z, GeoPlaceSnap);

            float minX = std::min(PlaneHit.hitPoint.x, NewPlaneStartPoint.x);
            float minY = std::min(PlaneHit.hitPoint.y, NewPlaneStartPoint.y);

            float maxX = std::max(PlaneHit.hitPoint.x, NewPlaneStartPoint.x);
            float maxY = std::max(PlaneHit.hitPoint.y, NewPlaneStartPoint.y);

            NewPlaneMin = Vec3f(minX, minY, PlaneHit.hitPoint.z);
            NewPlaneMax = Vec3f(maxX, maxY, PlaneHit.hitPoint.z);

            Vec3f Green = Vec3f(0.1f, 1.0f, 0.3f);

            Vec3f SouthWest = NewPlaneMin;
            Vec3f NorthWest = Vec3f(minX, maxY, PlaneHit.hitPoint.z);
            Vec3f NorthEast = NewPlaneMax;
            Vec3f SouthEast = Vec3f(maxX, minY, PlaneHit.hitPoint.z);

            for (int i = 0; i < NewPlaneSubdivisions + 1; ++i)
            {
                Vec3f HorizonalLeft = SouthWest + ((float)i * ((NorthWest - SouthWest) / (float)NewPlaneSubdivisions));
                Vec3f HorizontalRight = SouthEast + ((float)i * ((NorthEast - SouthEast) / (float)NewPlaneSubdivisions));

                Vec3f VerticalBottom = SouthWest + ((float)i * ((SouthEast - SouthWest) / (float)NewPlaneSubdivisions));
                Vec3f VerticalTop = NorthWest + ((float)i * ((NorthEast - NorthWest) / (float)NewPlaneSubdivisions));

                Graphics->DebugDrawLine(HorizonalLeft, HorizontalRight, Green);
                Graphics->DebugDrawLine(VerticalBottom, VerticalTop, Green);
            }

            //graphics.DebugDrawAABB(aabbBox, Vec3f(0.1f, 1.0f, 0.3f));
            //graphics.DebugDrawLine(originalHitPoint, hit.hitPoint, Vec3f(1.0f, 0.5f, 0.5f));
        }

    }
    else
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        if (ClickState.justPressed && Dragging == DraggingMode::None && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
        {
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
            RayCastHit SceneHit = EditorScenePtr->RayCast(MouseRay).rayCastHit;

            RayCastHit FinalHit = PlaneHit.hitDistance < SceneHit.hitDistance ? PlaneHit : SceneHit;

            if (FinalHit.hit)
            {
                Vec3f HitPoint = FinalHit.hitPoint;

                HitPoint.x = Math::Round(HitPoint.x, GeoPlaceSnap);
                HitPoint.y = Math::Round(HitPoint.y, GeoPlaceSnap);
                HitPoint.z = Math::Round(HitPoint.z, GeoPlaceSnap);

                IsCreatingNewPlane = true;
                NewPlaneStartPoint = HitPoint;
            }
        }
    }
}

void CursorState::UpdateHalfEdgeTool()
{
    // Copied from box create... likely buggy
    InputModule* Input = InputModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    KeyState ClickState = Input->GetMouseState().GetMouseButtonState(MouseButton::LMB);

    if (Dragging != DraggingMode::None)
    {
        IsCreatingNewBox = false;
    }

    if (IsCreatingNewBox)
    {
        if (ClickState.justReleased)
        {
            IsCreatingNewBox = false;

            if (BoxBeingCreated.XSize() <= 0.0001f
                || BoxBeingCreated.YSize() <= 0.0001f
                || BoxBeingCreated.ZSize() < 0.0001f)
            {
                // Box too smol
            }
            else
            {
                // Create the half edge mesh, add to scene
                
                he::HalfEdgeMesh* newHeMesh = new he::HalfEdgeMesh();
                newHeMesh->MakeAABB(BoxBeingCreated, Graphics->m_DebugMaterial);

                EditorScenePtr->AddHalfEdgeMesh(newHeMesh);
            }

        }
        else
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(NewBoxStartPoint, Vec3f(0.0f, 0.0f, 1.0f)));

            if (PlaneHit.hit)
            {
                int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
                if (DeltaMouseWheel > 0 || Input->GetKeyState(Key::Plus).justPressed)
                {
                    NewBoxHeight += GeoPlaceSnap;
                }
                else if (DeltaMouseWheel < 0 || Input->GetKeyState(Key::Minus).justPressed)
                {
                    NewBoxHeight -= GeoPlaceSnap;
                }

                if (NewBoxHeight < GeoPlaceSnap) NewBoxHeight = GeoPlaceSnap;

                Vec3f HitPoint = PlaneHit.hitPoint;

                HitPoint.x = NewBoxStartPoint.x + Math::Round(HitPoint.x - NewBoxStartPoint.x, GeoPlaceSnap);
                HitPoint.y = NewBoxStartPoint.y + Math::Round(HitPoint.y - NewBoxStartPoint.y, GeoPlaceSnap);
                HitPoint.z = NewBoxStartPoint.z + Math::Round(HitPoint.z - NewBoxStartPoint.z, GeoPlaceSnap);

                float minX = std::min(HitPoint.x, NewBoxStartPoint.x);
                float minY = std::min(HitPoint.y, NewBoxStartPoint.y);

                float maxX = std::max(HitPoint.x, NewBoxStartPoint.x);
                float maxY = std::max(HitPoint.y, NewBoxStartPoint.y);

                BoxBeingCreated.min = Vec3f(minX, minY, HitPoint.z);
                BoxBeingCreated.max = Vec3f(maxX, maxY, HitPoint.z + NewBoxHeight);

                Graphics->DebugDrawAABB(BoxBeingCreated, Vec3f(0.1f, 1.0f, 0.3f));
            }
        }
    }
    else
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        if (ClickState.justPressed && Dragging == DraggingMode::None && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
        {
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
            RayCastHit SceneHit = EditorScenePtr->RayCast(MouseRay).rayCastHit;

            //RayCastHit FinalHit = PlaneHit.hitDistance < SceneHit.hitDistance ? PlaneHit : SceneHit;
            RayCastHit FinalHit = SceneHit.hit ? SceneHit : PlaneHit;

            if (FinalHit.hit)
            {
                Vec3f HitPoint = FinalHit.hitPoint;

                Vec3f HitNormal = FinalHit.hitNormal;

                bool xAligned = abs(Math::dot(HitNormal, Vec3f(1.0f, 0.0f, 0.0f))) > 0.99f;
                bool yAligned = abs(Math::dot(HitNormal, Vec3f(0.0f, 1.0f, 0.0f))) > 0.99f;
                bool zAligned = abs(Math::dot(HitNormal, Vec3f(0.0f, 0.0f, 1.0f))) > 0.99f;

                if (xAligned || yAligned || zAligned)
                {
                    if (!xAligned) HitPoint.x = Math::Round(HitPoint.x, GeoPlaceSnap);
                    if (!yAligned) HitPoint.y = Math::Round(HitPoint.y, GeoPlaceSnap);
                    if (!zAligned) HitPoint.z = Math::Round(HitPoint.z, GeoPlaceSnap);
                }

                //HitPoint.x = Math::Round(HitPoint.x, GeoPlaceSnap);
                //HitPoint.y = Math::Round(HitPoint.y, GeoPlaceSnap);
                //HitPoint.z = Math::Round(HitPoint.z, GeoPlaceSnap);

                IsCreatingNewBox = true;
                NewBoxStartPoint = HitPoint;
            }
        }
    }
}

void CursorState::UpdateWaterTool()
{
    InputModule* Input = InputModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    KeyState ClickState = Input->GetMouseState().GetMouseButtonState(MouseButton::LMB);

    if (Dragging != DraggingMode::None)
    {
        IsCreatingNewWater = false;
    }

    if (IsCreatingNewWater)
    {
        if (ClickState.justReleased)
        {
            IsCreatingNewWater = false;
            if (BoxBeingCreated.XSize() <= 0.0001f
                || BoxBeingCreated.YSize() <= 0.0001f)
            {
                // Box too smol
            }
            else
            {
                // Create the water volume, add to scene
                //EditorScenePtr->AddWaterVolume(new WaterVolume(BoxBeingCreated));
            }
        }
        else
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());
            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(NewBoxStartPoint, Vec3f(0.0f, 0.0f, 1.0f)));
            if (PlaneHit.hit)
            {
                int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
                if (DeltaMouseWheel > 0 || Input->GetKeyState(Key::Plus).justPressed)
                {
                    NewBoxHeight += GeoPlaceSnap;
                }
                else if (DeltaMouseWheel < 0 || Input->GetKeyState(Key::Minus).justPressed)
                {
                    NewBoxHeight -= GeoPlaceSnap;
                }

                if (NewBoxHeight < GeoPlaceSnap) NewBoxHeight = GeoPlaceSnap;

                Vec3f HitPoint = PlaneHit.hitPoint;
                HitPoint.x = NewBoxStartPoint.x + Math::Round(HitPoint.x - NewBoxStartPoint.x, GeoPlaceSnap);
                HitPoint.y = NewBoxStartPoint.y + Math::Round(HitPoint.y - NewBoxStartPoint.y, GeoPlaceSnap);
                HitPoint.z = NewBoxStartPoint.z + Math::Round(HitPoint.z - NewBoxStartPoint.z, GeoPlaceSnap);
                float minX = std::min(HitPoint.x, NewBoxStartPoint.x);
                float minY = std::min(HitPoint.y, NewBoxStartPoint.y);
                float maxX = std::max(HitPoint.x, NewBoxStartPoint.x);
                float maxY = std::max(HitPoint.y, NewBoxStartPoint.y);
                BoxBeingCreated.min = Vec3f(minX, minY, HitPoint.z);
                BoxBeingCreated.max = Vec3f(maxX, maxY, HitPoint.z + NewBoxHeight);
                Graphics->DebugDrawAABB(BoxBeingCreated, Vec3f(0.1f, 0.3f, 1.0f));
            }
        }
    }
    else {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        if (ClickState.justPressed && Dragging == DraggingMode::None && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
        {
            Ray MouseRay = EditorStatePtr->GetMouseRay(*CameraPtr, MousePos, EditorStatePtr->GetEditorSceneViewportRect());
            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
            RayCastHit SceneHit = EditorScenePtr->RayCast(MouseRay).rayCastHit;
            RayCastHit FinalHit = PlaneHit.hitDistance < SceneHit.hitDistance ? PlaneHit : SceneHit;
            if (FinalHit.hit)
            {
                Vec3f HitPoint = FinalHit.hitPoint;
                HitPoint.x = Math::Round(HitPoint.x, GeoPlaceSnap);
                HitPoint.y = Math::Round(HitPoint.y, GeoPlaceSnap);
                HitPoint.z = Math::Round(HitPoint.z, GeoPlaceSnap);
                IsCreatingNewWater = true;
                NewBoxStartPoint = HitPoint;
            }
        }
    }
}

void CursorState::UpdateSelectedObjects()
{
    for (auto& Object : SelectedObjects)
    {
        Object.second->Update();
    }
}

void CursorState::DrawSelectedObjects()
{
    for (auto& Object : SelectedObjects)
    {
        Object.second->Draw();
    }
}

void CursorState::DrawSelectedInspectorPanels()
{
    UIModule* UI = UIModule::Get();

    bool selectedObjectStateChanged = false;
    for (auto& Object : SelectedObjects)
    {
        selectedObjectStateChanged |= Object.second->DrawInspectorPanel();
        UI->NewLine(30.0f);
    }

    // The objects' transforms may have been changed by sliders in the inspector panel, so re-calculate proxy and offsets
    if (selectedObjectStateChanged)
    {
        RecalculateProxyAndObjectOffsets();
    }
}

void CursorState::DeleteSelectedObjects()
{
    for (auto& Object : SelectedObjects)
    {
        Object.second->DeleteObject();
        delete Object.second;
    }

    SelectedObjects.clear();
}

void CursorState::UnselectSelectedObjects()
{
    for (auto& Object : SelectedObjects)
    {
        delete Object.second;
    }

    SelectedObjects.clear();
}

void CursorState::AddToSelectedObjects(ISelectedObject* NewSelectedObject)
{
    for (auto& Obj : SelectedObjects)
    {
        ISelectedObject* itObject = Obj.second;

        if (*itObject == *NewSelectedObject)
        {
            delete NewSelectedObject;
            return;
        }
    }
    SelectedObjects.push_back(std::make_pair(OffsetInfo(), NewSelectedObject));

    RecalculateProxyAndObjectOffsets();
}

void CursorState::RecalculateProxyAndObjectOffsets()
{
    Vec3f AveragePos = Vec3f(0.0f, 0.0f, 0.0f);

    for (auto& Obj : SelectedObjects)
    {
        AveragePos += Obj.second->GetTransform()->GetPosition();
    }

    AveragePos /= (float)SelectedObjects.size();

    // Update selected object offsets
    for (auto& Obj : SelectedObjects)
    {
        Obj.first.Offset = AveragePos - Obj.second->GetTransform()->GetPosition();
    }

    SelectedProxyTransform.SetPosition(AveragePos);

    // Reset proxy rotation, determine quaternion difference of selected objects

    SelectedProxyTransform.SetRotation(Quaternion());

    for (auto& Obj : SelectedObjects)
    {
        Obj.first.RotationDiff = Obj.second->GetTransform()->GetRotation() * SelectedProxyTransform.GetRotation().Inverse();
    }

    // Reset scale
    for (auto& Obj : SelectedObjects)
    {
        Obj.first.InitialScale = Obj.second->GetTransform()->GetScale().x;
    }

    SelectedProxyTransform.SetScale(1.0f);
}

void CursorState::UpdateSelectedTransformsBasedOnProxy()
{
    Vec3f ProxyPos = SelectedProxyTransform.GetPosition();

    // Handle translation
    for (auto& Object : SelectedObjects)
    {
        Object.second->GetTransform()->SetPosition(ProxyPos - Object.first.Offset);
    }

    // Handle scaling
    if (SelectedObjects.size() == 1)
    {
        SelectedObjects[0].second->GetTransform()->SetScale(SelectedProxyTransform.GetScale() * SelectedObjects[0].first.InitialScale);
    }
    else
    {
        for (auto& Object : SelectedObjects)
        {
            // TODO: Non-uniform scaling
            float ProxyScale = SelectedProxyTransform.GetScale().x;

            Vec3f ObjOffset = Object.first.Offset;

            float OffsetDist = ObjOffset.Magnitude();

            Object.second->GetTransform()->SetPosition(ProxyPos - (ObjOffset * ProxyScale));

            Object.second->GetTransform()->SetScale(SelectedProxyTransform.GetScale() * Object.first.InitialScale);

            //Object.second->GetTransform()->SetScale(SelectedProxyTransform.GetScale());
        }
    }

    // Handle rotation
    for (auto& Object : SelectedObjects)
    {

        Object.second->GetTransform()->RotateAroundPoint(ProxyPos, SelectedProxyTransform.GetRotation());

        Object.second->GetTransform()->SetRotation(SelectedProxyTransform.GetRotation() * Object.first.RotationDiff);
    }


}

void CursorState::RotateSelectedTransforms(Quaternion Rotation)
{
    Vec3f ProxyPos = SelectedProxyTransform.GetPosition();


}

ISelectedObject* CursorState::ClickCastGeneric(Ray mouseRay)
{
    std::vector<IEditorClickable*>& clickables = EditorScenePtr->GetGenericEditorClickables();

    ISelectedObject* closestSelectedObject = nullptr;
    RayCastHit closestHit;

    for (IEditorClickable* clickable : clickables)
    {
        ISelectedObject* newSelectedObject = nullptr;
        RayCastHit newHit = clickable->ClickCast(mouseRay, newSelectedObject);

        if (newHit.hit && newHit.hitDistance < closestHit.hitDistance)
        {
            if (closestSelectedObject)
            {
                delete closestSelectedObject;
            }

            closestHit = newHit;
            closestSelectedObject = newSelectedObject;
        }
        else if (newSelectedObject)
        {
            delete newSelectedObject;
        }
    }

    return closestSelectedObject;
}

std::vector<ISelectedObject*> CursorState::ClickCastHalfEdgeMesh(Ray mouseRay)
{
    return std::vector<ISelectedObject*>();
}

bool CursorState::ClickCastApplyMaterial(Ray mouseRay, Material* material)
{
    std::vector<IEditorClickable*>& clickables = EditorScenePtr->GetGenericEditorClickables();

    ISelectedObject* closestSelectedObject = nullptr;
    RayCastHit closestHit;

    for (IEditorClickable* clickable : clickables)
    {
        ISelectedObject* newSelectedObject = nullptr;
        RayCastHit newHit = clickable->ClickCast(mouseRay, newSelectedObject);

        if (newHit.hit && newHit.hitDistance < closestHit.hitDistance)
        {
            if (closestSelectedObject)
            {
                delete closestSelectedObject;
            }

            closestHit = newHit;
            closestSelectedObject = newSelectedObject;
        }
        else if (newSelectedObject)
        {
            delete newSelectedObject;
        }
    }
    // Special case: if we hit a half edge mesh, we want to check if we hit a face and apply the material to that face instead of the whole mesh
    if (dynamic_cast<he::SelectedHalfEdgeMesh*>(closestSelectedObject))
    {
        delete closestSelectedObject;
        closestSelectedObject = nullptr;
        closestHit = RayCastHit();
        
        std::vector<he::HalfEdgeMesh*> heMeshes = EditorScenePtr->GetHalfEdgeMeshes();

        for (he::HalfEdgeMesh* heMesh : heMeshes)
        {
            ISelectedObject* newSelectedFace = nullptr;
            RayCastHit newHit = heMesh->ClickCastFaces(mouseRay, newSelectedFace);

            if (newHit.hit)
            {
                if (newHit.hitDistance < closestHit.hitDistance)
                {
                    if (closestSelectedObject)
                    {
                        delete closestSelectedObject;
                    }

                    closestHit = newHit;
                    closestSelectedObject = newSelectedFace;
                }
            }
            else if (newSelectedFace)
            {
                delete newSelectedFace;
            }
        }
    }

    if (closestSelectedObject)
    {
        closestSelectedObject->ApplyMaterial(*material);
        delete closestSelectedObject;
        return true;
    }
    return false;
}
void CursorState::ApplyMaterialToSelectedObjects(Material& material)
{
    for (auto& Object : SelectedObjects)
    {
        Object.second->ApplyMaterial(material);
    }
}
#endif