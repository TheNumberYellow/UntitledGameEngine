#include "Halfedge.h"

#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/InputModule.h"

he::SelectedHalfEdgeVertex::SelectedHalfEdgeVertex(HalfEdgeMesh* inMeshPtr, he::Vertex* inVertPtr)
{
    m_HalfEdgeMesh = inMeshPtr;
    m_VertPtr = inVertPtr;

    m_Transform.SetPosition(m_VertPtr->vec);
}

void he::SelectedHalfEdgeVertex::Draw()
{
    GraphicsModule* graphics = GraphicsModule::Get();

    graphics->DebugDrawSphere(m_Transform.GetPosition(), 0.15f, MakeColour(125, 125, 255));

}

void he::SelectedHalfEdgeVertex::Update()
{
    // Test next vec
    if (InputModule::Get()->GetKeyState(Key::X).justPressed)
    {
        if (m_VertPtr->halfEdge->next)
        {
            m_VertPtr = m_VertPtr->halfEdge->next->vert;
            m_Transform.SetPosition(m_VertPtr->vec);

            m_Dirty = true;
        }
    }

    // Test flip to twin vert
    if (InputModule::Get()->GetKeyState(Key::Q).justPressed)
    {
        if (m_VertPtr->halfEdge->twin)
        {
            m_VertPtr = m_VertPtr->halfEdge->twin->vert;
            m_Transform.SetPosition(m_VertPtr->vec);

            m_Dirty = true;
        }
    }

    m_VertPtr->vec = m_Transform.GetPosition();
}

bool he::SelectedHalfEdgeVertex::DrawInspectorPanel()
{
    UIModule* ui = UIModule::Get();

    ui->TextButton("Vert", Vec2f(80, 40), 0.5f);

    if (m_Dirty)
    {
        m_Dirty = false;
        return true;
    }
    return false;
}

Transform* he::SelectedHalfEdgeVertex::GetTransform()
{
    return &m_Transform;
}

void he::SelectedHalfEdgeVertex::DeleteObject()
{
}

bool he::SelectedHalfEdgeVertex::IsEqual(const ISelectedObject& other) const
{
    const SelectedHalfEdgeVertex& otherSelection = static_cast<const SelectedHalfEdgeVertex&>(other);

    return otherSelection.m_HalfEdgeMesh == m_HalfEdgeMesh
        && otherSelection.m_VertPtr == m_VertPtr;
}

he::SelectedHalfEdgeFace::SelectedHalfEdgeFace(HalfEdgeMesh* inMeshPtr, he::Face* inFacePtr)
{
    m_HalfEdgeMesh = inMeshPtr;
    m_FacePtr = inFacePtr;

    //// Iterate through verts to get face pos
    //// Note(fraser): I really gotta make this less verbose/cumbersome (iterators or something?)
    //Vec3f pos = Vec3f(0.0f);
    //int numVertsInFace = 0;

    //he::Face face = m_HalfEdgeMesh->m_Faces[m_FaceIndex];
    //he::HalfEdge firstHalfEdge = m_HalfEdgeMesh->m_HalfEdges[face.halfEdge];

    //he::HalfEdge currentHalfEdge = firstHalfEdge;
    //do
    //{
    //    he::Vertex vert = m_HalfEdgeMesh->m_Verts[currentHalfEdge.vert];

    //    pos += vert.vec;
    //    numVertsInFace++;

    //    currentHalfEdge = m_HalfEdgeMesh->m_HalfEdges[currentHalfEdge.next];

    //} while (currentHalfEdge != firstHalfEdge);

    //pos /= numVertsInFace;

    //m_Transform.SetPosition(pos);
}

void he::SelectedHalfEdgeFace::Draw()
{
    GraphicsModule* graphics = GraphicsModule::Get();

    graphics->DebugDrawSphere(m_Transform.GetPosition(), 0.15f);

    HalfEdge* firstHalfEdge = m_FacePtr->halfEdge;

    HalfEdge* currentHalfEdge = firstHalfEdge;

    do
    {
        graphics->DebugDrawSphere(currentHalfEdge->vert->vec, 0.25f, Vec3f(0.0f, 1.0f, 1.0f));
        currentHalfEdge = currentHalfEdge->next;
    } while (currentHalfEdge != firstHalfEdge);
}

void he::SelectedHalfEdgeFace::Update()
{
    // Iterate through verts to get face pos
    // Note(fraser): I really gotta make this less verbose/cumbersome (iterators or something?)
    Vec3f pos = Vec3f(0.0f);
    int numVertsInFace = 0;

    he::Face* face = m_FacePtr;
    he::HalfEdge* firstHalfEdge = face->halfEdge;

    he::HalfEdge* currentHalfEdge = firstHalfEdge;
    do
    {
        he::Vertex* vert = currentHalfEdge->vert;

        pos += vert->vec;
        numVertsInFace++;

        currentHalfEdge = currentHalfEdge->next;

    } while (currentHalfEdge != firstHalfEdge);

    pos /= numVertsInFace;

    m_Transform.SetPosition(pos);
}

bool he::SelectedHalfEdgeFace::DrawInspectorPanel()
{
    UIModule* ui = UIModule::Get();

    ui->TextButton("Face", Vec2f(80, 40), 0.5f);

    return false;
}

Transform* he::SelectedHalfEdgeFace::GetTransform()
{
    return &m_Transform;
}

void he::SelectedHalfEdgeFace::DeleteObject()
{
}

bool he::SelectedHalfEdgeFace::IsEqual(const ISelectedObject& other) const
{
    const SelectedHalfEdgeFace& otherSelection = static_cast<const SelectedHalfEdgeFace&>(other);

    return otherSelection.m_HalfEdgeMesh == m_HalfEdgeMesh
        && otherSelection.m_FacePtr == m_FacePtr;
}


void he::HalfEdgeMesh::MakeQuad()
{
    Clear();

    m_Verts.push_back(new Vertex(Vec3f(-1.0f, -1.0f, 0.0f)));
    m_Verts.push_back(new Vertex(Vec3f(-1.0f, 1.0f, 0.0f)));
    m_Verts.push_back(new Vertex(Vec3f(1.0f, 1.0f, 0.0f)));
    m_Verts.push_back(new Vertex(Vec3f(1.0f, -1.0f, 0.0f)));

    m_Faces.push_back(new Face());
    m_Faces.push_back(new Face());

    HalfEdge leftInEdge;
    HalfEdge topInEdge;
    HalfEdge innerTopLeft;

    HalfEdge rightInEdge;
    HalfEdge bottomInEdge;
    HalfEdge innerBottomRight;

    m_HalfEdges.push_back(new HalfEdge());
    m_HalfEdges.push_back(new HalfEdge());
    m_HalfEdges.push_back(new HalfEdge());

    m_HalfEdges.push_back(new HalfEdge());
    m_HalfEdges.push_back(new HalfEdge());
    m_HalfEdges.push_back(new HalfEdge());

    m_HalfEdges[0]->vert = m_Verts[0];
    m_HalfEdges[1]->vert = m_Verts[1];
    m_HalfEdges[2]->vert = m_Verts[2];

    m_HalfEdges[3]->vert = m_Verts[2];
    m_HalfEdges[4]->vert = m_Verts[3];
    m_HalfEdges[5]->vert = m_Verts[0];

    m_HalfEdges[0]->next = m_HalfEdges[1];
    m_HalfEdges[1]->next = m_HalfEdges[2];
    m_HalfEdges[2]->next = m_HalfEdges[0];

    m_HalfEdges[3]->next = m_HalfEdges[4];
    m_HalfEdges[4]->next = m_HalfEdges[5];
    m_HalfEdges[5]->next = m_HalfEdges[3];

    m_HalfEdges[2]->twin = m_HalfEdges[5];
    m_HalfEdges[5]->twin = m_HalfEdges[2];

    m_HalfEdges[0]->face = m_Faces[0];
    m_HalfEdges[1]->face = m_Faces[0];
    m_HalfEdges[2]->face = m_Faces[0];

    m_HalfEdges[3]->face = m_Faces[1];
    m_HalfEdges[4]->face = m_Faces[1];
    m_HalfEdges[5]->face = m_Faces[1];

    m_Verts[0]->halfEdge = m_HalfEdges[0];
    m_Verts[1]->halfEdge = m_HalfEdges[1];
    m_Verts[2]->halfEdge = m_HalfEdges[3];
    m_Verts[3]->halfEdge = m_HalfEdges[4];

    m_Faces[0]->halfEdge = m_HalfEdges[0];
    m_Faces[1]->halfEdge = m_HalfEdges[3];
}

void he::HalfEdgeMesh::MakeAABB(AABB inAABB)
{
    Clear();

    Vec3f min = inAABB.min;
    Vec3f max = inAABB.max;

    float xLen = max.x - min.x;
    float yLen = max.y - min.y;
    float zLen = max.z - min.z;

    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y, min.z)));
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y, min.z)));
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y + yLen, min.z)));
    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y + yLen, min.z)));

    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y, min.z + zLen)));
    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y + yLen, min.z + zLen)));
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y + yLen, min.z + zLen)));
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y, min.z + zLen)));

    // Faces
    m_Faces.push_back(new Face());
    m_Faces.push_back(new Face());
    m_Faces.push_back(new Face());
    m_Faces.push_back(new Face());
    m_Faces.push_back(new Face());
    m_Faces.push_back(new Face());

    Face* bottomFace = m_Faces[0];
    Face* topFace = m_Faces[1];

    Face* negXFace = m_Faces[2];
    Face* posXFace = m_Faces[3];

    Face* negYFace = m_Faces[4];
    Face* posYFace = m_Faces[5];


    // Half-edges
    
    //TODO: look into why triangle fans use counter-clockwise face culling???
    // Bottom
    m_HalfEdges.push_back(new HalfEdge(m_Verts[0], bottomFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[1], bottomFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[2], bottomFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[3], bottomFace, nullptr, nullptr));

    m_HalfEdges[0]->next = m_HalfEdges[1];
    m_HalfEdges[1]->next = m_HalfEdges[2];
    m_HalfEdges[2]->next = m_HalfEdges[3];
    m_HalfEdges[3]->next = m_HalfEdges[0];

    // Top
    m_HalfEdges.push_back(new HalfEdge(m_Verts[4], topFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[5], topFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[6], topFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[7], topFace, nullptr, nullptr));

    m_HalfEdges[4]->next = m_HalfEdges[5];
    m_HalfEdges[5]->next = m_HalfEdges[6];
    m_HalfEdges[6]->next = m_HalfEdges[7];
    m_HalfEdges[7]->next = m_HalfEdges[4];

    // Neg-Y
    m_HalfEdges.push_back(new HalfEdge(m_Verts[0], negXFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[4], negXFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[7], negXFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[1], negXFace, nullptr, nullptr));

    m_HalfEdges[8]->next = m_HalfEdges[9];
    m_HalfEdges[9]->next = m_HalfEdges[10];
    m_HalfEdges[10]->next = m_HalfEdges[11];
    m_HalfEdges[11]->next = m_HalfEdges[8];

    // Pos-Y
    m_HalfEdges.push_back(new HalfEdge(m_Verts[2], posXFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[6], posXFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[5], posXFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[3], posXFace, nullptr, nullptr));

    m_HalfEdges[12]->next = m_HalfEdges[13];
    m_HalfEdges[13]->next = m_HalfEdges[14];
    m_HalfEdges[14]->next = m_HalfEdges[15];
    m_HalfEdges[15]->next = m_HalfEdges[12];

    // Neg-X
    m_HalfEdges.push_back(new HalfEdge(m_Verts[3], negYFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[5], negYFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[4], negYFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[0], negYFace, nullptr, nullptr));

    m_HalfEdges[16]->next = m_HalfEdges[17];
    m_HalfEdges[17]->next = m_HalfEdges[18];
    m_HalfEdges[18]->next = m_HalfEdges[19];
    m_HalfEdges[19]->next = m_HalfEdges[16];

    // Pos-X
    m_HalfEdges.push_back(new HalfEdge(m_Verts[1], posYFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[7], posYFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[6], posYFace, nullptr, nullptr));
    m_HalfEdges.push_back(new HalfEdge(m_Verts[2], posYFace, nullptr, nullptr));

    m_HalfEdges[20]->next = m_HalfEdges[21];
    m_HalfEdges[21]->next = m_HalfEdges[22];
    m_HalfEdges[22]->next = m_HalfEdges[23];
    m_HalfEdges[23]->next = m_HalfEdges[20];

    // Half edge twins
    
    // Bottom
    m_HalfEdges[0]->twin = m_HalfEdges[11];
    m_HalfEdges[1]->twin = m_HalfEdges[23];
    m_HalfEdges[2]->twin = m_HalfEdges[15];
    m_HalfEdges[3]->twin = m_HalfEdges[19];

    // Top
    m_HalfEdges[4]->twin = nullptr;
    m_HalfEdges[5]->twin = nullptr;
    m_HalfEdges[6]->twin = nullptr;
    m_HalfEdges[7]->twin = nullptr;

    // Neg-y
    m_HalfEdges[8]->twin = nullptr;
    m_HalfEdges[9]->twin = nullptr;
    m_HalfEdges[10]->twin = nullptr;
    m_HalfEdges[11]->twin = nullptr;

    // Pos-y
    m_HalfEdges[12]->twin = nullptr;
    m_HalfEdges[13]->twin = nullptr;
    m_HalfEdges[14]->twin = nullptr;
    m_HalfEdges[15]->twin = nullptr;

    // Neg-x
    m_HalfEdges[16]->twin = nullptr;
    m_HalfEdges[17]->twin = nullptr;
    m_HalfEdges[18]->twin = nullptr;
    m_HalfEdges[19]->twin = nullptr;

    // Pos-x
    m_HalfEdges[20]->twin = nullptr;
    m_HalfEdges[21]->twin = nullptr;
    m_HalfEdges[22]->twin = nullptr;
    m_HalfEdges[23]->twin = nullptr;

    // Verts
    m_Verts[0]->halfEdge = m_HalfEdges[0];
    m_Verts[1]->halfEdge = m_HalfEdges[1];
    m_Verts[2]->halfEdge = m_HalfEdges[2];
    m_Verts[3]->halfEdge = m_HalfEdges[3];
    m_Verts[4]->halfEdge = m_HalfEdges[4];
    m_Verts[5]->halfEdge = m_HalfEdges[5];
    m_Verts[6]->halfEdge = m_HalfEdges[6];
    m_Verts[7]->halfEdge = m_HalfEdges[7];

    bottomFace->halfEdge = m_HalfEdges[0];
    topFace->halfEdge = m_HalfEdges[4];
    negXFace->halfEdge = m_HalfEdges[8];
    posXFace->halfEdge = m_HalfEdges[12];
    negYFace->halfEdge = m_HalfEdges[16];
    posYFace->halfEdge = m_HalfEdges[20];
}

void he::HalfEdgeMesh::SubDivide()
{

}

void he::HalfEdgeMesh::SubDivideFace(Face* inFace)
{

}

void he::HalfEdgeMesh::EditorDraw()
{  
    GraphicsModule* graphics = GraphicsModule::Get();

    //for (auto& face : m_Faces)
    //{
    //    HalfEdge* initialHalfEdge = face->halfEdge;

    //    HalfEdge* currentHalfEdge = initialHalfEdge;

    //    do 
    //    {
    //        Vertex* thisVert = currentHalfEdge->vert;
    //        Vertex* nextVert = currentHalfEdge->next->vert;

    //        graphics->DebugDrawLine(thisVert->vec, nextVert->vec, MakeColour(0, 255, 0));

    //        currentHalfEdge = currentHalfEdge->next;

    //    } while (currentHalfEdge != initialHalfEdge);
    //}

    for (auto& face : m_Faces)
    {
        Vec3f averageFacePos = Vec3f(0.0f, 0.0f, 0.0f);
        int numVertsInFace = 0;

        HalfEdge* initialHalfEdge = face->halfEdge;

        HalfEdge* currentHalfEdge = initialHalfEdge;

        do {
            Vec3f vertPos = currentHalfEdge->vert->vec;

            averageFacePos += vertPos;
            numVertsInFace++;

            currentHalfEdge = currentHalfEdge->next;
        } while (currentHalfEdge != initialHalfEdge);


        averageFacePos /= numVertsInFace;

        graphics->DebugDrawSphere(averageFacePos, 0.2f, MakeColour(255, 200, 200));

        currentHalfEdge = initialHalfEdge;

        do {
            HalfEdge* nextHalfEdge = currentHalfEdge->next;

            Vec3f pointA = currentHalfEdge->vert->vec;
            Vec3f pointB = nextHalfEdge->vert->vec;

            pointA = Math::Lerp(pointA, averageFacePos, 0.1f);
            pointB = Math::Lerp(pointB, averageFacePos, 0.1f);

            pointA = Math::Lerp(pointA, pointB, 0.1f);
            pointB = Math::Lerp(pointB, pointA, 0.1f);


            graphics->DebugDrawArrow(pointA, pointB, MakeColour(80, 255, 80));

            currentHalfEdge = currentHalfEdge->next;

        } while (currentHalfEdge != initialHalfEdge);
    }

    //for (auto& halfEdge : m_HalfEdges)
    //{
    //    Vec3f pointA = halfEdge->vert->vec;
    //    Vec3f pointB = halfEdge->next->vert->vec;

    //    graphics->DebugDrawLine(pointA, pointB, MakeColour(255, 0, 120));
    //}
}

void he::HalfEdgeMesh::Clear()
{
    m_Verts.clear();
    m_Faces.clear();
    m_HalfEdges.clear();
}

RayCastHit he::HalfEdgeMesh::ClickCastFaces(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    CollisionModule* collisions = CollisionModule::Get();

    he::Face* hitFacePtr = nullptr;
    RayCastHit closestHitFace;

    for (int i = 0; i < m_Faces.size(); ++i)
    {
        std::vector<Vec3f> faceVerts;

        HalfEdge* initialHalfEdge = m_Faces[i]->halfEdge;
        HalfEdge* currentHalfEdge = initialHalfEdge;
        
        do
        {
            HalfEdge& thisHalfEdge = *currentHalfEdge;

            Vec3f faceVert = thisHalfEdge.vert->vec;
            faceVerts.push_back(faceVert);

            currentHalfEdge = thisHalfEdge.next;

        } while (currentHalfEdge != initialHalfEdge);

        assert(faceVerts.size() >= 3);

        size_t numFaces = faceVerts.size();

        for (int j = 1; j < numFaces - 1; ++j)
        {
            Triangle tri;
            tri.a = faceVerts[0];
            tri.b = faceVerts[j];
            tri.c = faceVerts[j + 1];

            RayCastHit newHit = collisions->RayCast(mouseRay, tri);

            if (newHit.hit && newHit.hitDistance < closestHitFace.hitDistance)
            {
                closestHitFace = newHit;
                hitFacePtr = m_Faces[i];
            }
        }
    }

    if (hitFacePtr != nullptr)
    {
        outSelectedObject = new SelectedHalfEdgeFace(this, hitFacePtr);
    }

    return closestHitFace;
}

RayCastHit he::HalfEdgeMesh::ClickCastVerts(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    CollisionModule* collisions = CollisionModule::Get();
    
    he::Vertex* hitVertPtr = nullptr;
    RayCastHit closestHitVert;

    for (int i = 0; i < m_Verts.size(); ++i)
    {
        Sphere vertSphere;
        vertSphere.position = m_Verts[i]->vec;
        vertSphere.radius = 0.15f;

        RayCastHit vertHit = collisions->RayCast(mouseRay, vertSphere);

        if (vertHit.hitDistance < closestHitVert.hitDistance)
        {
            closestHitVert = vertHit;
            hitVertPtr = m_Verts[i];
        }
    }

    if (hitVertPtr != nullptr)
    {
        outSelectedObject = new SelectedHalfEdgeVertex(this, hitVertPtr);
    }

    return closestHitVert;
}

//RayCastHit he::HalfEdgeMesh::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
//{
//    GraphicsModule* graphics = GraphicsModule::Get();
//    CollisionModule* collisions = CollisionModule::Get();
//    
//    VertIndex hitVertIndex = -1;
//    RayCastHit closestHitVert;
//
//    for (int i = 0; i < m_Verts.size(); ++i)
//    {
//        Sphere vertSphere;
//        vertSphere.position = m_Verts[i].vec;
//        vertSphere.radius = 0.15f;
//
//        RayCastHit vertHit = collisions->RayCast(mouseRay, vertSphere);
//
//        if (vertHit.hitDistance < closestHitVert.hitDistance)
//        {
//            closestHitVert = vertHit;
//            hitVertIndex = i;
//        }
//    }
//
//    FaceIndex hitFaceIndex = -1;
//    RayCastHit closestHitFace;
//
//    for (int i = 0; i < m_Faces.size(); ++i)
//    {
//        std::vector<Vec3f> faceVerts;
//
//        HalfEdgeIndex initialHalfEdge = m_Faces[i].halfEdge;
//        HalfEdgeIndex currentHalfEdge = initialHalfEdge;
//        
//        do
//        {
//            HalfEdge& thisHalfEdge = m_HalfEdges[currentHalfEdge];
//
//            Vec3f faceVert = m_Verts[thisHalfEdge.vert].vec;
//            faceVerts.push_back(faceVert);
//
//            currentHalfEdge = thisHalfEdge.next;
//
//        } while (currentHalfEdge != initialHalfEdge);
//
//        assert(faceVerts.size() == 3);
//
//        Triangle tri;
//        tri.a = faceVerts[0];
//        tri.b = faceVerts[1];
//        tri.c = faceVerts[2];
//
//        RayCastHit newHit = collisions->RayCast(mouseRay, tri);
//
//        if (newHit.hit && newHit.hitDistance < closestHitFace.hitDistance)
//        {
//            closestHitFace = newHit;
//            hitFaceIndex = i;
//        }
//    }
//
//    std::vector<RayCastHit> casts = { closestHitVert, closestHitFace };
//
//    RayCastHit closestHit;
//    int closestHitIndex = -1;
//
//    for (int i = 0; i < casts.size(); ++i)
//    {
//        if (casts[i].hit && casts[i].hitDistance < closestHit.hitDistance)
//        {
//            closestHitIndex = i;
//            closestHit = casts[i];
//        }
//    }
//
//    if (closestHitIndex == 0)
//    {
//        outSelectedObject = new SelectedHalfEdgeVertex(this, hitVertIndex);
//    }
//    else if (closestHitIndex == 1)
//    {
//        outSelectedObject = new SelectedHalfEdgeFace(this, hitFaceIndex);
//    }
//    else
//    {
//        // No hit
//    }
//
//    return closestHit;
//}
