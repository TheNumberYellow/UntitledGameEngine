#include "HalfEdge.h"

#include "Graphics/HotspotTexture.h"
#include "Modules/CollisionModule.h"
#include "Modules/GraphicsModule.h"
#include "Modules/InputModule.h"
#include "Modules/UIModule.h"
#include "Scene/Scene.h"

void he::ConvertHalfEdgeMeshToPolys(HalfEdgeMesh& mesh, std::vector<PolygonFace>& outPolys)
{
    outPolys.clear();
    for (Face* face : mesh.m_Faces)
    {
        PolygonFace poly;
        poly.OriginalFace = face;
        HalfEdge* startEdge = face->halfEdge;
        HalfEdge* currentEdge = startEdge;
        do
        {
            poly.Vertices.push_back(currentEdge->vert->vec);
            currentEdge = currentEdge->next;
        } while (currentEdge != startEdge);
        outPolys.push_back(poly);
    }
}

void he::SplitPolygonByPlane(PolygonFace& poly, Plane inPlane, PolygonFace& outFrontPoly, PolygonFace& outBackPoly, std::vector<Vec3f>* intersectionPoints)
{
    PolygonFace frontPoly;
    PolygonFace backPoly;

    if (poly.Vertices.empty())
    {
        return;
    }

    frontPoly.OriginalFace = poly.OriginalFace;
    backPoly.OriginalFace = poly.OriginalFace;

    size_t count = poly.Vertices.size();

    for (int i = 0; i < count; ++i)
    {
        Vec3f currentVert = poly.Vertices[i];
        Vec3f nextVert = poly.Vertices[(i + 1) % count];

        float distCurrent = Math::dot(currentVert - inPlane.center, inPlane.normal);
        float distNext = Math::dot(nextVert - inPlane.center, inPlane.normal);

        bool currentInFront = distCurrent >= 0.00001f;
        bool nextInFront = distNext >= 0.00001f;

        if (currentInFront != nextInFront)
        {
            // Edge intersects the plane, calculate intersection point
            float t = distCurrent / (distCurrent - distNext);
            Vec3f intersectionPoint = currentVert + t * (nextVert - currentVert);
            frontPoly.Vertices.push_back(intersectionPoint);

            if (intersectionPoints && currentInFront)
                intersectionPoints->push_back(intersectionPoint);
        }
        if (nextInFront)
        {
            frontPoly.Vertices.push_back(nextVert);
        }
    }

    for (int i = 0 ; i < count; ++i)
    {
        Vec3f currentVert = poly.Vertices[i];
        Vec3f nextVert = poly.Vertices[(i + 1) % count];
        
        float distCurrent = Math::dot(currentVert - inPlane.center, inPlane.normal);
        float distNext = Math::dot(nextVert - inPlane.center, inPlane.normal);
        
        bool currentBehind = distCurrent <= -0.00001f;
        bool nextBehind = distNext <= -0.00001f;
        
        if (currentBehind != nextBehind)
        {
            // Edge intersects the plane, calculate intersection point
            float t = distCurrent / (distCurrent - distNext);
            Vec3f intersectionPoint = currentVert + t * (nextVert - currentVert);
            backPoly.Vertices.push_back(intersectionPoint);
        }
        if (nextBehind)
        {
            backPoly.Vertices.push_back(nextVert);
        }
    }


    outFrontPoly = frontPoly;
    outBackPoly = backPoly;
}

void he::AddPolygonToHalfEdgeMesh(HalfEdgeMesh& mesh, PolygonFace& poly, Material inMaterial)
{
    if (poly.Vertices.size() < 3)
    {
        return; // Not a valid polygon
    }

    he::Face* newFace = new he::Face();
    // Copy over face material settings from original face if this polygon came from an existing half edge face (e.g. from slicing)

    mesh.m_Faces.push_back(newFace);

    std::vector<he::Vertex*> newVerts;
    std::vector<he::HalfEdge*> newHalfEdges;

    size_t count = poly.Vertices.size();

    newVerts.resize(count);
    newHalfEdges.resize(count);

    for (int i = 0; i < count; ++i)
    {
        // Check if vert already exists in mesh and reuse it instead of creating a new one
        bool vertExists = false;
        for (he::Vertex* existingVert : mesh.m_Verts)
        {
            if ((existingVert->vec - poly.Vertices[i]).Magnitude() < 0.0001f)
            {
                newVerts[i] = existingVert;
                vertExists = true;
                break;
            }
        }
        if (!vertExists)
        {
            he::Vertex* newVert = new he::Vertex(poly.Vertices[i]);
            mesh.m_Verts.push_back(newVert);
            newVerts[i] = newVert;
        }
        he::HalfEdge* newHalfEdge = new he::HalfEdge();
        mesh.m_HalfEdges.push_back(newHalfEdge);
        newHalfEdges[i] = newHalfEdge;
    }

    for (int i = 0; i < count; ++i)
    {
        size_t nextIndex = (i + 1) % count;

        newHalfEdges[i]->vert = newVerts[i];
        newHalfEdges[i]->next = newHalfEdges[nextIndex];
        newHalfEdges[i]->face = newFace;

        if (newVerts[i]->halfEdge == nullptr)
        {
            newVerts[i]->halfEdge = newHalfEdges[i];
        }
    }

    newFace->halfEdge = newHalfEdges[0];
    newFace->material = inMaterial;

    // Set twins for new half edges by checking if there is already a half edge in the mesh that goes in the opposite direction (i.e. has the same verts but reversed)
    for (he::HalfEdge* existingHalfEdge : mesh.m_HalfEdges)
    {
        for (he::HalfEdge* newHalfEdge : newHalfEdges)
        {
            if (existingHalfEdge->vert->vec == newHalfEdge->next->vert->vec 
                && existingHalfEdge->next->vert->vec == newHalfEdge->vert->vec)
            {
                existingHalfEdge->twin = newHalfEdge;
                newHalfEdge->twin = existingHalfEdge;
                break;
            }
        }

    }

    // Copy over texture settings from original face if this polygon came from an existing half edge face (e.g. from slicing)
    if (poly.OriginalFace)
    {
        newFace->textureNudgeU = poly.OriginalFace->textureNudgeU;
        newFace->textureNudgeV = poly.OriginalFace->textureNudgeV;
        newFace->textureScaleU = poly.OriginalFace->textureScaleU;
        newFace->textureScaleV = poly.OriginalFace->textureScaleV;
        newFace->textureRot = poly.OriginalFace->textureRot;

        newFace->flipFace = poly.OriginalFace->flipFace;

        if (poly.OriginalFace->appliedHotspotTexture)
        {
            newFace->ApplyHotspotTexture(*poly.OriginalFace->appliedHotspotTexture);
        }

    }
}

he::PolygonFace he::BuildCapPolygonForSlice(std::vector<Vec3f>& intersectionPoints, Plane slicePlane)
{
    he::PolygonFace capPoly;

    if (intersectionPoints.size() < 3)
    {
        return capPoly; // Not a valid polygon
    }

    // Remove duplicates

    std::vector<Vec3f> uniquePoints;
    for (Vec3f point : intersectionPoints)
    {
        bool isDuplicate = false;
        for (Vec3f uniquePoint : uniquePoints)
        {
            if (point == uniquePoint)
            {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate)
        {
            uniquePoints.push_back(point);
        }
    }

    if (uniquePoints.size() < 3)
    {
        return capPoly; // Not a valid polygon
    }

    // Sort the intersection points in clockwise order around the centroid to form a valid polygon
    Vec3f centroid(0.0f);
    for (Vec3f point : uniquePoints)
    {
        centroid += point;
    }

    centroid /= static_cast<float>(uniquePoints.size());

    // Build basis on plane to project points onto for sorting
    Vec3f tangent;
    if (std::abs(slicePlane.normal.x) > 0.5f)
    {
        tangent = Math::normalize(Math::cross(Vec3f(0.0f, 1.0f, 0.0f), slicePlane.normal));
    }
    else
    {
        tangent = Math::normalize(Math::cross(Vec3f(1.0f, 0.0f, 0.0f), slicePlane.normal));
    }

    Vec3f bitangent = Math::cross(slicePlane.normal, tangent);

    std::sort(uniquePoints.begin(), uniquePoints.end(), [&](Vec3f a, Vec3f b)
    {
        Vec3f aDir = a - centroid;
        Vec3f bDir = b - centroid;
        float aAngle = std::atan2(Math::dot(aDir, bitangent), Math::dot(aDir, tangent));
        float bAngle = std::atan2(Math::dot(bDir, bitangent), Math::dot(bDir, tangent));
        return aAngle < bAngle;
        });

    capPoly.Vertices = uniquePoints;

    return capPoly;
}

bool he::IsPolygonConvex(PolygonFace& poly)
{
    size_t count = poly.Vertices.size();
    if (count < 3)
    {
        return false; // Not a valid polygon
    }

    Vec3f lastCross(0.0f);
    bool firstCross = true;

    for (int i = 0; i < count; ++i)
    {
        Vec3f a = poly.Vertices[i];
        Vec3f b = poly.Vertices[(i + 1) % count];
        Vec3f c = poly.Vertices[(i + 2) % count];
        Vec3f ab = b - a;
        Vec3f bc = c - b;

        Vec3f cross = Math::cross(ab, bc);

        if (firstCross)
        {
            lastCross = cross;
            firstCross = false;
        }
        else
        {
            if (Math::dot(cross, lastCross) < 0.0f)
            {
                return false; // Cross products have different signs, so polygon is not convex
            }
        }
    }
    return true; // All cross products had the same sign, so polygon is convex
}

std::vector<he::FaceIsland> he::BuildFaceIslands(HalfEdgeMesh& mesh)
{
    std::vector<FaceIsland> faceIslands;

    std::vector<Face*> unprocessedFaces = mesh.m_Faces;

    while (!unprocessedFaces.empty())
    {
        Face* currentFace = unprocessedFaces.back();
        unprocessedFaces.pop_back();
        FaceIsland newIsland;
        newIsland.Faces.push_back(currentFace);

        // Recursively find all connected faces to this face and add them to the island IF
        // 1. They are not already in the island or another island
        // 2. Their normals are within a certain threshold of the current face's normal

        std::function<void(Face*)> findConnectedFaces = [&](Face* face)
            {
                Vec3f faceNormal = face->GetNormal();
                for (HalfEdge* halfEdge : mesh.m_HalfEdges)
                {
                    if (halfEdge->face == face && halfEdge->twin && halfEdge->twin->face)
                    {
                        Face* connectedFace = halfEdge->twin->face;
                        if (std::find(newIsland.Faces.begin(), newIsland.Faces.end(), connectedFace) == newIsland.Faces.end())
                        {
                            Vec3f connectedNormal = connectedFace->GetNormal();
                            float angle = std::acos(Math::dot(faceNormal, connectedNormal));
                            if (abs(angle) < Deg2Rad(28.0f) && !halfEdge->isSeam)
                            {
                                newIsland.Faces.push_back(connectedFace);
                                unprocessedFaces.erase(std::remove(unprocessedFaces.begin(), unprocessedFaces.end(), connectedFace), unprocessedFaces.end());
                                findConnectedFaces(connectedFace);
                            }
                        }
                    }
                }
            };

        findConnectedFaces(currentFace);


        faceIslands.push_back(newIsland);
    }

    return faceIslands;
}

std::vector<he::PolygonFace> he::GetFlattenedAttachedFaces(he::HalfEdgeMesh& mesh, he::Face* face)
{
    std::vector<FaceIsland> faceIslands = BuildFaceIslands(mesh);

    std::vector<he::PolygonFace> polyFaces;

    // Add this face, unaltered

    PolygonFace poly;

    HalfEdge* startEdge = face->halfEdge;

    HalfEdge* currentEdge = startEdge;

    do
    {
        poly.Vertices.push_back(currentEdge->vert->vec);
        currentEdge = currentEdge->next;
    } while (currentEdge != startEdge);

    polyFaces.push_back(poly);

    return polyFaces;
}

void he::DebugDrawSelectedHalfEdgeMeshFace(HalfEdgeMesh& mesh, Face* face, Vec3f colour)
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    HalfEdge* startEdge = face->halfEdge;

    HalfEdge* currentEdge = startEdge;

    std::vector<Vec3f> faceVerts;

    do
    {
        faceVerts.push_back(currentEdge->vert->vec);
        currentEdge = currentEdge->next;
    } while (currentEdge != startEdge);

    // Draw the face as a triangle fan
    for (size_t i = 1; i < faceVerts.size() - 1; ++i)
    {
        Graphics->DebugDrawTriangle(faceVerts[0], faceVerts[i], faceVerts[i + 1], Vec4f(colour, 0.65f));
    }
    // Draw the edges of the face
    for (size_t i = 0; i < faceVerts.size(); ++i)
    {
        size_t nextIndex = (i + 1) % faceVerts.size();
        Graphics->DebugDrawLine(faceVerts[i], faceVerts[nextIndex], colour);
    }

}

void he::DebugDrawSelectedHalfEdgeMeshEdge(HalfEdgeMesh& mesh, HalfEdge* edge, Vec3f colour)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    Vec3f startVert = edge->vert->vec;
    Vec3f endVert = edge->next->vert->vec;
    Graphics->DebugDrawLine(startVert, endVert, colour);
}

void he::DebugDrawSelectedHalfEdgeMeshVertex(HalfEdgeMesh& mesh, he::Vertex* vert, Vec3f colour)
{
    GraphicsModule* Graphics = GraphicsModule::Get();
    Graphics->DebugDrawSphere(vert->vec, 0.15f, colour);
}

void he::Face::ApplyHotspotTexture(HotspotTexture& inHotspotTexture)
{
    appliedHotspotTexture = &inHotspotTexture;

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
    const float randomScoreWeight = 0.0f;

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
            
            Vec2f uvOverride;

            uvOverride.x = Math::Remap(0.0f, bestRectSize.x, bestHotspotRectDebug.location.x, bestHotspotRectDebug.location.x + bestHotspotRectDebug.size.x, edgeProj);
            uvOverride.y = Math::Remap(0.0f, bestRectSize.y, bestHotspotRectDebug.location.y, bestHotspotRectDebug.location.y + bestHotspotRectDebug.size.y, normalProj);

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
    if (tempShowingIslands)
    {
        return;
    }

    // Draw each face
    for (he::Face* face : m_HalfEdgeMesh->m_Faces)
    {
        he::DebugDrawSelectedHalfEdgeMeshFace(*m_HalfEdgeMesh, face, MakeColour(100, 255, 255));
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

    // Test show face islands
    if (Input->GetKeyState(Key::I).pressed)
    {
        tempShowingIslands = true;

        std::vector<he::FaceIsland> faceIslands = he::BuildFaceIslands(*m_HalfEdgeMesh);
        for (he::FaceIsland& island : faceIslands)
        {
            // Determine color for this island based on its index in the list of islands so that the island colors are consistent across frames
            int islandIndex = &island - &faceIslands[0];

            double hue = fmod(290.0 + islandIndex * 127389.325, 360.0);
            double saturation = 1.0;
            double brightness = 1.0;

            //Colour islandColour = MakeColour((0 + (islandIndex * 167834169070)) % 255, (0 + (islandIndex * 78056439)) % 255, (255 + (islandIndex * 22391)) % 255);

            Colour islandColour = MakeColourHSV(hue, saturation, brightness);

            for (he::Face* face : island.Faces)
            {
                he::DebugDrawSelectedHalfEdgeMeshFace(*m_HalfEdgeMesh, face, islandColour);
            }
        }
    }
    else
    {
        tempShowingIslands = false;
    }

}

bool he::SelectedHalfEdgeMesh::DrawInspectorPanel()
{
    UIModule* ui = UIModule::Get();

    if (ui->CheckBox("Blend islands", m_HalfEdgeMesh->m_BlendIslands))
    {
        return true;
    }

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

    // Test edge split
    if (InputModule::Get()->GetKeyState(Key::E).justPressed)
    {
        if (m_VertPtr->halfEdge)
        {
            m_HalfEdgeMesh->SplitEdge(m_VertPtr->halfEdge);
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
    if (tempShowingFlattenedAttachedFaces)
    {
        return;
    }

    GraphicsModule* graphics = GraphicsModule::Get();

    DebugDrawSelectedHalfEdgeMeshFace(*m_HalfEdgeMesh, m_FacePtr, MakeColour(100, 255, 255));

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

    // Test show flattened attached faces
    if (Input->GetKeyState(Key::O).pressed)
    {
        tempShowingFlattenedAttachedFaces = true;
        std::vector<he::PolygonFace> flattenedPolys = he::GetFlattenedAttachedFaces(*m_HalfEdgeMesh, m_FacePtr);
        for (he::PolygonFace& poly : flattenedPolys)
        {
            for (size_t i = 0; i < poly.Vertices.size(); ++i)
            {
                size_t nextIndex = (i + 1) % poly.Vertices.size();
                GraphicsModule::Get()->DebugDrawLine(poly.Vertices[i], poly.Vertices[nextIndex], MakeColour(255, 255, 0));
            }
        }
    }
    else
    {
        tempShowingFlattenedAttachedFaces = false;
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
        m_FacePtr->appliedHotspotTexture = nullptr;
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

he::SelectedHalfEdgeEdge::SelectedHalfEdgeEdge(HalfEdgeMesh* inMeshPtr, he::HalfEdge* inEdgePtr)
    : m_HalfEdgeMesh(inMeshPtr)
    , m_EdgePtr(inEdgePtr)
{

    // Get edge pos
    Vec3f edgePos = Vec3f(0.0f);
    

    edgePos += inEdgePtr->vert->vec;
    edgePos += inEdgePtr->next->vert->vec;

    edgePos /= 2;

    m_Transform.SetPosition(edgePos);

    // Get edge vert offset transforms

    Vec3f offsetA = inEdgePtr->vert->vec - edgePos;
    Vec3f offsetB = inEdgePtr->next->vert->vec - edgePos;

    edgeOffsetA = Math::GenerateTransformMatrix(offsetA);
    edgeOffsetB = Math::GenerateTransformMatrix(offsetB);

}

void he::SelectedHalfEdgeEdge::Draw()
{
    GraphicsModule* graphics = GraphicsModule::Get();

    graphics->DebugDrawLine(m_EdgePtr->vert->vec, m_EdgePtr->next->vert->vec, MakeColour(100, 255, 255), 0.08f);

}

void he::SelectedHalfEdgeEdge::Update()
{

    m_HalfEdgeMesh->m_RepModelsNeedUpdate = true;

    m_EdgePtr->vert->vec = Vec3f(0.0f, 0.0f, 0.0f) * (m_Transform.GetTransformMatrix() * edgeOffsetA);
    m_EdgePtr->next->vert->vec = Vec3f(0.0f, 0.0f, 0.0f) * (m_Transform.GetTransformMatrix() * edgeOffsetB);

}

bool he::SelectedHalfEdgeEdge::DrawInspectorPanel()
{
    UIModule* ui = UIModule::Get();

    if (ui->CheckBox("Is Seam", m_EdgePtr->isSeam))
    {
        // Update the twin edge's seam status to match this edge's seam status
        if (m_EdgePtr->twin)
        {
            m_EdgePtr->twin->isSeam = m_EdgePtr->isSeam;
        }
        return true;
    }

    return false;
}

Transform* he::SelectedHalfEdgeEdge::GetTransform()
{
    return &m_Transform;
}

void he::SelectedHalfEdgeEdge::DeleteObject()
{
}

bool he::SelectedHalfEdgeEdge::IsEqual(const ISelectedObject& other) const
{
    const SelectedHalfEdgeEdge& otherSelection = static_cast<const SelectedHalfEdgeEdge&>(other);

    return otherSelection.m_HalfEdgeMesh == m_HalfEdgeMesh
        && otherSelection.m_EdgePtr == m_EdgePtr;
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

void he::HalfEdgeMesh::MakeCylinder(Vec3f baseCenter, float radius, float height, int numSegments, Material inMaterial)
{
    std::vector<PolygonFace> faces;

    for (int i = 0; i < numSegments; i++)
    {
        float angleStart = (static_cast<float>(i) / numSegments) * 2.0f * 3.14159265f;
        float angleEnd = (static_cast<float>(i + 1) / numSegments) * 2.0f * 3.14159265f;
        Vec3f offsetStart = Vec3f(cos(angleStart), sin(angleStart), 0.0f) * radius + baseCenter;
        Vec3f offsetEnd = Vec3f(cos(angleEnd), sin(angleEnd), 0.0f) * radius + baseCenter;

        PolygonFace face;
        face.Vertices.push_back(offsetStart);
        face.Vertices.push_back(offsetStart + Vec3f(0.0f, 0.0f, height));
        face.Vertices.push_back(offsetEnd + Vec3f(0.0f, 0.0f, height));
        face.Vertices.push_back(offsetEnd);
        
        faces.push_back(face);
    }

    // Add top/bottom caps
    PolygonFace topFace;
    PolygonFace bottomFace;

    for (int i = 0; i < numSegments; i++)
    {
        float angle = (static_cast<float>(i) / numSegments) * 2.0f * 3.14159265f;
        Vec3f offset = Vec3f(cos(angle), sin(angle), 0.0f) * radius + baseCenter;
        bottomFace.Vertices.push_back(offset);
        topFace.Vertices.push_back(offset + Vec3f(0.0f, 0.0f, height));
    }

    std::reverse(topFace.Vertices.begin(), topFace.Vertices.end()); // Reverse for correct winding order

    faces.push_back(bottomFace);
    faces.push_back(topFace);

    Clear();

    for (PolygonFace& face : faces)
    {
        AddPolygonToHalfEdgeMesh(*this, face, inMaterial);
    }
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


    for (he::HalfEdge* halfEdge : adjacentEdges)
    {
        if (halfEdge->twin)
        {
            halfEdge->twin->twin = nullptr;
        }
    }

    // Remove half edges and face
    for (he::HalfEdge*& halfEdge : adjacentEdges)
    {
        auto it = std::find(m_HalfEdges.begin(), m_HalfEdges.end(), halfEdge);
        if (it != m_HalfEdges.end())
        {
            m_HalfEdges.erase(it);
            delete halfEdge;
            halfEdge = nullptr;
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
        
        if (!outerEdge)
        {
            continue;
        }

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

        if (inFace->appliedHotspotTexture)
        {
            newFace->appliedHotspotTexture = inFace->appliedHotspotTexture;
        }

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

void he::HalfEdgeMesh::SplitEdge(HalfEdge* inEdge)
{
    he::HalfEdge* twinEdge = inEdge->twin;

    he::Vertex* v0 = inEdge->vert;
    he::Vertex* v1 = inEdge->next->vert;

    // Create midpoint vertex
    Vec3f mid = (v0->vec + v1->vec) * 0.5f;
    he::Vertex* midVert = new he::Vertex(mid);
    m_Verts.push_back(midVert);

    // Create new half-edges
    he::HalfEdge* newEdge = new he::HalfEdge();
    he::HalfEdge* newTwinEdge = new he::HalfEdge();
    m_HalfEdges.push_back(newEdge);
    m_HalfEdges.push_back(newTwinEdge);

    he::HalfEdge* inNext = inEdge->next;

    newEdge->vert = midVert;
    newEdge->face = inEdge->face;
    newEdge->next = inNext;

    inEdge->next = newEdge;

    he::HalfEdge* twinNext = twinEdge->next;

    newTwinEdge->vert = midVert;
    newTwinEdge->face = twinEdge->face;
    newTwinEdge->next = twinNext;

    twinEdge->next = newTwinEdge;

    inEdge->twin = newTwinEdge;
    newEdge->twin = twinEdge;

    midVert->halfEdge = newEdge;

    //// Rewire original half-edge
    //inEdge->vert = midVert;

    //// Setup new half-edge
    //newEdge->vert = v1;
    //newEdge->face = inEdge->face;
    //newEdge->next = inEdge->next;

    //inEdge->next = newEdge;

    //// Rewire twin half-edge
    //he::HalfEdge* twinNext = twinEdge->next;

    //twinEdge->vert = midVert;

    //newTwinEdge->vert = v0;
    //newTwinEdge->face = twinEdge->face;
    //newTwinEdge->next = twinNext;

    //twinEdge->next = newTwinEdge;

    //// Fix twins
    //inEdge->twin = newTwinEdge;
    //newTwinEdge->twin = inEdge;

    //newEdge->twin = twinEdge;
    //twinEdge->twin = newEdge;

    //midVert->halfEdge = newEdge;
}

bool he::HalfEdgeMesh::Slice(Plane inPlane, HalfEdgeMesh* outFrontMesh, HalfEdgeMesh* outBackMesh, bool addCaps)
{
    // First turn half edge mesh into vector of polys for easier processing
    std::vector<PolygonFace> polys;

    ConvertHalfEdgeMeshToPolys(*this, polys);

    std::vector<PolygonFace> frontPolys;
    std::vector<PolygonFace> backPolys;

    std::vector<Vec3f> intersectionPoints;

    for (PolygonFace& poly : polys)
    {
        PolygonFace frontPoly;
        PolygonFace backPoly;
        SplitPolygonByPlane(poly, inPlane, frontPoly, backPoly, &intersectionPoints);
        if (frontPoly.Vertices.size() >= 3)
        {
            frontPolys.push_back(frontPoly);
        }
        if (backPoly.Vertices.size() >= 3)
        {
            backPolys.push_back(backPoly);
        }
    }

    if (frontPolys.size() == 0 || backPolys.size() == 0)
    {
        return false;
    }

    if (outFrontMesh)
    {
        outFrontMesh->Clear();
    }
    if (outBackMesh)
    {
        outBackMesh->Clear();
    }

    Material defaultMat = GraphicsModule::Get()->m_DebugMaterial;

    if (outFrontMesh)
    {
        for (PolygonFace& frontPoly : frontPolys)
        {
            AddPolygonToHalfEdgeMesh(*outFrontMesh, frontPoly, frontPoly.OriginalFace ? frontPoly.OriginalFace->material : defaultMat);
        }
    }
    if (outBackMesh)
    {
        for (PolygonFace& backPoly : backPolys)
        {
            AddPolygonToHalfEdgeMesh(*outBackMesh, backPoly, backPoly.OriginalFace ? backPoly.OriginalFace->material : defaultMat);
        }
    }

    if (!addCaps)
    {
        return true;
    }

    // Add caps
    if (outFrontMesh)
    {
        PolygonFace frontCapPoly = BuildCapPolygonForSlice(intersectionPoints, inPlane);
        // Check if cap polys are convex - if not, we would need to triangulate them before adding to mesh. For now, just don't add a cap if it's not convex
        //if (IsPolygonConvex(frontCapPoly))
        {
            AddPolygonToHalfEdgeMesh(*outFrontMesh, frontCapPoly, defaultMat);
        }
    }

    if (outBackMesh)
    {
        PolygonFace backCapPoly = BuildCapPolygonForSlice(intersectionPoints, Plane(inPlane.center, inPlane.normal * -1.0f));
        // Check if cap polys are convex - if not, we would need to triangulate them before adding to mesh. For now, just don't add a cap if it's not convex
        //if (IsPolygonConvex(backCapPoly))
        {
            AddPolygonToHalfEdgeMesh(*outBackMesh, backCapPoly, defaultMat);
        }
    }

    return true;
}

void he::HalfEdgeMesh::SliceFaces(Plane inPlane)
{
    CollisionModule* collision = CollisionModule::Get();
    GraphicsModule* graphics = GraphicsModule::Get();

    for (auto& face : m_Faces)
    {
        // Get all adjacent edges
        std::vector<he::HalfEdge*> adjacentEdges;
        HalfEdge* firstHalfEdge = face->halfEdge;
        HalfEdge* currentHalfEdge = firstHalfEdge;
        do
        {
            adjacentEdges.push_back(currentHalfEdge);
            currentHalfEdge = currentHalfEdge->next;
        } while (currentHalfEdge != firstHalfEdge);

        // Classify verts as in front or behind plane
        std::vector<he::Vertex*> frontVerts;
        std::vector<he::Vertex*> backVerts;

        for (he::HalfEdge* halfEdge : adjacentEdges)
        {
            he::Vertex* vert = halfEdge->vert;

            float DistanceToPlane = Math::VecDistToPlane(vert->vec, inPlane);
            if (DistanceToPlane > 0.0f)
            {
                frontVerts.push_back(vert);
            }
            else
            {
                backVerts.push_back(vert);
            }
        }

        // Temp debugging: draw verts classified in front of plane as green, behind as red
        
        for (he::Vertex* vert : frontVerts)
        {
            graphics->DebugDrawSphere(vert->vec, 0.2f, MakeColour(0, 255, 0));
        }
        for (he::Vertex* vert : backVerts)
        {
            graphics->DebugDrawSphere(vert->vec, 0.2f, MakeColour(255, 0, 0));
        }

        // If all verts are on the same side of the plane, no need to slice
        if (frontVerts.size() == 0 || backVerts.size() == 0)
        {
            continue;
        }

        // Loop through front verts to find edges that intersect with plane, add new verts at intersection points
        he::Vertex* currentVert = frontVerts[0];
        do
        {
            he::HalfEdge* currentHalfEdge = currentVert->halfEdge;
            he::Vertex* nextVert = currentHalfEdge->next->vert;
            bool nextVertInFront = std::find(frontVerts.begin(), frontVerts.end(), nextVert) != frontVerts.end();
            if (!nextVertInFront)
            {
                // If the next vertex is behind the plane, find intersection point and add new vertex
                Vec3f lineStart = currentVert->vec;
                Vec3f lineEnd = nextVert->vec;

                Vec3f lineDir = lineEnd - lineStart;

                LineCastHit hit = collision->LineCast(lineStart, lineDir, inPlane);
                if (hit.hit)
                {
                    Vec3f newVertPos = hit.hitPoint;
                    // Before adding new vertex, find the next half edge it should connect to by searching for the 
                    // next edge which connects to a vertex back on the front side of the plane
                    he::HalfEdge* nextFrontHalfEdge = currentHalfEdge->next;
                    while (std::find(frontVerts.begin(), frontVerts.end(), nextFrontHalfEdge->vert) == frontVerts.end())
                    {
                        nextFrontHalfEdge = nextFrontHalfEdge->next;
                    }





                }
                else
                {
                    // This should never happen
                    Engine::FatalError("Expected to find intersection point between edge and plane, but no hit was found.");
                }


            }
            currentVert = currentHalfEdge->next->vert;
        } while (currentVert != frontVerts[0]);
    }
}

void he::HalfEdgeMesh::EditorDraw()
{  
    GraphicsModule* graphics = GraphicsModule::Get();

    for (auto& halfEdge : m_HalfEdges)
    {
        Vec3f pointA = halfEdge->vert->vec;
        Vec3f pointB = halfEdge->next->vert->vec;

        Colour edgeColour;
        if (halfEdge->isSeam)
        {
            edgeColour = MakeColour(255, 0, 0);
        }
        else
        {
            edgeColour = MakeColour(255, 0, 200);
        }

        graphics->DebugDrawLine(pointA, pointB, edgeColour);
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

RayCastHit he::HalfEdgeMesh::ClickCastEdges(Ray mouseRay, ISelectedObject*& outSelectedObject)
{
    he::HalfEdge* hitEdgePtr = nullptr;
    RayCastHit closestHitEdge;

    for (int i = 0; i < m_HalfEdges.size(); ++i)
    {
        Vec3f pointA = m_HalfEdges[i]->vert->vec;
        Vec3f pointB = m_HalfEdges[i]->next->vert->vec;

        std::pair<Vec3f, Vec3f> closestPoints = Math::ClosestPointsOnLineSegments(LineSegment(pointA, pointB), LineSegment(mouseRay.point, mouseRay.point + mouseRay.direction * 100.0f));
        
        RayCastHit edgeHit;
        if ((closestPoints.first - closestPoints.second).Magnitude() < 0.2f)
        {
            //GraphicsModule::Get()->DebugDrawPoint(closestPoints.first, MakeColour(0, 255, 0));
            //GraphicsModule::Get()->DebugDrawPoint(closestPoints.second, MakeColour(0, 0, 255));

            //GraphicsModule::Get()->DebugDrawCylinder(pointA, pointB, 0.2f, 8);
            //GraphicsModule::Get()->DebugDrawLine(pointA, pointB, Vec3f(1.0f, 1.0f, 1.0f));
            edgeHit.hit = true;
            edgeHit.hitPoint = closestPoints.first;
            edgeHit.hitDistance = (edgeHit.hitPoint - mouseRay.point).Magnitude();
        }

        if (edgeHit.hit && edgeHit.hitDistance < closestHitEdge.hitDistance)
        {
            closestHitEdge = edgeHit;
            hitEdgePtr = m_HalfEdges[i];
        }
    }

    if (hitEdgePtr != nullptr)
    {
        outSelectedObject = new SelectedHalfEdgeEdge(this, hitEdgePtr);
    }

    return closestHitEdge;
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

he::Face* he::HalfEdgeMesh::RayCastFaces(Ray mouseRay, Vec3f& outHitPoint, float& outHitDistance)
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
        outHitPoint = closestHitFace.hitPoint;
        outHitDistance = closestHitFace.hitDistance;
    }
    return hitFacePtr;
}

he::HalfEdge* he::HalfEdgeMesh::RayCastEdges(Ray mouseRay, Vec3f& outHitPoint, float& outHitDistance)
{
    he::HalfEdge* hitEdgePtr = nullptr;
    RayCastHit closestHitEdge;
    for (int i = 0; i < m_HalfEdges.size(); ++i)
    {
        Vec3f pointA = m_HalfEdges[i]->vert->vec;
        Vec3f pointB = m_HalfEdges[i]->next->vert->vec;
        std::pair<Vec3f, Vec3f> closestPoints = Math::ClosestPointsOnLineSegments(LineSegment(pointA, pointB), LineSegment(mouseRay.point, mouseRay.point + mouseRay.direction * 100.0f));
        
        RayCastHit edgeHit;
        if ((closestPoints.first - closestPoints.second).Magnitude() < 0.2f)
        {
            edgeHit.hit = true;
            edgeHit.hitPoint = closestPoints.first;
            edgeHit.hitDistance = (edgeHit.hitPoint - mouseRay.point).Magnitude();
        }
        if (edgeHit.hit && edgeHit.hitDistance < closestHitEdge.hitDistance)
        {
            closestHitEdge = edgeHit;
            hitEdgePtr = m_HalfEdges[i];
        }
    }
    if (hitEdgePtr != nullptr)
    {
        outHitPoint = closestHitEdge.hitPoint;
        outHitDistance = closestHitEdge.hitDistance;
    }
    return hitEdgePtr;
}

he::Vertex* he::HalfEdgeMesh::RayCastVerts(Ray mouseRay, Vec3f& outHitPoint, float& outHitDistance)
{
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
        outHitPoint = closestHitVert.hitPoint;
        outHitDistance = closestHitVert.hitDistance;
    }
    return hitVertPtr;
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


