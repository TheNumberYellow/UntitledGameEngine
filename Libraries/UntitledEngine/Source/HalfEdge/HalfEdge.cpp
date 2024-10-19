#include "Halfedge.h"

#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"

void he::HalfEdgeMesh::MakeQuad()
{
    Clear();

    m_Verts.push_back(Vec3f(-1.0f, -1.0f, 0.0f));
    m_Verts.push_back(Vec3f(-1.0f, 1.0f, 0.0f));
    m_Verts.push_back(Vec3f(1.0f, 1.0f, 0.0f));
    m_Verts.push_back(Vec3f(1.0f, -1.0f, 0.0f));

    m_Verts[0].halfEdge = 0;
    m_Verts[1].halfEdge = 1;
    m_Verts[2].halfEdge = 3;
    m_Verts[3].halfEdge = 4;

    m_Faces.push_back(Face());
    m_Faces.push_back(Face());

    m_Faces[0].halfEdge = 0;
    m_Faces[1].halfEdge = 3;

    HalfEdge leftInEdge;
    HalfEdge topInEdge;
    HalfEdge innerTopLeft;
    
    HalfEdge rightInEdge;
    HalfEdge bottomInEdge;
    HalfEdge innerBottomRight;

    leftInEdge.vert = 0;
    topInEdge.vert = 1;
    innerTopLeft.vert = 2;

    rightInEdge.vert = 2;
    bottomInEdge.vert = 3;
    innerBottomRight.vert = 0;

    leftInEdge.next = 1;
    topInEdge.next = 2;
    innerTopLeft.next = 0;

    rightInEdge.next = 4;
    bottomInEdge.next = 5;
    innerBottomRight.next = 3;

    innerTopLeft.twin = 5;
    innerBottomRight.twin = 2;

    leftInEdge.face = 0;
    topInEdge.face = 0;
    innerTopLeft.face = 0;

    rightInEdge.face = 1;
    bottomInEdge.face = 1;
    innerBottomRight.face = 1;

    m_HalfEdges.push_back(leftInEdge);
    m_HalfEdges.push_back(topInEdge);
    m_HalfEdges.push_back(innerTopLeft);

    m_HalfEdges.push_back(rightInEdge);
    m_HalfEdges.push_back(bottomInEdge);
    m_HalfEdges.push_back(innerBottomRight);
}

void he::HalfEdgeMesh::SubDivide()
{

}

void he::HalfEdgeMesh::Draw()
{  
    GraphicsModule* graphics = GraphicsModule::Get();

    for (auto& face : m_Faces)
    {
        Vec3f averageFacePos = Vec3f(0.0f, 0.0f, 0.0f);
        int numVertsInFace = 0;

        HalfEdgeIndex initialHalfEdge = face.halfEdge;

        HalfEdgeIndex currentHalfEdge = initialHalfEdge;

        do {
            HalfEdge& halfEdge = m_HalfEdges[currentHalfEdge];

            Vec3f vertPos = m_Verts[halfEdge.vert].vec;

            averageFacePos += vertPos;
            numVertsInFace++;

            currentHalfEdge = halfEdge.next;
        } while (currentHalfEdge != initialHalfEdge);


        averageFacePos /= numVertsInFace;

        graphics->DebugDrawSphere(averageFacePos, 0.2f, MakeColour(255, 200, 200));

        currentHalfEdge = initialHalfEdge;

        do {
            HalfEdge& halfEdge = m_HalfEdges[currentHalfEdge];

            HalfEdge& nextHalfEdge = m_HalfEdges[halfEdge.next];

            Vec3f pointA = m_Verts[halfEdge.vert].vec;
            Vec3f pointB = m_Verts[nextHalfEdge.vert].vec;

            pointA = Math::Lerp(pointA, averageFacePos, 0.1f);
            pointB = Math::Lerp(pointB, averageFacePos, 0.1f);

            pointA = Math::Lerp(pointA, pointB, 0.1f);
            pointB = Math::Lerp(pointB, pointA, 0.1f);


            graphics->DebugDrawArrow(pointA, pointB, MakeColour(80, 255, 80));

            currentHalfEdge = halfEdge.next;

        } while (currentHalfEdge != initialHalfEdge);
    }

    for (auto& halfEdge : m_HalfEdges)
    {
        Vec3f pointA = m_Verts[halfEdge.vert].vec;
        Vec3f pointB = m_Verts[m_HalfEdges[halfEdge.next].vert].vec;

        graphics->DebugDrawLine(pointA, pointB, MakeColour(255, 0, 120));
    }
}

void he::HalfEdgeMesh::Clear()
{
    m_Verts.clear();
    m_Faces.clear();
    m_HalfEdges.clear();
}

RayCastHit he::HalfEdgeMesh::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    CollisionModule* collisions = CollisionModule::Get();
    VertIndex hitVertIndex = -1;
    RayCastHit closestHit;

    for (int i = 0; i < m_Verts.size(); ++i)
    {
        Sphere vertSphere;
        vertSphere.position = m_Verts[i].vec;
        vertSphere.radius = 0.15f;

        RayCastHit vertHit = collisions->RayCast(mouseRay, vertSphere);

        if (vertHit.hitDistance < closestHit.hitDistance)
        {
            closestHit = vertHit;
            hitVertIndex = i;
        }
        //graphics->DebugDrawSphere(vertSphere.position, vertSphere.radius);
    }

    if (hitVertIndex == -1)
    {
        return closestHit;
    }
    else
    {
        outSelectedObject = new SelectedHalfEdgeVertex(this, hitVertIndex);
        return closestHit;
    }
}

he::SelectedHalfEdgeVertex::SelectedHalfEdgeVertex(HalfEdgeMesh* inMeshPtr, VertIndex inVertIndex)
{
    m_HalfEdgeMesh = inMeshPtr;
    m_VertIndex = inVertIndex;

    m_Transform.SetPosition(m_HalfEdgeMesh->m_Verts[m_VertIndex].vec);
}

void he::SelectedHalfEdgeVertex::Draw()
{
    GraphicsModule* graphics = GraphicsModule::Get();

    graphics->DebugDrawSphere(m_Transform.GetPosition(), 0.15f, MakeColour(125, 125, 255));

}

void he::SelectedHalfEdgeVertex::Update()
{
    m_HalfEdgeMesh->m_Verts[m_VertIndex].vec = m_Transform.GetPosition();
}

bool he::SelectedHalfEdgeVertex::DrawInspectorPanel()
{
    UIModule* ui = UIModule::Get();

    ui->TextButton("Subdivide", Vec2f(80, 40), 0.5f);

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
    return false;
}