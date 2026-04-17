#include "HalfEdge.h"

#include "Graphics/HotspotTexture.h"
#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/InputModule.h"
#include "Modules/UIModule.h"
#include "Scene.h"

void he::Face::ApplyHotspotTexture(HotspotTexture& inHotspotTexture)
{
    // Assume face is convex for now

    // Collect all verts and half edges for this face
    std::vector<he::Vertex*> faceVerts;
    std::vector<he::HalfEdge*> faceHalfEdges;
    he::HalfEdge* startEdge = halfEdge;
    he::HalfEdge* currentEdge = startEdge;
    do
    {
        faceVerts.push_back(currentEdge->vert);
        faceHalfEdges.push_back(currentEdge);
        currentEdge = currentEdge->next;
    } while (currentEdge != startEdge);

    // Determine containing rect for this face which minimizes area
    // Use rotating calipers method to find best rect orientation
    // 
    // Loop through face edges, for each edge project all verts onto edge direction 
    // and edge normal to get min/max in both directions, which gives us a rect for this edge orientation, 
    // then find the rect with the smallest area and use that as the face rect to compare against hotspot texture rects
    Vec2f bestRectSize = Vec2f(FLT_MAX, FLT_MAX);
    Vec3f bestRectUp = Vec3f(0.0f, 0.0f, 1.0f);
    Vec3f bestRectRight = Vec3f(1.0f, 0.0f, 0.0f);
    Vec3f bestRectStartPos = Vec3f(0.0f);
    float bestNormalAlignmentScore = -FLT_MAX;

    // Get face plane normal (assuming for now that the face is planar and convex, so we can just use the normal of the first 3 verts)
    Vec3f edge1 = faceVerts[1]->vec - faceVerts[0]->vec;
    Vec3f edge2 = faceVerts[2]->vec - faceVerts[0]->vec;
    Vec3f faceNormal = Math::cross(edge1, edge2).GetNormalized();

    for(he::HalfEdge* edge : faceHalfEdges)
    {
        Vec3f edgeDir = (edge->next->vert->vec - edge->vert->vec).GetNormalized();
        Vec3f edgeNormal = Math::cross(faceNormal, edgeDir).GetNormalized();

        // Project verts onto edge direction and normal to get rect size for this orientation
        float minEdge = FLT_MAX;
        float maxEdge = -FLT_MAX;
        float minNormal = FLT_MAX;
        float maxNormal = -FLT_MAX;

        
        Vec3f currentBestRectStartPos = edge->vert->vec;

        for (he::Vertex* vert : faceVerts)
        {
            Vec3f vertToEdge = vert->vec - edge->vert->vec;
            float edgeProj = Math::dot(vertToEdge, edgeDir);
            float normalProj = Math::dot(vertToEdge, edgeNormal);
            if (edgeProj < minEdge)
            {
                minEdge = edgeProj;
            }
            if (edgeProj > maxEdge)
            {
                maxEdge = edgeProj;
            }
            if (normalProj < minNormal)
            {
                minNormal = normalProj;
            }
            if (normalProj > maxNormal)
            {
                maxNormal = normalProj;
            }

            if (minEdge < 0.0f)
            {
                currentBestRectStartPos = edge->vert->vec + (edgeDir * minEdge);
            }
            if (minNormal < 0.0f)
            {
                currentBestRectStartPos = edge->vert->vec + (edgeNormal * minNormal);
            }
        }

        Vec2f rectSize = Vec2f(maxEdge - minEdge, maxNormal - minNormal);
        // Prefer rects with normals facing up (since our hotspot textures will be designed with that in mind) by giving them a score boost based on how closely the rect normal aligns with the world up vector
        float normalAlignmentScore = Math::dot(edgeNormal, Vec3f::Up());


        if ((rectSize.x * rectSize.y) - (0.1f * normalAlignmentScore) < (bestRectSize.x * bestRectSize.y) - bestNormalAlignmentScore)
        {
            bestNormalAlignmentScore = normalAlignmentScore;
            bestRectStartPos = currentBestRectStartPos;
            bestRectSize = rectSize;
            
            bestRectRight = edgeDir;
            bestRectUp = edgeNormal;
            
            // Temp debugging
            m_bestRectStartPos = bestRectStartPos;
            m_bestRectSize = bestRectSize;
            m_bestRectRight = bestRectRight;
            m_bestRectUp = bestRectUp;
        }

    }

    // Loop through the hotspot texture's rects and find the one that best matches the aspect ratio of the face rect, 
    // then apply the hotspot texture to the face using the best rect orientation and size
    
    Vec2f faceRectSize = bestRectSize;
    float bestScore = 0.0f;
    Rect bestHotspotRect;
    bool foundHotspotRect = false;
    bool shouldRotateHotspotUVs = false;
    
    const float aspectScoreWeight = 0.8f;
    const float sizeScoreWeight = 0.2f;
    const float randomScoreWeight = 0.01f;

    for (Rect hotspotRect : inHotspotTexture.m_Hotspots)
    {
        float hotspotAspect = hotspotRect.size.x / hotspotRect.size.y;
        float idealAspect = faceRectSize.x / faceRectSize.y;
        float aspectScore = 1.0f - abs(hotspotAspect - idealAspect) / idealAspect;

        float randomScore = Math::RandomFloat(0.0f, 1.0f);

        // Barf
        Vec2f textureSize = GraphicsModule::Get()->m_Renderer.GetTextureSize(inHotspotTexture.m_Material.m_Albedo->GetID());
        Vec2f hotspotTextureSize = hotspotRect.size * textureSize;

        // For now our ideal is 1m = 100 texels
        Vec2f idealHotspotSize = faceRectSize * 100.0f;
        float sizeScore = 1.0f - (hotspotTextureSize - idealHotspotSize).Magnitude() / idealHotspotSize.Magnitude();

        float totalScore = aspectScore * aspectScoreWeight + sizeScore * sizeScoreWeight + randomScore * randomScoreWeight;

        if (!foundHotspotRect || totalScore > bestScore)
        {
            bestHotspotRect = hotspotRect;
            bestScore = totalScore;
            foundHotspotRect = true;
            shouldRotateHotspotUVs = false;
        }
    
        // Also check rotated hotspot rect
        if (inHotspotTexture.m_AllowRotation)
        {
            float rotatedHotspotAspect = hotspotRect.size.y / hotspotRect.size.x;
            aspectScore = 1.0f - abs(rotatedHotspotAspect - idealAspect) / idealAspect;
            sizeScore = 1.0f - (Vec2f(hotspotTextureSize.y, hotspotTextureSize.x) - idealHotspotSize).Magnitude() / idealHotspotSize.Magnitude();

            totalScore = aspectScore * aspectScoreWeight + sizeScore * sizeScoreWeight + randomScore * randomScoreWeight;
            if (totalScore > bestScore)
            {
                bestHotspotRect = hotspotRect;
                bestScore = totalScore;
                foundHotspotRect = true;
                shouldRotateHotspotUVs = true;
            }
        }

    }
    // Apply the hotspot texture to the face using the best rect and orientation
    if (foundHotspotRect)
    {
        uvOverrides.clear();
        useUVOverride = true;
        material = inHotspotTexture.m_Material;

        if (shouldRotateHotspotUVs)
        {
            // Swap right and up for UV projection
            Vec3f temp = bestRectRight;
            bestRectRight = bestRectUp;
            bestRectUp = temp;
            // Swap rect size for UV projection
            float tempSize = bestRectSize.x;
            bestRectSize.x = bestRectSize.y;
            bestRectSize.y = tempSize;
        }

        for (he::Vertex* vert : faceVerts)
        {
            // Project vert onto best rect orientation to get UVs
            
            Vec3f vertToEdge = vert->vec - bestRectStartPos;
            float edgeProj = Math::dot(vertToEdge, bestRectRight);
            float normalProj = Math::dot(vertToEdge, bestRectUp);

            Rect bestHotspotRectDebug = bestHotspotRect;
            //bestHotspotRectDebug.location -= Vec2f(0.01f, 0.01f);
            //bestHotspotRectDebug.size += Vec2f(0.02f, 0.02f);
            
            Vec2f uvOverride;

            uvOverride.x = Math::Remap(0.0f, bestRectSize.x, bestHotspotRectDebug.location.x, bestHotspotRectDebug.location.x + bestHotspotRectDebug.size.x, edgeProj);
            uvOverride.y = Math::Remap(0.0f, bestRectSize.y, bestHotspotRectDebug.location.y, bestHotspotRectDebug.location.y + bestHotspotRectDebug.size.y, normalProj);

            //vert->uv.x = Math::Remap(0.0f, 1.0f, 1.0f, 0.0f, uvOverride.x);
            uvOverride.y = Math::Remap(0.0f, 1.0f, 1.0f, 0.0f, uvOverride.y);

            uvOverrides.push_back(uvOverride);
        }

    }
}

Vec3f he::Face::GetNormal()
{
    // Just get normal from first 3 verts for now, since we are assuming faces are planar and convex
    Vec3f edge1 = halfEdge->next->vert->vec - halfEdge->vert->vec;
    Vec3f edge2 = halfEdge->next->next->vert->vec - halfEdge->vert->vec;
    return -Math::cross(edge1, edge2).GetNormalized();
}

he::SelectedHalfEdgeMesh::SelectedHalfEdgeMesh(HalfEdgeMesh* inMeshPtr)
{
    m_HalfEdgeMesh = inMeshPtr;

    // Iterate through all verts to get average pos for half edge mesh
    Vec3f averagePos = Vec3f(0.0f);
    for (he::Vertex* vert : m_HalfEdgeMesh->m_Verts)
    {
        averagePos += vert->vec;
    }
    averagePos /= m_HalfEdgeMesh->m_Verts.size();

    m_Transform.SetPosition(averagePos);

    // Iterate through verts to get offset transforms
    for (he::Vertex* vert : m_HalfEdgeMesh->m_Verts)
    {
        Vec3f vertPos = vert->vec;
        Vec3f offset = vertPos - averagePos;
        m_VertTransOffsets.push_back(Math::GenerateTransformMatrix(offset));
    }

}

void he::SelectedHalfEdgeMesh::Draw()
{
    GraphicsModule* graphics = GraphicsModule::Get();
    // Draw lines for each half edge
    for (he::HalfEdge* halfEdge : m_HalfEdgeMesh->m_HalfEdges)
    {
        Vec3f start = halfEdge->vert->vec;
        Vec3f end = halfEdge->next ? halfEdge->next->vert->vec : start;
        graphics->DebugDrawLine(start, end, MakeColour(125, 125, 255));
    }
}

void he::SelectedHalfEdgeMesh::Update()
{
    InputModule* Input = InputModule::Get();

    m_HalfEdgeMesh->m_RepModelsNeedUpdate = true;

    for (size_t i = 0; i < m_HalfEdgeMesh->m_Verts.size(); i++)
    {
        Mat4x4f offset = m_VertTransOffsets[i];
        he::Vertex* vert = m_HalfEdgeMesh->m_Verts[i];
        vert->vec = Vec3f(0.0f, 0.0f, 0.0f) * (m_Transform.GetTransformMatrix() * offset);
    }

    if (Input->GetKeyState(Key::F).justPressed)
    {
        // Flip all faces
        for (he::Face* face : m_HalfEdgeMesh->m_Faces)
        {
            m_HalfEdgeMesh->FlipFace(face);
        }
    }
}

bool he::SelectedHalfEdgeMesh::DrawInspectorPanel()
{

    return false;
}

Transform* he::SelectedHalfEdgeMesh::GetTransform()
{
    return &m_Transform;
}

void he::SelectedHalfEdgeMesh::DeleteObject()
{
    ScenePtr->DeleteHalfEdgeMesh(m_HalfEdgeMesh);
}

void he::SelectedHalfEdgeMesh::ApplyMaterial(Material& inMaterial)
{
    for (Face* face : m_HalfEdgeMesh->m_Faces)
    {
        face->useUVOverride = false; 
        face->material = inMaterial;
    }
}

void he::SelectedHalfEdgeMesh::ApplyHotspotTexture(HotspotTexture& inHotspotTexture)
{
    for (Face* face : m_HalfEdgeMesh->m_Faces)
    {
        face->ApplyHotspotTexture(inHotspotTexture);
    }
}

bool he::SelectedHalfEdgeMesh::IsEqual(const ISelectedObject& other) const
{
    return &other == this || (typeid(*this) == typeid(other) && static_cast<const SelectedHalfEdgeMesh&>(other).m_HalfEdgeMesh == m_HalfEdgeMesh);
}


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
    m_HalfEdgeMesh->m_RepModelsNeedUpdate = true;

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

    bool positionChanged = false;
    positionChanged |= ui->FloatDragger("X", PlacementType::FIT_WIDTH, m_Transform.GetPosition().x, 0.1f);
    positionChanged |= ui->FloatDragger("Y", PlacementType::FIT_WIDTH, m_Transform.GetPosition().y, 0.1f);
    positionChanged |= ui->FloatDragger("Z", PlacementType::FIT_WIDTH, m_Transform.GetPosition().z, 0.1f);

    if (positionChanged)
    {
        return true;
    }
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

    // Iterate through verts to get face pos
    Vec3f averagePos = Vec3f(0.0f);
    int numVertsInFace = 0;

    he::Face* face = m_FacePtr;
    he::HalfEdge* firstHalfEdge = face->halfEdge;

    he::HalfEdge* currentHalfEdge = firstHalfEdge;
    do
    {
        he::Vertex* vert = currentHalfEdge->vert;

        averagePos  += vert->vec;
        numVertsInFace++;

        currentHalfEdge = currentHalfEdge->next;

    } while (currentHalfEdge != firstHalfEdge);

    averagePos /= numVertsInFace;

    m_Transform.SetPosition(averagePos);

    // Iterate through verts to get offset transforms
    currentHalfEdge = firstHalfEdge;
    do 
    {
        he::Vertex* vert = currentHalfEdge->vert;
        Vec3f vertPos = vert->vec;

        Vec3f offset = vertPos - averagePos;

        m_VertTransOffsets.push_back(Math::GenerateTransformMatrix(offset));
        currentHalfEdge = currentHalfEdge->next;

    } while (currentHalfEdge != firstHalfEdge);

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

    // Temp debugging
    // Debug draw the best rect
    if (shouldDebugDrawHotspotRect)
    {
        Vec3f rectCorners[4];
        rectCorners[0] = m_FacePtr->m_bestRectStartPos;
        rectCorners[1] = m_FacePtr->m_bestRectStartPos + (m_FacePtr->m_bestRectRight * m_FacePtr->m_bestRectSize.x);
        rectCorners[2] = m_FacePtr->m_bestRectStartPos + (m_FacePtr->m_bestRectRight * m_FacePtr->m_bestRectSize.x) + (m_FacePtr->m_bestRectUp * m_FacePtr->m_bestRectSize.y);
        rectCorners[3] = m_FacePtr->m_bestRectStartPos + (m_FacePtr->m_bestRectUp * m_FacePtr->m_bestRectSize.y);
        for (int i = 0; i < 4; i++)
        {
            graphics->DebugDrawLine(rectCorners[i], rectCorners[(i + 1) % 4], MakeColour(255, 0, 255));
        }
        graphics->DebugDrawArrow(m_FacePtr->m_bestRectStartPos, m_FacePtr->m_bestRectStartPos + m_FacePtr->m_bestRectUp, MakeColour(255, 255, 0));
    }
}

void he::SelectedHalfEdgeFace::Update()
{
    InputModule* Input = InputModule::Get();

    m_HalfEdgeMesh->m_RepModelsNeedUpdate = true;

    he::Face* face = m_FacePtr;

    if (true)
    {
        Vec3f faceNormal = face->GetNormal();
        m_Transform.SetPosition(m_Transform.GetPosition() + faceNormal * Input->GetMouseState().GetDeltaMouseWheel() * 0.25f);
    }

    he::HalfEdge* firstHalfEdge = face->halfEdge;

    he::HalfEdge* currentHalfEdge = firstHalfEdge;
    int i = 0;
    do
    {
        Mat4x4f offset = m_VertTransOffsets[i];

        he::Vertex* vert = currentHalfEdge->vert;
        vert->vec = Vec3f(0.0f, 0.0f, 0.0f) * (m_Transform.GetTransformMatrix() * offset);
                
        currentHalfEdge = currentHalfEdge->next;
        i++;
    } while (currentHalfEdge != firstHalfEdge);

    

    if (Input->GetKeyState(Key::E).justReleased)
    {
        isUserExtruding = false;
    }
    if (Input->GetKeyState(Key::E).justPressed)
    {
        isUserExtruding = true;
        m_HalfEdgeMesh->ExtrudeFace(m_FacePtr);
    }
    if (Input->GetKeyState(Key::F).justReleased)
    {
        m_HalfEdgeMesh->FlipFace(m_FacePtr);
    }

}

bool he::SelectedHalfEdgeFace::DrawInspectorPanel()
{
    UIModule* ui = UIModule::Get();

    ui->TextButton("Face", Vec2f(80, 40), 0.5f);

    ui->NewLine();

    ui->Text("Texture Nudge U", PlacementType::FIT_WIDTH);
    //ui->NewLine();
    ui->FloatDragger("TextureNudgeU_Dragger", PlacementType::FIT_WIDTH, m_FacePtr->textureNudgeU, 0.001f);
    ui->NewLine();

    ui->Text("Texture Nudge V", PlacementType::FIT_WIDTH);
    //ui->NewLine();
    ui->FloatDragger("TextureNudgeV_Dragger", PlacementType::FIT_WIDTH, m_FacePtr->textureNudgeV, 0.001f);
    ui->NewLine();

    ui->Text("Texture Scale U", PlacementType::FIT_WIDTH);
    //ui->NewLine();
    ui->FloatDragger("TextureScaleU_Dragger", PlacementType::FIT_WIDTH, m_FacePtr->textureScaleU, 0.001f);
    ui->NewLine();

    ui->Text("Texture Scale V", PlacementType::FIT_WIDTH);
    //ui->NewLine();
    ui->FloatDragger("TextureScaleV_Dragger", PlacementType::FIT_WIDTH, m_FacePtr->textureScaleV, 0.001f);
    ui->NewLine();
    
    ui->Text("Texture Rotation", PlacementType::FIT_WIDTH);
    ui->NewLine();
    
    ui->FloatDragger("TextureRotation_Dragger", PlacementType::FIT_WIDTH, m_FacePtr->textureRot, 0.001f);
    //ui->FloatSlider("TextureRotation", Vec2f(240.0f, 40.0f), m_FacePtr->textureRot, -3.14f, 3.14f);
    

    ui->CheckBox("Debug Draw HotspotRect", shouldDebugDrawHotspotRect);

    return false;
}

Transform* he::SelectedHalfEdgeFace::GetTransform()
{
    return &m_Transform;
}

void he::SelectedHalfEdgeFace::DeleteObject()
{
    m_HalfEdgeMesh->DeleteFace(m_FacePtr);
}

void he::SelectedHalfEdgeFace::ApplyMaterial(Material& inMaterial)
{
    if (m_FacePtr)
    {
        m_FacePtr->useUVOverride = false;
        m_FacePtr->material = Material(inMaterial);
        m_HalfEdgeMesh->m_RepModelsNeedUpdate = true;
    }
}

void he::SelectedHalfEdgeFace::ApplyHotspotTexture(HotspotTexture& inHotspotTexture)
{
    if (m_FacePtr)
    {
        m_FacePtr->ApplyHotspotTexture(inHotspotTexture);
        m_HalfEdgeMesh->m_RepModelsNeedUpdate = true;
    }
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

void he::HalfEdgeMesh::MakeAABB(AABB inAABB, Material inMaterial)
{
    Clear();

    Vec3f min = inAABB.min;
    Vec3f max = inAABB.max;

    float xLen = max.x - min.x;
    float yLen = max.y - min.y;
    float zLen = max.z - min.z;

    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y, min.z)));                          // 0
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y, min.z)));                   // 1
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y + yLen, min.z)));            // 2
    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y + yLen, min.z)));                   // 3

    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y, min.z + zLen)));                   // 4
    m_Verts.push_back(new Vertex(Vec3f(min.x, min.y + yLen, min.z + zLen)));            // 5
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y + yLen, min.z + zLen)));     // 6
    m_Verts.push_back(new Vertex(Vec3f(min.x + xLen, min.y, min.z + zLen)));            // 7

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

    bottomFace->material = inMaterial;
    topFace->material = inMaterial;
    negXFace->material = inMaterial;
    posXFace->material = inMaterial;
    negYFace->material = inMaterial;
    posYFace->material = inMaterial;

    // Half-edges
    
    //TODO: look into why triangle fans use counter-clockwise face culling???
    // Bottom
    m_HalfEdges.push_back(new HalfEdge(m_Verts[0], bottomFace, nullptr, nullptr)); // 0
    m_HalfEdges.push_back(new HalfEdge(m_Verts[1], bottomFace, nullptr, nullptr)); // 1
    m_HalfEdges.push_back(new HalfEdge(m_Verts[2], bottomFace, nullptr, nullptr)); // 2
    m_HalfEdges.push_back(new HalfEdge(m_Verts[3], bottomFace, nullptr, nullptr)); // 3

    m_HalfEdges[0]->next = m_HalfEdges[1];
    m_HalfEdges[1]->next = m_HalfEdges[2];
    m_HalfEdges[2]->next = m_HalfEdges[3];
    m_HalfEdges[3]->next = m_HalfEdges[0];

    // Top
    m_HalfEdges.push_back(new HalfEdge(m_Verts[4], topFace, nullptr, nullptr)); // 4
    m_HalfEdges.push_back(new HalfEdge(m_Verts[5], topFace, nullptr, nullptr)); // 5
    m_HalfEdges.push_back(new HalfEdge(m_Verts[6], topFace, nullptr, nullptr)); // 6
    m_HalfEdges.push_back(new HalfEdge(m_Verts[7], topFace, nullptr, nullptr)); // 7

    m_HalfEdges[4]->next = m_HalfEdges[5];
    m_HalfEdges[5]->next = m_HalfEdges[6];
    m_HalfEdges[6]->next = m_HalfEdges[7];
    m_HalfEdges[7]->next = m_HalfEdges[4];

    // Neg-Y
    m_HalfEdges.push_back(new HalfEdge(m_Verts[0], negXFace, nullptr, nullptr)); // 8
    m_HalfEdges.push_back(new HalfEdge(m_Verts[4], negXFace, nullptr, nullptr)); // 9
    m_HalfEdges.push_back(new HalfEdge(m_Verts[7], negXFace, nullptr, nullptr)); // 10
    m_HalfEdges.push_back(new HalfEdge(m_Verts[1], negXFace, nullptr, nullptr)); // 11

    m_HalfEdges[8]->next = m_HalfEdges[9];
    m_HalfEdges[9]->next = m_HalfEdges[10];
    m_HalfEdges[10]->next = m_HalfEdges[11];
    m_HalfEdges[11]->next = m_HalfEdges[8];

    // Pos-Y
    m_HalfEdges.push_back(new HalfEdge(m_Verts[2], posXFace, nullptr, nullptr)); // 12
    m_HalfEdges.push_back(new HalfEdge(m_Verts[6], posXFace, nullptr, nullptr)); // 13
    m_HalfEdges.push_back(new HalfEdge(m_Verts[5], posXFace, nullptr, nullptr)); // 14
    m_HalfEdges.push_back(new HalfEdge(m_Verts[3], posXFace, nullptr, nullptr)); // 15

    m_HalfEdges[12]->next = m_HalfEdges[13];
    m_HalfEdges[13]->next = m_HalfEdges[14];
    m_HalfEdges[14]->next = m_HalfEdges[15];
    m_HalfEdges[15]->next = m_HalfEdges[12];

    // Neg-X
    m_HalfEdges.push_back(new HalfEdge(m_Verts[3], negYFace, nullptr, nullptr)); // 16
    m_HalfEdges.push_back(new HalfEdge(m_Verts[5], negYFace, nullptr, nullptr)); // 17
    m_HalfEdges.push_back(new HalfEdge(m_Verts[4], negYFace, nullptr, nullptr)); // 18
    m_HalfEdges.push_back(new HalfEdge(m_Verts[0], negYFace, nullptr, nullptr)); // 19

    m_HalfEdges[16]->next = m_HalfEdges[17];
    m_HalfEdges[17]->next = m_HalfEdges[18];
    m_HalfEdges[18]->next = m_HalfEdges[19];
    m_HalfEdges[19]->next = m_HalfEdges[16];

    // Pos-X
    m_HalfEdges.push_back(new HalfEdge(m_Verts[1], posYFace, nullptr, nullptr)); // 20
    m_HalfEdges.push_back(new HalfEdge(m_Verts[7], posYFace, nullptr, nullptr)); // 21
    m_HalfEdges.push_back(new HalfEdge(m_Verts[6], posYFace, nullptr, nullptr)); // 22
    m_HalfEdges.push_back(new HalfEdge(m_Verts[2], posYFace, nullptr, nullptr)); // 23

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
    m_HalfEdges[4]->twin = m_HalfEdges[17];
    m_HalfEdges[5]->twin = m_HalfEdges[13];
    m_HalfEdges[6]->twin = m_HalfEdges[21];
    m_HalfEdges[7]->twin = m_HalfEdges[9];

    // Neg-y
    m_HalfEdges[8]->twin = m_HalfEdges[18];
    m_HalfEdges[9]->twin = m_HalfEdges[7];
    m_HalfEdges[10]->twin = m_HalfEdges[20];
    m_HalfEdges[11]->twin = m_HalfEdges[0];

    // Pos-y
    m_HalfEdges[12]->twin = m_HalfEdges[22];
    m_HalfEdges[13]->twin = m_HalfEdges[5];
    m_HalfEdges[14]->twin = m_HalfEdges[16];
    m_HalfEdges[15]->twin = m_HalfEdges[2];

    // Neg-x
    m_HalfEdges[16]->twin = m_HalfEdges[14];
    m_HalfEdges[17]->twin = m_HalfEdges[4];
    m_HalfEdges[18]->twin = m_HalfEdges[8];
    m_HalfEdges[19]->twin = m_HalfEdges[3];

    // Pos-x
    m_HalfEdges[20]->twin = m_HalfEdges[10];
    m_HalfEdges[21]->twin = m_HalfEdges[6];
    m_HalfEdges[22]->twin = m_HalfEdges[12];
    m_HalfEdges[23]->twin = m_HalfEdges[1];

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

void he::HalfEdgeMesh::DeleteFace(Face* inFace)
{
    // Get all adjacent edges
    std::vector<he::HalfEdge*> adjacentEdges;

    HalfEdge* firstHalfEdge = inFace->halfEdge;
    HalfEdge* currentHalfEdge = firstHalfEdge;

    do 
    {
        adjacentEdges.push_back(currentHalfEdge);
        currentHalfEdge = currentHalfEdge->next;
    } while (currentHalfEdge != firstHalfEdge);

    // Hook up adjacent faces to each other by connecting half edge twins of deleted face to each other
    for (he::HalfEdge* halfEdge : adjacentEdges)
    {
        if (halfEdge->twin)
        {
            halfEdge->twin->twin = halfEdge->next;
        }
        if (halfEdge->next)
        {
            halfEdge->next->twin = halfEdge->twin;
        }
    }

    // Remove half edges and face
    for (he::HalfEdge* halfEdge : adjacentEdges)
    {
        auto it = std::find(m_HalfEdges.begin(), m_HalfEdges.end(), halfEdge);
        if (it != m_HalfEdges.end())
        {
            m_HalfEdges.erase(it);
            delete halfEdge;
        }
    }
    
    auto faceIt = std::find(m_Faces.begin(), m_Faces.end(), inFace);
    if (faceIt != m_Faces.end())
    {
        m_Faces.erase(faceIt);
        delete inFace;
    }
}

void he::HalfEdgeMesh::FlipFace(Face* inFace)
{
    inFace->flipFace = !inFace->flipFace;
}

void he::HalfEdgeMesh::ExtrudeFace(he::Face* inFace)
{
    // Get all adjacent edges
    std::vector<he::HalfEdge*> innerEdges;
    std::vector<he::HalfEdge*> outerEdges;

    HalfEdge* firstHalfEdge = inFace->halfEdge;
    HalfEdge* currentHalfEdge = firstHalfEdge;

    do 
    {
        innerEdges.push_back(currentHalfEdge);
        outerEdges.push_back(currentHalfEdge->twin);

        currentHalfEdge = currentHalfEdge->next;
    } while (currentHalfEdge != firstHalfEdge);

    for (auto* innerEdge : innerEdges)
    {
        // Add new verts for extruded face so they can separate from connected faces
        Vec3f oldVecPos = innerEdge->vert->vec;
        he::Vertex* newVertex = new he::Vertex(oldVecPos);
        newVertex->halfEdge = innerEdge;

        m_Verts.push_back(newVertex);
        innerEdge->vert = newVertex;
    }

    // Vector to track inner half edges to be connected later
    std::vector<he::HalfEdge*> newInnerEdges;

    for (int i = 0; i < innerEdges.size(); ++i)
    {
        he::HalfEdge* innerEdge = innerEdges[i];
        he::HalfEdge* outerEdge = outerEdges[i];
        
        // Add 4 half edges making up new face between the inner and outer half edges
        he::HalfEdge* outerTwin = new he::HalfEdge();
        he::HalfEdge* innerTwin = new he::HalfEdge();
        he::HalfEdge* outerTwinNext = new he::HalfEdge();
        he::HalfEdge* innerTwinNext = new he::HalfEdge();

        // Add new face
        he::Face* newFace = new he::Face();
        newFace->halfEdge = outerTwin;

        // Copy material settings of extruded face
        newFace->material = inFace->material;
        newFace->textureNudgeU = inFace->textureNudgeU;
        newFace->textureNudgeV = inFace->textureNudgeV;

        newFace->flipFace = inFace->flipFace;

        newFace->textureScaleU = inFace->textureScaleU;
        newFace->textureScaleV = inFace->textureScaleV;

        newFace->textureRot = inFace->textureRot;

        m_Faces.push_back(newFace);

        outerTwin->vert = outerEdge->next->vert;
        outerTwin->face = newFace;
        outerTwin->next = outerTwinNext;
        outerTwin->twin = outerEdge;

        innerTwin->vert = innerEdge->next->vert;
        innerTwin->face = newFace;
        innerTwin->next = innerTwinNext;
        innerTwin->twin = innerEdge;

        outerTwinNext->vert = outerEdge->vert;
        outerTwinNext->face = newFace;
        outerTwinNext->next = innerTwin;
        outerTwinNext->twin = nullptr; // todo

        innerTwinNext->vert = innerEdge->vert;
        innerTwinNext->face = newFace;
        innerTwinNext->next = outerTwin;
        innerTwinNext->twin = nullptr; // todo

        // Hook up existing inner/outer half edges to new half edges
        innerEdge->twin = innerTwin;
        outerEdge->twin = outerTwin;

        m_HalfEdges.push_back(outerTwin);
        m_HalfEdges.push_back(innerTwin);
        m_HalfEdges.push_back(outerTwinNext);
        m_HalfEdges.push_back(innerTwinNext);

        newInnerEdges.push_back(innerTwinNext);
        newInnerEdges.push_back(outerTwinNext);
    }

    // Hook up inner edges of new face "ring"
    for (int i = 1; i < newInnerEdges.size() - 1; i += 2)
    {
        newInnerEdges[i]->twin = newInnerEdges[i + 1];
        newInnerEdges[i + 1]->twin = newInnerEdges[i];
    }

    newInnerEdges[newInnerEdges.size() - 1]->twin = newInnerEdges[0];
    newInnerEdges[0]->twin = newInnerEdges[newInnerEdges.size() - 1];
    
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

    //for (auto& face : m_Faces)
    //{
    //    Vec3f averageFacePos = Vec3f(0.0f, 0.0f, 0.0f);
    //    int numVertsInFace = 0;

    //    HalfEdge* initialHalfEdge = face->halfEdge;

    //    HalfEdge* currentHalfEdge = initialHalfEdge;

    //    do {
    //        Vec3f vertPos = currentHalfEdge->vert->vec;

    //        averageFacePos += vertPos;
    //        numVertsInFace++;

    //        currentHalfEdge = currentHalfEdge->next;
    //    } while (currentHalfEdge != initialHalfEdge);


    //    averageFacePos /= numVertsInFace;

    //    graphics->DebugDrawSphere(averageFacePos, 0.2f, MakeColour(255, 200, 200));

    //    currentHalfEdge = initialHalfEdge;

    //    do {
    //        HalfEdge* nextHalfEdge = currentHalfEdge->next;

    //        Vec3f pointA = currentHalfEdge->vert->vec;
    //        Vec3f pointB = nextHalfEdge->vert->vec;

    //        pointA = Math::Lerp(pointA, averageFacePos, 0.1f);
    //        pointB = Math::Lerp(pointB, averageFacePos, 0.1f);

    //        pointA = Math::Lerp(pointA, pointB, 0.1f);
    //        pointB = Math::Lerp(pointB, pointA, 0.1f);


    //        graphics->DebugDrawArrow(pointA, pointB, MakeColour(80, 255, 80));

    //        currentHalfEdge = currentHalfEdge->next;

    //    } while (currentHalfEdge != initialHalfEdge);
    //}

    for (auto& halfEdge : m_HalfEdges)
    {
        Vec3f pointA = halfEdge->vert->vec;
        Vec3f pointB = halfEdge->next->vert->vec;

        graphics->DebugDrawLine(pointA, pointB, MakeColour(255, 0, 120));
    }
}

void he::HalfEdgeMesh::Clear()
{
    // TODO: Delete rep models (they're currently being deleted every frame in the GraphicsModule

    for (he::Vertex* vert : m_Verts)
    {
        delete vert;
    }
    for (he::Face* face : m_Faces)
    {
        delete face;
    }
    for (he::HalfEdge* halfEdge : m_HalfEdges)
    {
        delete halfEdge;
    }

    m_Verts.clear();
    m_Faces.clear();
    m_HalfEdges.clear();
}

RayCastHit he::HalfEdgeMesh::RayCast(Ray ray)
{
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
            if (m_Faces[i]->flipFace)
            {
                tri.a = faceVerts[0];
                tri.b = faceVerts[j + 1];
                tri.c = faceVerts[j];
            }
            else
            {
                tri.a = faceVerts[0];
                tri.b = faceVerts[j];
                tri.c = faceVerts[j + 1];
            }

            RayCastHit newHit = collisions->RayCast(ray, tri);

            if (newHit.hit && newHit.hitDistance < closestHitFace.hitDistance)
            {
                closestHitFace = newHit;
                hitFacePtr = m_Faces[i];
            }
        }
    }

    return closestHitFace;
}

Intersection he::HalfEdgeMesh::SphereIntersect(Sphere sphere)
{
    CollisionModule* collisions = CollisionModule::Get();

    Intersection deepestIntersection;
    
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
            
            if (m_Faces[i]->flipFace)
            {
                tri.a = faceVerts[0];
                tri.b = faceVerts[j + 1];
                tri.c = faceVerts[j];
            }
            else
            {
                tri.a = faceVerts[0];
                tri.b = faceVerts[j];
                tri.c = faceVerts[j + 1];
            }

            Intersection newIntersection = collisions->SphereIntersection(sphere, tri);

            if (newIntersection.hit && newIntersection.penetrationDepth > deepestIntersection.penetrationDepth)
            {
                deepestIntersection = newIntersection;
            }
        }
    }
    return deepestIntersection;
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

RayCastHit he::HalfEdgeMesh::ClickCast(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    GraphicsModule* graphics = GraphicsModule::Get();
    CollisionModule* collisions = CollisionModule::Get();

    // We only need to check faces
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
            if (newHit.hit)
            {
                outSelectedObject = new SelectedHalfEdgeMesh(this);
                return newHit;
            }
        }
     }
     // No hit
    return RayCastHit();    
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

