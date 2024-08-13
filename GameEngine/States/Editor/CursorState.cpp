#include "CursorState.h"

#include "EditorState.h"


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
    UIModule* UI = UIModule::Get();

    float oldX = ModelPtr->GetTransform().GetPosition().x;
    float oldY = ModelPtr->GetTransform().GetPosition().y;
    float oldZ = ModelPtr->GetTransform().GetPosition().z;

    float newX = oldX;
    float newY = oldY;
    float newZ = oldZ;

    UI->FloatSlider("X", Vec2f(400.0f, 20.0f), newX, -10.0f, 10.0f);
    UI->FloatSlider("Y", Vec2f(400.0f, 20.0f), newY, -10.0f, 10.0f);
    UI->FloatSlider("Z", Vec2f(400.0f, 20.0f), newZ, -10.0f, 10.0f);

    if (newX != oldX || newY != oldY || newZ != oldZ)
    {
        ModelPtr->GetTransform().SetPosition(Vec3f(newX, newY, newZ));
        
    }
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

SelectedVertex::SelectedVertex(Vec3f* InVertPtr, Brush* InBrushPtr, Scene* InScenePtr)
{
    VertPtr = InVertPtr;
    BrushPtr = InBrushPtr;
    ScenePtr = InScenePtr;

    Trans.SetPosition(*VertPtr);
}

void SelectedVertex::Draw()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    AABB VertAABB = AABB(*VertPtr - Vec3f(0.35f, 0.35f, 0.35f), *VertPtr + Vec3f(0.35f, 0.35f, 0.35f));

    Graphics->DebugDrawAABB(VertAABB, c_SelectedBoxColour);
}

void SelectedVertex::Update()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    *VertPtr = Trans.GetPosition();


    if (!BrushPtr->UpdatedThisFrame)
    {
        Graphics->UpdateBrushModel(BrushPtr);
        //BrushPtr->UpdatedThisFrame = true;
    }
}

void SelectedVertex::DrawInspectorPanel()
{
}

Transform* SelectedVertex::GetTransform()
{
    return &Trans;
}

void SelectedVertex::DeleteObject()
{
    // TODO: can't delete individual brush vertices for now
    ScenePtr->DeleteBrush(BrushPtr);
}

bool SelectedVertex::operator==(const ISelectedObject& Other)
{
    const SelectedVertex* OtherPtr = dynamic_cast<const SelectedVertex*>(&Other);

    if (!OtherPtr)
    {
        return false;
    }
    else
    {
        return VertPtr == OtherPtr->VertPtr;
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

    static std::string ColourString = "Colour";
    static std::string IntensityString = "Intensity";

    UI->TextEntry("Colour", ColourString, Vec2f(250.0f, 20.0f), c_InspectorColour);

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), PointLightPtr->colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), PointLightPtr->colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), PointLightPtr->colour.b);

    UI->TextEntry("Intensity", IntensityString, Vec2f(250.0f, 20.0f), c_InspectorColour);

    UI->FloatSlider("Intensity", Vec2f(400.0f, 20.0f), PointLightPtr->intensity, 0.0f, 10.0f);
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

SelectedDirectionalLight::SelectedDirectionalLight(DirectionalLight* InDirLight, Scene* InScene)
{
}

void SelectedDirectionalLight::Draw()
{
}

void SelectedDirectionalLight::Update()
{
}

void SelectedDirectionalLight::DrawInspectorPanel()
{
}

Transform* SelectedDirectionalLight::GetTransform()
{
    return nullptr;
}

void SelectedDirectionalLight::DeleteObject()
{
}

bool SelectedDirectionalLight::operator==(const ISelectedObject& Other)
{
    return false;
}

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
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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
    else if (Dragging == DraggingMode::NewBehaviour)
    {
        Vec2i MousePos = Input->GetMouseState().GetMousePos();
        Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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
    case ToolMode::Brush:
        UpdateBrushTool();
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

        float DistFromCam = Math::magnitude(ObjTrans.GetPosition() - EditorStatePtr->ViewportCamera.GetPosition());

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

void CursorState::ResetAllState()
{
    Dragging = DraggingMode::None;

    DraggingModelPtr = nullptr;
    DraggingPointLightPtr = nullptr;
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

void CursorState::CycleSelectMode()
{
    ResetAllState();
    switch (Select)
    {
    case SelectMode::ModelSelect:
        Select = SelectMode::VertexSelect;
        break;
    case SelectMode::VertexSelect:
        Select = SelectMode::ModelSelect;
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

    // TEMP: edit directional light colour
    Colour dirLightColour = EditorScenePtr->m_DirLight.colour;
    Colour invDirlightColour = Colour(1.0f - dirLightColour.r, 1.0f - dirLightColour.g, 1.0f - dirLightColour.b);
    UI->TextButton("Directional Light Colour", Vec2f(250.0f, 20.0f), 8.0f, dirLightColour, invDirlightColour);

    UI->FloatSlider("R", Vec2f(400.0f, 20.0f), EditorScenePtr->m_DirLight.colour.r);
    UI->FloatSlider("G", Vec2f(400.0f, 20.0f), EditorScenePtr->m_DirLight.colour.g);
    UI->FloatSlider("B", Vec2f(400.0f, 20.0f), EditorScenePtr->m_DirLight.colour.b);

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
    case SelectMode::ModelSelect:
        UpdateModelSelectTool();
        break;
    case SelectMode::VertexSelect:
        UpdateVertexSelectTool();
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

    Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

    SceneRayCastHit FinalHit = EditorScenePtr->RayCast(MouseRay);

    if (FinalHit.rayCastHit.hit)
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

                StaticMesh_ID Mesh = PlaneModel->m_TexturedMeshes[0].m_Mesh.Id;

                std::vector<Vertex*> Vertices = Graphics->m_Renderer.MapMeshVertices(Mesh);

                for (auto& Vert : Vertices)
                {
                    float Dist = Math::magnitude(Vert->position - ModelSpaceVertPos);
                    //if (Dist < radius)
                    //{
                    float Strength = Math::SmoothStep(Dist, SculptRadius, 0.5f) * VerticalDir * (SculptRadius * 0.25f);
                    
                    Vec3f hitNorm = FinalHit.rayCastHit.hitNormal;
                    Vert->position += Strength * hitNorm * (float)DeltaTime;
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

                StaticMesh_ID Mesh = PlaneModel->m_TexturedMeshes[0].m_Mesh.Id;

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

void CursorState::UpdateBrushTool()
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
                Brush* NewBrush = new Brush(BoxBeingCreated);

                EditorScenePtr->AddBrush(NewBrush);
                Graphics->UpdateBrushModel(NewBrush);

                //EditorScenePtr->AddModel(NewBrush->RepModel);

                //EditorScenePtr->AddModel(Graphics->CreateBoxModel(BoxBeingCreated));
            }

        }
        else
        {
            Vec2i MousePos = Input->GetMouseState().GetMousePos();
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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

void CursorState::UpdateModelSelectTool()
{
    InputModule* Input = InputModule::Get();

    Vec2i MousePos = Input->GetMouseState().GetMousePos();

    if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
    {
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

void CursorState::UpdateVertexSelectTool()
{
    InputModule* Input = InputModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    Vec2i MousePos = Input->GetMouseState().GetMousePos();

    auto BrushVec = EditorScenePtr->GetBrushes();

    for (auto& B : BrushVec)
    {
        for (auto& Vert : B->Vertices)
        {
            AABB BrushVertAABB = AABB(Vert - Vec3f(0.15f, 0.15f, 0.15f), Vert + Vec3f(0.15f, 0.15f, 0.15f));
            Graphics->DebugDrawAABB(BrushVertAABB, Colour(0.7f, 0.9f, 0.3f));
        }
    }

    if (Input->GetMouseState().GetMouseButtonState(MouseButton::LMB).justPressed && EditorStatePtr->GetEditorSceneViewportRect().Contains(MousePos))
    {
        Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

        RayCastHit VertHit;

        Vec3f* HitVert = nullptr;
        Brush* HitBrush = nullptr;

        CollisionModule* Collisions = CollisionModule::Get();

        for (auto& B : BrushVec)
        {
            for (auto& Vert : B->Vertices)
            {
                AABB BrushVertAABB = AABB(Vert - Vec3f(0.15f, 0.15f, 0.15f), Vert + Vec3f(0.15f, 0.15f, 0.15f));

                RayCastHit NewVertHit = Collisions->RayCast(MouseRay, BrushVertAABB);

                if (NewVertHit.hit && NewVertHit.hitDistance < VertHit.hitDistance)
                {
                    VertHit = NewVertHit;
                    HitBrush = B;
                    HitVert = &Vert;
                }

            }
        }

        if (VertHit.hit && HitVert)
        {
            if (!Input->GetKeyState(Key::Shift).pressed)
            {
                UnselectSelectedObjects();
            }

            SelectedVertex* ClickedVert = new SelectedVertex(HitVert, HitBrush, EditorScenePtr);

            AddToSelectedObjects(ClickedVert);

            // Also add all vertices which were very close to the chosen vert
            for (auto& B : BrushVec)
            {
                for (auto& Vert : B->Vertices)
                {
                    if (Math::magnitude(*HitVert - Vert) < 0.00001f)
                    {
                        SelectedVertex* CloseVert = new SelectedVertex(&Vert, B, EditorScenePtr);

                        AddToSelectedObjects(CloseVert);
                    }
                }
            }
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
    Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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

        NewObjPos.x = Math::Round(NewObjPos.x, TransSnap);
        NewObjPos.y = Math::Round(NewObjPos.y, TransSnap);
        NewObjPos.z = Math::Round(NewObjPos.z, TransSnap);

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

        switch (Axis)
        {
        case EditingAxis::X:
            PointAlongAxis.x = Math::Round(PointAlongAxis.x, TransSnap);
            break;
        case EditingAxis::Y:
            PointAlongAxis.y = Math::Round(PointAlongAxis.y, TransSnap);
            break;
        case EditingAxis::Z:
            PointAlongAxis.z = Math::Round(PointAlongAxis.z, TransSnap);
            break;
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
    Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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

        //DeltaAngle = Math::Round(deltaAngle, RotSnap);

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
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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
            Ray MouseRay = EditorStatePtr->GetMouseRay(EditorStatePtr->ViewportCamera, MousePos, EditorStatePtr->GetEditorSceneViewportRect());

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

                IsCreatingNewPlane = true;
                NewPlaneStartPoint = HitPoint;
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
    for (auto& Object : SelectedObjects)
    {
        Object.second->DrawInspectorPanel();
    }

    // The objects' transforms may have been changed by sliders in the inspector panel, so re-calculate proxy and offsets
    RecalculateProxyAndObjectOffsets();
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
        if (*Obj.second == *NewSelectedObject)
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
}

void CursorState::UpdateSelectedTransformsBasedOnProxy()
{
    Vec3f ProxyPos = SelectedProxyTransform.GetPosition();

    for (auto& Object : SelectedObjects)
    {
        Object.second->GetTransform()->SetPosition(ProxyPos - Object.first.Offset);
    }
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

