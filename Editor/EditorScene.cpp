//#include "EditorScene.h"
//
//#include "FileLoader.h"
//
//void EditorScene::AddEditableMesh(Mesh_ID newMesh)
//{
//    EditableMesh editableMesh;
//    editableMesh.m_Mesh = newMesh;
//    editableMesh.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(newMesh, m_Renderer);
//    editableMesh.m_LineMesh = m_Renderer->GenerateLineMeshFromMesh(newMesh, Vec4f(0.0f, 1.0f, 1.0f, 1.0f));
//    editableMesh.m_Selected = false;
//
//    m_EditableStaticGeometry.push_back(editableMesh);
//}
//
//void EditorScene::SetEditableMeshSelected(EditableMesh* mesh)
//{
//    m_SelectedMesh = mesh;
//    mesh->m_Selected = true;
//}
//
//void EditorScene::UnselectSelectedEditableMesh()
//{
//    if (m_SelectedMesh)
//    {
//        m_SelectedMesh->m_Selected = false;
//        m_SelectedMesh = nullptr;
//    }
//}
//
//EditorRayCastInfo EditorScene::RayCast(Ray ray)
//{
//    EditorRayCastInfo result; 
//    EditableMesh* hitMesh = nullptr;
//
//    // If we have a mesh selected, we want to raycast to its tools first, since those need priority
//    if (m_SelectedMesh)
//    {
//        if (toolsType == ToolsType::TRANSLATION)
//        {
//            ToolType tool = ToolType::NONE;
//            result.hitInfo = Collisions::RayCast(ray, m_XAxisArrow.m_CollisionTriangles,
//                m_Renderer->GetMeshTransform(m_XAxisArrow.m_Mesh));
//            if (result.hitInfo.hit) tool = ToolType::X_ARROW;
//
//            RayCastHit yTest = Collisions::RayCast(ray, m_YAxisArrow.m_CollisionTriangles,
//                m_Renderer->GetMeshTransform(m_YAxisArrow.m_Mesh));
//            if (yTest.hit && yTest.hitDistance < result.hitInfo.hitDistance)
//            {
//                tool = ToolType::Y_ARROW;
//                result.hitInfo = yTest;
//            }
//
//            RayCastHit zTest = Collisions::RayCast(ray, m_ZAxisArrow.m_CollisionTriangles,
//                m_Renderer->GetMeshTransform(m_ZAxisArrow.m_Mesh));
//            if (zTest.hit && zTest.hitDistance < result.hitInfo.hitDistance)
//            {
//                tool = ToolType::Z_ARROW;
//                result.hitInfo = zTest;
//            }
//
//            if (result.hitInfo.hit)
//            {
//                result.hitType = HitType::TOOL;
//                result.toolType = tool;
//                result.hitMesh = m_SelectedMesh;
//
//                return result;
//            }
//        }
//        else if (toolsType == ToolsType::ROTATION)
//        {
//            ToolType tool = ToolType::NONE;
//            result.hitInfo = Collisions::RayCast(ray, m_XRotatorRing.m_CollisionTriangles,
//                m_Renderer->GetMeshTransform(m_XRotatorRing.m_Mesh));
//            if (result.hitInfo.hit) tool = ToolType::X_RING;
//
//            RayCastHit yTest = Collisions::RayCast(ray, m_YRotatorRing.m_CollisionTriangles,
//                m_Renderer->GetMeshTransform(m_YRotatorRing.m_Mesh));
//            if (yTest.hit && yTest.hitDistance < result.hitInfo.hitDistance)
//            {
//                tool = ToolType::Y_RING;
//                result.hitInfo = yTest;
//            }
//
//            RayCastHit zTest = Collisions::RayCast(ray, m_ZRotatorRing.m_CollisionTriangles,
//                m_Renderer->GetMeshTransform(m_ZRotatorRing.m_Mesh));
//            if (zTest.hit && zTest.hitDistance < result.hitInfo.hitDistance)
//            {
//                tool = ToolType::Z_RING;
//                result.hitInfo = zTest;
//            }
//
//            if (result.hitInfo.hit)
//            {
//                result.hitType = HitType::TOOL;
//                result.toolType = tool;
//                result.hitMesh = m_SelectedMesh;
//
//                return result;
//            }
//        }
//    }
//
//    for (int i = 0; i < m_EditableStaticGeometry.size(); ++i)
//    {
//        RayCastHit newHit = Collisions::RayCast(ray, m_EditableStaticGeometry[i].m_CollisionTriangles, m_Renderer->GetMeshTransform(m_EditableStaticGeometry[i].m_Mesh));
//    
//        if (newHit.hit && newHit.hitDistance < result.hitInfo.hitDistance)
//        {
//            result.hitInfo = newHit;
//            hitMesh = &m_EditableStaticGeometry[i];
//        }
//    }
//    
//    result.hitMesh = hitMesh;
//    if (result.hitMesh)
//    {
//        result.hitType = HitType::MESH;
//    }
//    return result;
//}
//
//void EditorScene::CycleToolType()
//{
//    if (toolsType == ToolsType::TRANSLATION)
//    {
//        toolsType = ToolsType::ROTATION;
//    }
//    else if (toolsType == ToolsType::ROTATION)
//    {
//        toolsType = ToolsType::TRANSLATION;
//    }
//}
//
//void EditorScene::UpdateToolMeshPositions(Vec3f position)
//{
//    m_Renderer->SetMeshPosition(m_XAxisArrow.m_Mesh, position + Vec3f(TOOL_ARROW_DIST_FROM_OBJECT_ORIGIN, 0.0f, 0.0f));
//    m_Renderer->SetMeshPosition(m_YAxisArrow.m_Mesh, position + Vec3f(0.0f, TOOL_ARROW_DIST_FROM_OBJECT_ORIGIN, 0.0f));
//    m_Renderer->SetMeshPosition(m_ZAxisArrow.m_Mesh, position + Vec3f(0.0f, 0.0f, TOOL_ARROW_DIST_FROM_OBJECT_ORIGIN));
//
//    m_Renderer->SetMeshPosition(m_XRotatorRing.m_Mesh, position);
//    m_Renderer->SetMeshPosition(m_YRotatorRing.m_Mesh, position);
//    m_Renderer->SetMeshPosition(m_ZRotatorRing.m_Mesh, position);
//
//    Quaternion meshRotation = m_Renderer->GetMeshRotation(m_SelectedMesh->m_Mesh);
//}
//
//EditorScene::EditorScene(Renderer* renderer)
//    : Scene(renderer)
//{
//    m_XAxisArrow.m_Mesh = FileLoader::LoadOBJFile("models/ArrowSmooth.obj", *m_Renderer);
//    renderer->SetMeshColour(m_XAxisArrow.m_Mesh, Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
//    renderer->SetMeshScale(m_XAxisArrow.m_Mesh, Vec3f(0.2f, 0.2f, 0.2f));
//    renderer->RotateMeshAroundAxis(m_XAxisArrow.m_Mesh, Vec3f(0.0f, 0.0f, -1.0f), Math::Pi() / 2.0f);
//    m_XAxisArrow.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(m_XAxisArrow.m_Mesh, m_Renderer);
//    
//    m_YAxisArrow.m_Mesh = FileLoader::LoadOBJFile("models/ArrowSmooth.obj", *m_Renderer);
//    renderer->SetMeshColour(m_YAxisArrow.m_Mesh, Vec4f(0.0f, 1.0f, 0.0f, 1.0f));
//    renderer->SetMeshScale(m_YAxisArrow.m_Mesh, Vec3f(0.2f, 0.2f, 0.2f));
//    m_YAxisArrow.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(m_YAxisArrow.m_Mesh, m_Renderer);
//    
//    m_ZAxisArrow.m_Mesh = FileLoader::LoadOBJFile("models/ArrowSmooth.obj", *m_Renderer);
//    renderer->SetMeshColour(m_ZAxisArrow.m_Mesh, Vec4f(0.0f, 0.0f, 1.0f, 1.0f));
//    renderer->SetMeshScale(m_ZAxisArrow.m_Mesh, Vec3f(0.2f, 0.2f, 0.2f));
//    renderer->RotateMeshAroundAxis(m_ZAxisArrow.m_Mesh, Vec3f(1.0f, 0.0f, 0.0f), Math::Pi() / 2.0f);
//    m_ZAxisArrow.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(m_ZAxisArrow.m_Mesh, m_Renderer);
//
//    m_XRotatorRing.m_Mesh = FileLoader::LoadOBJFile("models/RotatorRing.obj", *m_Renderer);
//    renderer->SetMeshColour(m_XRotatorRing.m_Mesh, Vec4f(1.0f, 0.0f, 0.0f, 1.0f));
//    renderer->SetMeshScale(m_XRotatorRing.m_Mesh, Vec3f(0.7f, 0.7f, 0.7f));
//    renderer->RotateMeshAroundAxis(m_XRotatorRing.m_Mesh, Vec3f(0.0f, 0.0f, 1.0f), Math::Pi() / 2.0f);
//    m_XRotatorRing.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(m_XRotatorRing.m_Mesh, m_Renderer);
//
//    m_YRotatorRing.m_Mesh = FileLoader::LoadOBJFile("models/RotatorRing.obj", *m_Renderer);
//    renderer->SetMeshColour(m_YRotatorRing.m_Mesh, Vec4f(0.0f, 1.0f, 0.0f, 1.0f));
//    renderer->SetMeshScale(m_YRotatorRing.m_Mesh, Vec3f(0.7f, 0.7f, 0.7f));
//    m_YRotatorRing.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(m_YRotatorRing.m_Mesh, m_Renderer);
//
//    m_ZRotatorRing.m_Mesh = FileLoader::LoadOBJFile("models/RotatorRing.obj", *m_Renderer);
//    renderer->SetMeshColour(m_ZRotatorRing.m_Mesh, Vec4f(0.0f, 0.0f, 1.0f, 1.0f));
//    renderer->SetMeshScale(m_ZRotatorRing.m_Mesh, Vec3f(0.7f, 0.7f, 0.7f));
//    renderer->RotateMeshAroundAxis(m_ZRotatorRing.m_Mesh, Vec3f(1.0f, 0.0f, 0.0f), Math::Pi() / 2.0f);
//    m_ZRotatorRing.m_CollisionTriangles = Collisions::GenerateCollisionGeometryFromMesh(m_ZRotatorRing.m_Mesh, m_Renderer);
//}
//
//void EditorScene::DrawScene()
//{
//    __super::DrawScene();
//
//    for (int i = 0; i < m_EditableStaticGeometry.size(); ++i)
//    {
//        m_Renderer->DrawMesh(m_EditableStaticGeometry[i].m_Mesh);
//        if (m_EditableStaticGeometry[i].m_Selected)
//        {
//            m_Renderer->DrawMesh(m_EditableStaticGeometry[i].m_LineMesh);
//        }
//    }
//    if (m_SelectedMesh)
//    {
//        m_Renderer->ClearDepthBuffer();
//        //TODO(fraser): Update tools positions in a scene OnUpdate function, handled by engine(?)
//        UpdateToolMeshPositions(m_Renderer->GetMeshPosition(m_SelectedMesh->m_Mesh));
//
//        if (toolsType == ToolsType::TRANSLATION)
//        {
//            m_Renderer->DrawMesh(m_XAxisArrow.m_Mesh);
//            m_Renderer->DrawMesh(m_YAxisArrow.m_Mesh);
//            m_Renderer->DrawMesh(m_ZAxisArrow.m_Mesh);
//        }
//        if (toolsType == ToolsType::ROTATION)
//        {
//            m_Renderer->DrawMesh(m_XRotatorRing.m_Mesh);
//            m_Renderer->DrawMesh(m_YRotatorRing.m_Mesh);
//            m_Renderer->DrawMesh(m_ZRotatorRing.m_Mesh);
//        }
//    }
//}
//
//void EditableMesh::SetPosition(Vec3f position, Renderer* renderer)
//{
//    renderer->SetMeshPosition(m_Mesh, position);
//    renderer->SetMeshPosition(m_LineMesh, position);
//}
//
//void EditableMesh::SetScale(Vec3f scale, Renderer* renderer)
//{
//    renderer->SetMeshScale(m_Mesh, scale);
//    renderer->SetMeshScale(m_LineMesh, scale);
//}
//
//void EditableMesh::RotateMeshAroundAxis(Vec3f axis, float rotationAmount, Renderer* renderer)
//{
//    renderer->RotateMeshAroundAxis(m_Mesh, axis, rotationAmount);
//    renderer->RotateMeshAroundAxis(m_LineMesh, axis, rotationAmount);
//}
