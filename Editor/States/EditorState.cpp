#include "EditorState.h"

#include <filesystem>

SelectedModel::SelectedModel(Model* InModel, Scene* InScene)
{
    ModelPtr = InModel;
    ScenePtr = InScene;

}

void SelectedModel::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collision = CollisionModule::Get();

    Graphics->DebugDrawAABB(Collision->GetCollisionMeshFromMesh(ModelPtr->m_TexturedMeshes[0].m_Mesh)->boundingBox, c_SelectedBoxColour, ModelPtr->GetTransform().GetTransformMatrix());
}

void SelectedModel::DrawInspectorPanel()
{
}

Transform* SelectedModel::GetTransform()
{
    return &ModelPtr->GetTransform();
}

void SelectedModel::DeleteObject()
{
    ScenePtr->DeleteModel(ModelPtr);
}

bool SelectedModel::operator==(const ISelectedObject& Other)
{
    const SelectedModel* OtherPtr = dynamic_cast<const SelectedModel*>(&Other);
    if (!OtherPtr)
    {
        return false;
    }
    else
    {
        return ModelPtr == OtherPtr->ModelPtr;
    }
}

SelectedLight::SelectedLight(PointLight* InPointLight, Scene* InScene)
{
    PointLightPtr = InPointLight;
    ScenePtr = InScene;

    Trans.SetPosition(PointLightPtr->position);
}

void SelectedLight::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    AABB LightAABB = AABB(PointLightPtr->position - Vec3f(0.35f, 0.35f, 0.35f), PointLightPtr->position + Vec3f(0.35f, 0.35f, 0.35f));

    Graphics->DebugDrawAABB(LightAABB, c_SelectedBoxColour);
}

void SelectedLight::Update()
{
    PointLightPtr->position = Trans.GetPosition();
}

void SelectedLight::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    Vec3f Pos = PointLightPtr->position;
    Vec3f Col = PointLightPtr->colour;

    static std::string TestString = "Test";

    UI->TextEntry("Test", TestString, Vec2f(100.0f, 20.0f), c_InspectorColour);

    UI->TextButton("X: " + std::to_string(Pos.x), Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);
    UI->TextButton("Y: " + std::to_string(Pos.y), Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);
    UI->TextButton("Z: " + std::to_string(Pos.z), Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

    UI->TextButton("R: " + std::to_string(Col.x), Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);
    UI->TextButton("G: " + std::to_string(Col.y), Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);
    UI->TextButton("B: " + std::to_string(Col.z), Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

}

Transform* SelectedLight::GetTransform()
{
    return &Trans;
}

void SelectedLight::DeleteObject()
{
    ScenePtr->DeletePointLight(PointLightPtr);
}

bool SelectedLight::operator==(const ISelectedObject& Other)
{
    const SelectedLight* OtherPtr = dynamic_cast<const SelectedLight*>(&Other);
    if (!OtherPtr)
    {
        return false;
    }
    else
    {
        return PointLightPtr == OtherPtr->PointLightPtr;
    }

}


inline CursorState::CursorState(EditorState* InEditorState, Scene* InEditorScene)
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
}

void CursorState::Update()
{
    InputModule* Input = InputModule::Get();
    UIModule* UI = UIModule::Get();

    if (Dragging == DraggingMode::NewModel)
    {
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            DraggingModelPtr->GetTransform().SetPosition(MouseRay.point + MouseRay.direction * 8.0f);

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
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            DraggingPointLightPtr->position = MouseRay.point + MouseRay.direction * 8.0f;
        
            Vec3f LightScalingIncrement = Vec3f(0.1f, 0.1f, 0.1f);
            
            int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
            if (DeltaMouseWheel > 0)
            {
                DraggingPointLightPtr->colour += LightScalingIncrement;
            }
            else if (DeltaMouseWheel < 0)
            {
                DraggingPointLightPtr->colour -= LightScalingIncrement;
            }
        }
        else
        {
            DraggingPointLightPtr = nullptr;

            Dragging = DraggingMode::None;
        }
    }

    else if (Dragging == DraggingMode::NewTexture)
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).pressed)
        {
            UI->ImgPanel(DraggingMaterialPtr->m_Albedo, Rect(Vec2f(MousePos), Vec2f(40.0f, 40.0f)));
        }
        else
        {
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            auto SceneHit = EditorScenePtr->RayCast(MouseRay);

            if (SceneHit.rayCastHit.hit)
            {
                SceneHit.hitModel->SetMaterial(*DraggingMaterialPtr);
            }

            DraggingMaterialPtr = nullptr;

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
        UpdateTranslateTool();
        break;
    case ToolMode::Geometry:
        UpdateGeometryTool();
        break;
    default:
    {
        break;
    }
    }

    if (!SelectedObjects.empty())
    {
        // Update widget positions
        Transform ObjTrans = *SelectedObjects[0]->GetTransform();

        float DistFromCam = Math::magnitude(ObjTrans.GetPosition() - EditorStatePtr->ViewportCamera.GetPosition());

        //XAxisTrans->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(5.0f, 0.0f, 0.0f));
        //YAxisTrans->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 5.0f, 0.0f));
        //ZAxisTrans->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 0.0f, 5.0f));

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

        XAxisScale->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(5.0f, 0.0f, 0.0f));
        YAxisScale->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 5.0f, 0.0f));
        ZAxisScale->GetTransform().SetPosition(ObjTrans.GetPosition() + Vec3f(0.0f, 0.0f, 5.0f));

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
    }


}

void CursorState::CycleToolMode()
{
    switch (Tool)
    {
    case ToolMode::Select:
        Tool = ToolMode::Transform;
        break;
    case ToolMode::Transform:
        Tool = ToolMode::Geometry;
        break;
    case ToolMode::Geometry:
        Tool = ToolMode::Vertex;
        break;
    case ToolMode::Vertex:
        Tool = ToolMode::Sculpt;
        break;
    case ToolMode::Sculpt:
        Tool = ToolMode::Select;
        break;
    default:
        Engine::FatalError("Invalid editor cursor tool mode.");
        break;
    }
}

void CursorState::CycleGeometryMode()
{
    switch (GeoMode)
    {
    case GeometryMode::Box:
        GeoMode = GeometryMode::Plane;
        break;
    case GeometryMode::Plane:
        GeoMode = GeometryMode::Box;
        break;
    default:
        Engine::FatalError("Invalid editor cursor geometry mode.");
        break;
    }
}

void CursorState::CycleTransformMode()
{
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
    Tool = InToolMode;
}

ToolMode CursorState::GetToolMode()
{
    return Tool;
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

void CursorState::StartDraggingNewMaterial(Material* NewMaterial)
{
    if (Dragging != DraggingMode::None)
    {
        return;
    }

    Dragging = DraggingMode::NewTexture;

    DraggingMaterialPtr = NewMaterial;
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
            Graphics->Draw(*XAxisScale);
            Graphics->Draw(*YAxisScale);
            Graphics->Draw(*ZAxisScale);
        }
    }
}

void CursorState::DrawInspectorPanel()
{
    UIModule* UI = UIModule::Get();

    if (SelectedObjects.empty())
    {
        UI->TextButton("No object selected.", Vec2f(120.0f, 60.0f), 4.0f);
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
    InputModule* Input = InputModule::Get();
    
    if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed)
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

        CollisionModule* Collisions = CollisionModule::Get();
        GraphicsModule* Graphics = GraphicsModule::Get();

        RayCastHit PointLightHit;

        PointLight* HitLight = nullptr;

        auto& Lights = EditorScenePtr->GetPointLights();

        for (PointLight* Light : Lights)
        {
            AABB LightAABB = AABB(Light->position - Vec3f(0.35f, 0.35f, 0.35f), Light->position + Vec3f(0.35f, 0.35f, 0.35f));
            RayCastHit NewPointLightHit = Collisions->RayCast(MouseRay, LightAABB);

            if (NewPointLightHit.hit && NewPointLightHit.hitDistance < PointLightHit.hitDistance)
            {
                PointLightHit = NewPointLightHit;
                HitLight = Light;
            }
        }

        auto ModelHit = EditorScenePtr->RayCast(MouseRay);

        bool HoldingShift = Input->GetKeyState(Key::Shift).pressed;

        if (ModelHit.rayCastHit.hit || PointLightHit.hit)
        {
            if (!HoldingShift)
            {
                UnselectSelectedObjects();
            }

            if (ModelHit.rayCastHit.hitDistance < PointLightHit.hitDistance)
            {
                if (ModelHit.hitModel != DraggingModelPtr)
                {
                    SelectedModel* EdModel = new SelectedModel(ModelHit.hitModel, EditorScenePtr);

                    AddToSelectedObjects(EdModel);
                }
            }
            else
            {
                if (HitLight != DraggingPointLightPtr)
                {
                    SelectedLight* EdLight = new SelectedLight(HitLight, EditorScenePtr);

                    AddToSelectedObjects(EdLight);
                }
            }
        }
    }
}

void CursorState::UpdateMoveTool()
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

    default:
        break;
    }
}

void CursorState::UpdateVertexTool()
{
}

void CursorState::UpdateSculptTool()
{
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
    Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

    CollisionModule* Collisions = CollisionModule::Get();

    Vec3f SelectedPos = SelectedObjects[0]->GetTransform()->GetPosition();

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
            ObjectDistanceAtHit = Math::magnitude(OmniHit.hitPoint - EditorStatePtr->ViewportCamera.GetPosition());
        }
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
    
        SelectedObjects[0]->GetTransform()->SetPosition(NewObjPos);
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

        //GraphicsModule::Get()->DebugDrawPoint(SelectedObject->GetTransform()->GetPosition() + ObjectRelativeHitPoint, Vec3f(1.0f, 0.0f, 0.0f));

        Line MouseLine(MouseRay.point - ObjectRelativeHitPoint, MouseRay.direction);
        Line AxisLine(SelectedPos, SlidingAxis);

        Vec3f PointAlongAxis = Math::ClosestPointsOnLines(MouseLine, AxisLine).second;

        SelectedObjects[0]->GetTransform()->SetPosition(PointAlongAxis);
    }

    if (Axis == EditingAxis::None)
    {
        UpdateSelectTool();
    }
}

void CursorState::UpdateRotateTool()
{
}

void CursorState::UpdateScaleTool()
{
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
            // Create the box model, add to scene
            IsCreatingNewBox = false;

            EditorScenePtr->AddModel(Graphics->CreateBoxModel(BoxBeingCreated));
        }
        else
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

            RayCastHit PlaneHit = Collisions->RayCast(MouseRay, Plane(NewBoxStartPoint, Vec3f(0.0f, 0.0f, 1.0f)));

            if (PlaneHit.hit)
            {
                int DeltaMouseWheel = Input->GetMouseState().GetDeltaMouseWheel();
                if (DeltaMouseWheel > 0)
                {
                    NewBoxHeight += GeoPlaceSnap;
                }
                else if (DeltaMouseWheel < 0)
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
        if (ClickState.justPressed && Dragging == DraggingMode::None)
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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
}

void CursorState::UpdateSelectedObjects()
{
    for (ISelectedObject* Object : SelectedObjects)
    {
        Object->Update();
    }
}

void CursorState::DrawSelectedObjects()
{
    for (ISelectedObject* Object : SelectedObjects)
    {
        Object->Draw();
    }
}

void CursorState::DrawSelectedInspectorPanels()
{
    for (ISelectedObject* Object : SelectedObjects)
    {
        Object->DrawInspectorPanel();
    }
}

void CursorState::DeleteSelectedObjects()
{
    for (ISelectedObject* Object : SelectedObjects)
    {
        Object->DeleteObject();
        delete Object;
    }

    SelectedObjects.clear();
}

void CursorState::UnselectSelectedObjects()
{
    for (ISelectedObject* Object : SelectedObjects)
    {
        delete Object;
    }

    SelectedObjects.clear();
}

void CursorState::AddToSelectedObjects(ISelectedObject* NewSelectedObject)
{
    for (ISelectedObject* Obj : SelectedObjects)
    {
        if (*Obj == *NewSelectedObject)
        {
            return;
        }
    }
    SelectedObjects.push_back(NewSelectedObject);
}

void EditorState::OnInitialized()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    CollisionModule* Collisions = CollisionModule::Get();
    TextModule* Text = TextModule::Get();
    InputModule* Input = InputModule::Get();

    LoadEditorResources();

    // Load user resources
    LoadedModels = LoadModels(*Graphics);
    LoadedMaterials = LoadMaterials(*Graphics);

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
    EditorScene.Init(*Graphics, *Collisions);
    EditorScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.0f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });
    EditorScene.SetCamera(&ViewportCamera);

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

    for (int i = 0; i < 1000; i++)
    {
        RandomSizes.push_back(Math::RandomFloat(120.0f, 200.0f));
        Vec3f Colour = Vec3f(Math::RandomFloat(0.0f, 1.0f), Math::RandomFloat(0.0f, 1.0f), Math::RandomFloat(0.0f, 1.0f));
        RandomColours.push_back(Colour);
    }
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
    UpdateEditor(DeltaTime);
}

void EditorState::OnResize()
{
    GraphicsModule* graphics = GraphicsModule::Get();
    InputModule* input = InputModule::Get();

    Rect ViewportRect = GetEditorSceneViewportRect();

    ViewportCamera.SetScreenSize(ViewportRect.size);
    graphics->ResizeGBuffer(ViewportBuffer, ViewportRect.size);
    input->SetMouseCenter(ViewportRect.Center());
}

void EditorState::UpdateEditor(float DeltaTime)
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

        EditorScene.SetDirectionalLight(DirectionalLight{ ViewportCamera.GetDirection(), Vec3f(1.0f, 1.0f, 1.0f) });

    }

    if (Engine::IsWindowFocused())
    {
        // Update tools
        // TODO(Fraser): This is heavily reliant on order since Update() calls UI functions - another reason to make the UI module use render commands
        Cursor.Update();
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

    // Keyboard hotkeys for switching tools
    if (Input->GetKeyState(Key::One).justPressed)
    {
        Cursor.SetToolMode(ToolMode::Select);
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


    Vec2i ViewportSize = Engine::GetClientAreaSize();

    EditorScene.Draw(*Graphics, ViewportBuffer);

    Graphics->SetActiveFrameBuffer(WidgetBuffer);

    Graphics->SetCamera(&ViewportCamera);

    Graphics->SetRenderMode(RenderMode::FULLBRIGHT);

    Cursor.DrawTransientModels();

    Graphics->ResetFrameBuffer();
    UI->BufferPanel(ViewportBuffer.FinalOutput, GetEditorSceneViewportRect());

    UI->BufferPanel(WidgetBuffer, GetEditorSceneViewportRect());

    DrawEditorUI();
}

void EditorState::UpdateGame(float DeltaTime)
{
}

Rect EditorState::GetEditorSceneViewportRect()
{
    Vec2f ViewportSize = Engine::GetClientAreaSize();

    Rect SceneViewportRect = Rect(  Vec2f(80.0f, 40.0f), 
                                    Vec2f(ViewportSize.x - 320.0f, ViewportSize.y * 0.7f - 40.0f));
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
    Texture RedTexture = *Registry->LoadTexture("textures/red.png");
    Texture GreenTexture = *Registry->LoadTexture("textures/green.png");
    Texture BlueTexture = *Registry->LoadTexture("textures/blue.png");
    Texture PurpleTexture = *Registry->LoadTexture("textures/purple.png");

    WhiteMaterial = Graphics->CreateMaterial(*Registry->LoadTexture("images/white.png"));

    // Create translation gizmo models
    xAxisArrow = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ArrowSmooth.obj"),
        Graphics->CreateMaterial(RedTexture)
    ));

    yAxisArrow = Graphics->CloneModel(xAxisArrow);
    zAxisArrow = Graphics->CloneModel(xAxisArrow);
    
    xAxisArrow.GetTransform().SetScale(Vec3f(0.2f, 1.0f, 0.2f));
    yAxisArrow.GetTransform().SetScale(Vec3f(0.2f, 1.0f, 0.2f));
    zAxisArrow.GetTransform().SetScale(Vec3f(0.2f, 1.0f, 0.2f));

    xAxisArrow.GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -M_PI_2));
    zAxisArrow.GetTransform().Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), M_PI_2));

    yAxisArrow.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zAxisArrow.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    TranslateBall = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/Buckyball.obj"),
        Graphics->CreateMaterial(PurpleTexture)
    ));

    // Create rotation gizmo models
    xAxisRing = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/RotationHoop.obj"),
        Graphics->CreateMaterial(RedTexture)
    ));

    yAxisRing = Graphics->CloneModel(xAxisRing);
    zAxisRing = Graphics->CloneModel(xAxisRing);

    xAxisRing.GetTransform().Rotate(Quaternion(Vec3f(0.0f, 0.0f, 1.0f), -M_PI_2));
    zAxisRing.GetTransform().Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), M_PI_2));

    yAxisRing.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zAxisRing.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Create scaling gizmo models
    xScaleWidget = Graphics->CreateModel(TexturedMesh(
        *Registry->LoadStaticMesh("models/ScaleWidget.obj"),
        Graphics->CreateMaterial(RedTexture)
    ));

    yScaleWidget = Graphics->CloneModel(xScaleWidget);
    zScaleWidget = Graphics->CloneModel(xScaleWidget);

    xScaleWidget.GetTransform().Rotate(Quaternion(Vec3f(0.0f, 1.0f, 0.0f), M_PI_2));
    yScaleWidget.GetTransform().Rotate(Quaternion(Vec3f(1.0f, 0.0f, 0.0f), -M_PI_2));

    xScaleWidget.GetTransform().SetScale(Vec3f(0.4f, 0.4f, 0.8f));
    yScaleWidget.GetTransform().SetScale(Vec3f(0.4f, 0.4f, 0.8f));
    zScaleWidget.GetTransform().SetScale(Vec3f(0.4f, 0.4f, 0.8f));

    yScaleWidget.SetMaterial(Graphics->CreateMaterial(GreenTexture));
    zScaleWidget.SetMaterial(Graphics->CreateMaterial(BlueTexture));

    // Load editor UI textures
    playButtonTexture = *Registry->LoadTexture("images/playButton.png");

    cursorToolTexture = *Registry->LoadTexture("images/cursorTool.png");
    boxToolTexture = *Registry->LoadTexture("images/boxTool.png");
    planeToolTexture = *Registry->LoadTexture("images/planeTool.png");

    translateToolTexture = *Registry->LoadTexture("images/translateTool.png");
    rotateToolTexture = *Registry->LoadTexture("images/rotateTool.png");
    scaleToolTexture = *Registry->LoadTexture("images/scaleTool.png");

    vertexToolTexture = *Registry->LoadTexture("images/vertexTool.png");
    sculptToolTexture = *Registry->LoadTexture("images/sculptTool.png");
    
    lightEntityTexture = *Registry->LoadTexture("images/lightTool.png");
    cameraEntityTexture = *Registry->LoadTexture("images/cameraButton.png");
    brainEntityTexture = *Registry->LoadTexture("images/brainTool.png");

    // Load editor fonts
    DefaultFont = Text->LoadFont("fonts/ARLRDBD.TTF", 30);
    InspectorFont = Text->LoadFont("fonts/ARLRDBD.TTF", 15);
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

void EditorState::MoveCamera(Camera* Camera, float PixelToRadians, double DeltaTime)
{
    InputModule* Input = InputModule::Get();

    const float CamSpeed = 10.0f;

    float Speed = CamSpeed * DeltaTime;

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

void EditorState::DrawEditorUI()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    // Left toolbar buttons
    Rect ToolbarButtonRect = Rect(Vec2f(0.0f, 40.0f), Vec2f(ViewportRect.location.x, ViewportRect.size.y));

    UI->StartFrame("Tools", ToolbarButtonRect, 0.0f, c_NicePink);
    {
        Vec3f SelectedColour = c_NiceGreen;
        Vec3f UnSelectedColour = c_NiceLighterBlue;

        if (UI->ImgButton("CursorTool", cursorToolTexture, Vec2f(80.0f, 80.0f), 12.0f, 
            Cursor.GetToolMode() == ToolMode::Select ? SelectedColour : UnSelectedColour))
        {
            Cursor.SetToolMode(ToolMode::Select);
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
    }
    UI->EndFrame();

    DrawTopPanel();
    DrawResourcesPanel();
    DrawInspectorPanel();
}

void EditorState::DrawTopPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect TopPanelRect = Rect(Vec2f(0.0f, 0.0f),
        Vec2f(ScreenSize.x, ViewportRect.location.y));

    Vec2f TopPanelButtonSize = Vec2f(ViewportRect.location.y, ViewportRect.location.y);

    UI->StartFrame("Top", TopPanelRect, 0.0f, c_NiceRed);
    {
        UI->ImgButton("PlayButton", playButtonTexture, TopPanelButtonSize, 8.0f);
        UI->TextButton("Open", TopPanelButtonSize, 8.0f);
    }
    UI->EndFrame();
}

void EditorState::DrawResourcesPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    // Draw meshes
    Graphics->SetCamera(&ModelCamera);

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect ResourcePanelRect = Rect(Vec2f(ViewportRect.location.x, ViewportRect.location.y + ViewportRect.size.y), Vec2f(ViewportRect.size.x, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y)));

    UI->StartFrame("Resources", ResourcePanelRect, 16.0f, c_NiceBlue);
    {
        UI->StartTab("Models", c_NicePurple);
        {
            int index = 0;
            for (auto& AModel : LoadedModels)
            {
                if (UI->TextButton(AModel.m_TexturedMeshes[0].m_Mesh.Path.GetFileNameNoExt(), Vec2f(120.0f, 40.0f), 10.0f, c_NiceYellow).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        Model* AddedModel = EditorScene.AddModel(AModel);
                        Cursor.StartDraggingNewModel(AddedModel);
                    }
                }
                index++;
            }
        }
        UI->EndTab();

        UI->StartTab("Materials", c_NicePurple);
        {
            for (auto& Mat : LoadedMaterials)
            {
                if (UI->ImgButton(Mat.m_Albedo.Path.GetFileNameNoExt(), Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_NiceYellow).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        Material* MatPtr = &Mat;
                        Cursor.StartDraggingNewMaterial(MatPtr);
                    }

                }
            }
        }
        UI->EndTab();

        UI->StartTab("Entities", c_NicePurple);
        {
            if (UI->ImgButton("LightEntity", lightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f).clicking)
            {
                if (!Cursor.IsDraggingSomething())
                {
                    PointLight* PointLightPtr = EditorScene.AddPointLight(PointLight());
                    Cursor.StartDraggingNewPointLight(PointLightPtr);
                }
            }
            if (UI->ImgButton("CameraEntity", cameraEntityTexture, Vec2f(80.0f, 80.0f), 12.0f))
            {
            }
            if (UI->ImgButton("BrainEntity", brainEntityTexture, Vec2f(80.0f, 80.0f), 12.0f))
            {
            }

        }
        UI->EndTab();

        UI->StartTab("Behaviours", c_NicePurple);
        {
            auto BehaviourMap = BehaviourRegistry::Get()->GetBehaviours();

            for (auto Behaviour : BehaviourMap)
            {
                if (UI->TextButton(Behaviour.first, Vec2f(120, 40), 10.0f, c_NiceYellow))
                {
                    
                }
            }
        }
        UI->EndTab();

        UI->StartTab("1000 Buttons", c_NicePurple);
        {
            for (int i = 0; i < 1000; ++i)
            {
                UI->TextButton("", Vec2f(20.0f, 20.0f), 4.0f, RandomColours[i]);
            }
        }
        UI->EndTab();

        UI->StartTab("Experiment", c_NicePurple);
        {
            
        }
        UI->EndTab();

    }
    UI->EndFrame();


}

void EditorState::DrawInspectorPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect InspectorPanelRect = Rect( Vec2f(ViewportRect.location.x + ViewportRect.size.x, ViewportRect.location.y), 
                                    Vec2f(ScreenSize.x - (ViewportRect.location.x + ViewportRect.size.x), ScreenSize.y - ViewportRect.location.y));

    UI->StartFrame("Inspector", InspectorPanelRect, 16.0f, c_NicePink);
    {
        Cursor.DrawInspectorPanel();
    }
    UI->EndFrame();

}

