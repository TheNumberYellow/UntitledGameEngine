#include "EditorState.h"

#include "GameState.h"

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
    UIModule* UI = UIModule::Get();

    Quaternion ModelQuat = ModelPtr->GetTransform().GetRotation();

    float QuatLength = (ModelQuat.x * ModelQuat.x) + (ModelQuat.y * ModelQuat.y) + (ModelQuat.z * ModelQuat.z) + (ModelQuat.w * ModelQuat.w);

    UI->TextButton("Rotation Quaternion length: " + std::to_string(QuatLength), Vec2f(240.0f, 40.0f), 12.0f, c_InspectorColour);

    static std::string TestString = "Test";

    UI->TextEntry("TestEntry", TestString, Vec2f(300.0f, 40.0f));

    //Material Mat = ModelPtr->m_TexturedMeshes[0].m_Material;

    //std::string MatStr = Mat.m_Albedo.Path.GetFullPath();
    //UI->TextButton("Albedo: " + MatStr, Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

    //MatStr = Mat.m_Normal.Path.GetFullPath();
    //UI->TextButton("Normal: " + MatStr, Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

    //MatStr = Mat.m_Roughness.Path.GetFullPath();
    //UI->TextButton("Roughness: " + MatStr, Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

    //MatStr = Mat.m_Metallic.Path.GetFullPath();
    //UI->TextButton("Metallic: " + MatStr, Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

    //MatStr = Mat.m_AO.Path.GetFullPath();
    //UI->TextButton("AO: " + MatStr, Vec2f(180.0f, 40.0f), 12.0f, c_InspectorColour);

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

SelectedVertex::SelectedVertex(Vec3f* InVertPtr, Brush* InBrushPtr)
{
    VertPtr = InVertPtr;
    BrushPtr = InBrushPtr;
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
    // Do nothing (can't delete individual brush vertices for now
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

void CursorState::Update(float DeltaTime)
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

void CursorState::UpdateSculptTool(float DeltaTime)
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

    if (Input->GetMouseState().GetDeltaMouseWheel() > 0)
    {
        SculptRadius += 0.1f;
    }
    else if (Input->GetMouseState().GetDeltaMouseWheel() < 0)
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
                    Vert->position += Vec3f(0.0f, 0.0f, Strength) * (float)DeltaTime;
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

                EditorScenePtr->AddModel(NewBrush->RepModel);

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

            SelectedVertex* ClickedVert = new SelectedVertex(HitVert, HitBrush);
        
            AddToSelectedObjects(ClickedVert);

            // Also add all vertices which were very close to the chosen vert
            for (auto& B : BrushVec)
            {
                for (auto& Vert : B->Vertices)
                {
                    if (Math::magnitude(*HitVert - Vert) < 0.00001f)
                    {
                        SelectedVertex* CloseVert = new SelectedVertex(&Vert, B);

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
                EditorScenePtr->AddModel(Graphics->CreateBoxModel(BoxBeingCreated));
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
            EditorScenePtr->AddModel(*NewPlane);
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
    OffsetInfo NewObjectOffsetInfo;

    Vec3f AveragePos = Vec3f(0.0f, 0.0f, 0.0f);

    for (auto& Obj : SelectedObjects)
    {
        AveragePos += Obj.second->GetTransform()->GetPosition();
        if (*Obj.second == *NewSelectedObject)
        {
            delete NewSelectedObject;
            return;
        }
    }
    
    AveragePos += NewSelectedObject->GetTransform()->GetPosition();
    AveragePos /= SelectedObjects.size() + 1;

    // Update selected object offsets
    for (auto& Obj : SelectedObjects)
    {
        Obj.first.Offset = AveragePos - Obj.second->GetTransform()->GetPosition();
    }

    NewObjectOffsetInfo.Offset = AveragePos - NewSelectedObject->GetTransform()->GetPosition();

    SelectedProxyTransform.SetPosition(AveragePos);

    // Reset proxy rotation, determine quaternion difference of selected objects

    SelectedProxyTransform.SetRotation(Quaternion());

    for (auto& Obj : SelectedObjects)
    {
        Obj.first.RotationDiff = Obj.second->GetTransform()->GetRotation() * SelectedProxyTransform.GetRotation().Inverse();
    }

    NewObjectOffsetInfo.RotationDiff = NewSelectedObject->GetTransform()->GetRotation() * SelectedProxyTransform.GetRotation().Inverse();


    SelectedObjects.push_back(std::make_pair(NewObjectOffsetInfo, NewSelectedObject));
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
    EditorScene.SetDirectionalLight(DirectionalLight{ Math::normalize(Vec3f(0.5f, 1.0f, -1.0f)), Vec3f(1.0f, 1.0f, 1.0f) });
    //EditorScene.SetCamera(&ViewportCamera);
    //EditorScene.SetCamera(&ModelCamera);

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

    TestFont = TextModule::Get()->LoadFont("fonts/ARLRDBD.TTF", 30);

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
#if 0
    UIModule* UI = UIModule::Get();
    GraphicsModule* Graphics = GraphicsModule::Get();

    //Graphics->ResetFrameBuffer();

    UI->StartFrame("Test", Rect(Vec2f(50.0f, 50.0f), Vec2f(600.0f, 400.0f)), 16.0f, c_LightGoldenRodYellow);
    {
        UI->StartFrame("Inner Test", Vec2f(400.0f, 150.0f), 16.0f, c_VegasGold);
        {
            for (int i = 0; i < 1000; ++i)
            {
                UI->TextButton("", Vec2f(20.0f, 20.0f), 4.0f, RandomColours[i]);
            }
        }
        UI->EndFrame();

        for (int i = 0; i < 1000; ++i)
        {
            UI->TextButton("", Vec2f(80.0f, 80.0f), 4.0f, RandomColours[i]);
        }
    }
    UI->EndFrame();

    UI->StartFrame("Test 2", Rect(Vec2f(50.0f, 500.0f), Vec2f(600.0f, 400.0f)), 16.0f, c_DarkOrange);
    {
        for (int i = 0; i < 1000; ++i)
        {
            UI->TextButton("", Vec2f(80.0f, 80.0f), 4.0f, RandomColours[i]);
        }
    }
    UI->EndFrame();

#else    
    UpdateEditor(DeltaTime);
#endif
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
        // TODO(Fraser): This is heavily reliant on order since Update() calls UI functions - another reason to make the UI module use deferred render commands
        Cursor.Update(DeltaTime);
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
    else
    {
        Cursor.ResetAllState();
    }

    if (Input->IsKeyDown(Key::E))
    {
        EditorScene.GetCamera()->SetPosition(ViewportCamera.GetPosition());
        EditorScene.GetCamera()->SetDirection(ViewportCamera.GetDirection());

        //EditorScene.GetCamera()->SetCamMatrix(ViewportCamera.GetInvCamMatrix());
    }

    // Keyboard hotkeys for switching tools
    if (Input->GetKeyState(Key::One).justPressed)
    {
        // If already in select tool, cycle sub-tool
        if (Cursor.GetToolMode() == ToolMode::Select)
        {
            Cursor.CycleSelectMode();
        }
        else
        {
            Cursor.SetToolMode(ToolMode::Select);
        }
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
    if (Input->GetKeyState(Key::Four).justPressed)
    {
        Cursor.SetToolMode(ToolMode::Sculpt);
    }

    if (Input->GetKeyState(Key::Five).justPressed)
    {
        Cursor.SetToolMode(ToolMode::Brush);
    }

    Vec2i ViewportSize = Engine::GetClientAreaSize();

    //Graphics->ResetFrameBuffer();

    UI->StartFrame("EditorFrame", Rect(Vec2f(0.0f, 0.0f), ViewportSize), 0.0f, c_VegasGold);
    {
        UI->StartTab("Level Editor");
        {
            DrawLevelEditor(Graphics, UI, DeltaTime);
        }
        UI->EndTab();

        UI->StartTab("Material Editor");
        {

        }
        UI->EndTab();       
    }
    UI->EndFrame();

}

void EditorState::UpdateGame(float DeltaTime)
{
}

Rect EditorState::GetEditorSceneViewportRect()
{
    Vec2f ViewportSize = Engine::GetClientAreaSize();

    Rect SceneViewportRect = Rect(  Vec2f(80.0f, 60.0f), 
                                    Vec2f(ViewportSize.x - 520.0f, ViewportSize.y * 0.7f - 60.0f));
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

    modelSelectToolTexture = *Registry->LoadTexture("images/cursorTool.png");
    vertexSelectToolTexture = *Registry->LoadTexture("images/vertexSelectTool.png");

    boxToolTexture = *Registry->LoadTexture("images/boxTool.png");
    planeToolTexture = *Registry->LoadTexture("images/planeTool.png");

    translateToolTexture = *Registry->LoadTexture("images/translateTool.png");
    rotateToolTexture = *Registry->LoadTexture("images/rotateTool.png");
    scaleToolTexture = *Registry->LoadTexture("images/scaleTool.png");

    vertexToolTexture = *Registry->LoadTexture("images/vertexTool.png");
    sculptToolTexture = *Registry->LoadTexture("images/sculptTool.png");
    
    lightEntityTexture = *Registry->LoadTexture("images/lightTool.png");
    directionalLightEntityTexture = *Registry->LoadTexture("images/dirLight.png");
    cameraEntityTexture = *Registry->LoadTexture("images/cameraButton.png");
    brainEntityTexture = *Registry->LoadTexture("images/brainTool.png");
    billboardEntityTexture = *Registry->LoadTexture("images/billboardTool.png");

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

            std::string NormalMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".norm" + extensionString;
            std::string RoughnessMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".rough" + extensionString;
            std::string MetallicMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".metal" + extensionString;
            std::string AOMapString = entry.path().parent_path().generic_string() + "/" + entry.path().stem().generic_string() + ".ao" + extensionString;

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

void EditorState::DrawLevelEditor(GraphicsModule* Graphics, UIModule* UI, float DeltaTime)
{
    Vec2i ViewportSize = Engine::GetClientAreaSize();

    auto BrushVec = EditorScene.GetBrushes();

    for (Brush* B : BrushVec)
    {
        B->UpdatedThisFrame = false;
        for (auto& Face : B->Faces)
        {
            Vec3f PlanePoint = Vec3f(0.0f, 0.0f, 0.0f);

            for (int i = 0; i < Face.size(); i++)
            {
                PlanePoint += *Face[i];
            }
            PlanePoint = PlanePoint / Face.size();

            for (int i = 0; i < Face.size() - 1; i++)
            {
                Graphics->DebugDrawLine(*Face[i], *Face[i + 1]);
            }
            Graphics->DebugDrawLine(*Face[Face.size() - 1], *Face[0]);

            Vec3f u = *Face[1] - *Face[0];
            Vec3f v = *Face[2] - *Face[0];

            Vec3f PlaneNorm = -Math::cross(u, v);
            PlaneNorm = Math::normalize(PlaneNorm);

            Vec3f UpProjection = Math::ProjectVecOnPlane(Vec3f(0.0f, 0.0f, 1.0f), Plane(PlanePoint, PlaneNorm));

            if (UpProjection.IsNearlyZero())
            {
                if (PlaneNorm.z > 0.0f)
                {
                    UpProjection = u;
                }
                else
                {
                    UpProjection = -u;
                }
            }

            UpProjection = Math::normalize(UpProjection);

            Graphics->DebugDrawLine(PlanePoint, PlanePoint + UpProjection, c_VegasGold);

            Graphics->DebugDrawLine(PlanePoint, PlanePoint + PlaneNorm, c_LightGoldenRodYellow);
        }

    }

    Graphics->SetActiveFrameBuffer(WidgetBuffer);

    EditorScene.EditorDraw(*Graphics, ViewportBuffer, &ViewportCamera);
    {
        Graphics->SetCamera(&ViewportCamera);

        Graphics->SetRenderMode(RenderMode::FULLBRIGHT);

        Cursor.DrawTransientModels();
    }
    Graphics->ResetFrameBuffer();

    UI->BufferPanel(ViewportBuffer.FinalOutput, GetEditorSceneViewportRect());

    UI->BufferPanel(WidgetBuffer, GetEditorSceneViewportRect());

    TextModule::Get()->DrawText("Frame Time: " + std::to_string(DeltaTime), &TestFont, GetEditorSceneViewportRect().location);

    PrevFrameTimeCount++;
    PrevFrameTimeSum += DeltaTime;

    if (PrevFrameTimeSum > 0.5f)
    {
        PrevAveFPS = (int)round(1.0f / (PrevFrameTimeSum / PrevFrameTimeCount));
        PrevFrameTimeCount = 0;
        PrevFrameTimeSum -= 0.5f;
    }

    TextModule::Get()->DrawText("FPS: " + std::to_string(PrevAveFPS), &TestFont, GetEditorSceneViewportRect().location + Vec2f(0.0f, 30.0f));

    DrawEditorUI();
}

void EditorState::DrawEditorUI()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    // Left toolbar buttons
    Rect ToolbarButtonRect = Rect(Vec2f(0.0f, 60.0f), Vec2f(ViewportRect.location.x, ViewportRect.size.y));

    UI->StartFrame("Tools", ToolbarButtonRect, 0.0f, c_VegasGold);
    {
        Vec3f SelectedColour = c_DarkOrange;
        Vec3f UnSelectedColour = c_GoldenYellow;

        Texture SelectModeTexture = modelSelectToolTexture;
        switch (Cursor.GetSelectMode())
        {
        case SelectMode::ModelSelect:
            SelectModeTexture = modelSelectToolTexture;
            break;
        case SelectMode::VertexSelect:
            SelectModeTexture = vertexSelectToolTexture;
            break;
        default:
            break;
        }

        if (UI->ImgButton("SelectTool", SelectModeTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Select ? SelectedColour : UnSelectedColour))
        {
            if (Cursor.GetToolMode() == ToolMode::Select)
            {
                Cursor.CycleSelectMode();
            }
            else
            {
                Cursor.SetToolMode(ToolMode::Select);
            }
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

        if (UI->ImgButton("SculptTool", sculptToolTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Sculpt ? SelectedColour : UnSelectedColour))
        {
            Cursor.SetToolMode(ToolMode::Sculpt);
        }

        if (UI->ImgButton("BrushTool", vertexToolTexture, Vec2f(80.0f, 80.0f), 12.0f,
            Cursor.GetToolMode() == ToolMode::Brush ? SelectedColour : UnSelectedColour))
        {
            Cursor.SetToolMode(ToolMode::Brush);
        }


    }
    UI->EndFrame();

    DrawTopPanel();
    DrawDrawerSettingsPanel();
    DrawResourcesPanel();
    DrawInspectorPanel();
}

void EditorState::DrawTopPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect TopPanelRect = Rect(Vec2f(0.0f, 20.0f),
        Vec2f(ScreenSize.x, ViewportRect.location.y - 20.0f));

    //Vec2f TopPanelButtonSize = Vec2f(ViewportRect.location.y, ViewportRect.location.y);

    UI->StartFrame("Top", TopPanelRect, 0.0f, c_VegasGold);
    {
        if (UI->ImgButton("PlayButton", playButtonTexture, Vec2f(40.0f, 40.0f), 8.0f))
        {
            Cursor.UnselectAll();
            Cursor.ResetAllState();
            GameState* NewGameState = new GameState();
            NewGameState->LoadScene(EditorScene);

            Machine->PushState(NewGameState);
        }
        if (UI->TextButton("New", Vec2f(40.0f, 40.0f), 8.0f))
        {
            Cursor.UnselectAll();
            EditorScene.Clear();
            BehaviourRegistry::Get()->ClearAllAttachedBehaviours();
        }
        if (UI->TextButton("Open", Vec2f(40.0f, 40.0f), 8.0f))
        {
            Cursor.UnselectAll();
            std::string FileName;
            if (Engine::FileOpenDialog(FileName))
            {
                EditorScene.Load(FileName);
            }
        }
        if (UI->TextButton("Save", Vec2f(40.0f, 40.0f), 8.0f))
        {
            std::string FileName;
            if (Engine::FileSaveDialog(FileName))
            {
                EditorScene.Save(FileName);
            }
        }
    }
    UI->EndFrame();
}

void EditorState::DrawDrawerSettingsPanel()
{
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect ModeSelectRect = Rect(
        Vec2f(0.0f, ViewportRect.location.y + ViewportRect.size.y), 
        Vec2f(ViewportRect.location.x, ScreenSize.y - (ViewportRect.location.y + ViewportRect.size.y))
    );

    UI->StartFrame("Mode Select", ModeSelectRect, 6.0f, c_VegasGold);
    {
        if (UI->TextButton("Content", Vec2f(ModeSelectRect.size.x - 16.0f, 40.0f), 6.0f))
        {
            Drawer = DrawerMode::CONTENT;
        }
        if (UI->TextButton("Browser", Vec2f(ModeSelectRect.size.x - 16.0f, 40.0f), 6.0f))
        {
            Drawer = DrawerMode::BROWSER;
        }

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

    if (Drawer == DrawerMode::CONTENT)
    {
        UI->StartFrame("Resources", ResourcePanelRect, 16.0f, c_VegasGold);
        {
            UI->StartTab("Models", c_LightGoldenRodYellow);
            {
                int index = 0;
                for (auto& AModel : LoadedModels)
                {
                    if (UI->TextButton(AModel.m_TexturedMeshes[0].m_Mesh.Path.GetFileNameNoExt(), Vec2f(120.0f, 40.0f), 10.0f, c_VegasGold).clicking)
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

            UI->StartTab("Materials", c_LightGoldenRodYellow);
            {
                for (auto& Mat : LoadedMaterials)
                {
                    if (UI->ImgButton(Mat.m_Albedo.Path.GetFileNameNoExt(), Mat.m_Albedo, Vec2f(80, 80), 5.0f, c_VegasGold).clicking)
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

            UI->StartTab("Entities", c_LightGoldenRodYellow);
            {
                if (UI->ImgButton("LightEntity", lightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f).clicking)
                {
                    if (!Cursor.IsDraggingSomething())
                    {
                        PointLight NewLight = PointLight();

                        PointLight* PointLightPtr = EditorScene.AddPointLight(NewLight);
                        Cursor.StartDraggingNewPointLight(PointLightPtr);
                    }
                }
                if (UI->ImgButton("DirectionalLightEntity", directionalLightEntityTexture, Vec2f(80.0f, 80.0f), 12.0f).clicking)
                {

                }
                if (UI->ImgButton("CameraEntity", cameraEntityTexture, Vec2f(80.0f, 80.0f), 12.0f))
                {
                }
                if (UI->ImgButton("BrainEntity", brainEntityTexture, Vec2f(80.0f, 80.0f), 12.0f))
                {
                }
                if (UI->ImgButton("BillboardEntity", billboardEntityTexture, Vec2f(80.0f, 80.0f), 12.0f))
                {
                }
            }
            UI->EndTab();

            UI->StartTab("Behaviours", c_LightGoldenRodYellow);
            {
                auto BehaviourMap = BehaviourRegistry::Get()->GetBehaviours();

                for (auto Behaviour : BehaviourMap)
                {
                    if (UI->TextButton(Behaviour.first, Vec2f(120, 40), 10.0f, c_VegasGold).clicking)
                    {
                        if (!Cursor.IsDraggingSomething())
                        {
                            Cursor.StartDraggingNewBehaviour(Behaviour.first);
                        }
                    }
                }
            }
            UI->EndTab();

            UI->StartTab("1000 Buttons", c_LightGoldenRodYellow);
            {
                for (int i = 0; i < 1000; ++i)
                {
                    UI->TextButton("", Vec2f(80.0f, 80.0f), 4.0f, RandomColours[i]);
                }
            }
            UI->EndTab();

            UI->StartTab("Experiment", c_LightGoldenRodYellow);
            {
            }
            UI->EndTab();

        }
        UI->EndFrame();
    }
    else if (Drawer == DrawerMode::BROWSER)
    {
        UI->StartFrame("Browser", ResourcePanelRect, 16.0f, c_LightGoldenRodYellow);
        {

        }
        UI->EndFrame();
    }

}

void EditorState::DrawInspectorPanel()
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    UIModule* UI = UIModule::Get();

    Vec2i ScreenSize = Engine::GetClientAreaSize();
    Rect ViewportRect = GetEditorSceneViewportRect();

    Rect InspectorPanelRect = Rect( Vec2f(ViewportRect.location.x + ViewportRect.size.x, ViewportRect.location.y), 
                                    Vec2f(ScreenSize.x - (ViewportRect.location.x + ViewportRect.size.x), ScreenSize.y - ViewportRect.location.y));

    UI->StartFrame("Inspector", InspectorPanelRect, 16.0f, c_LightGoldenRodYellow);
    {
        Cursor.DrawInspectorPanel();
    }
    UI->EndFrame();

}
