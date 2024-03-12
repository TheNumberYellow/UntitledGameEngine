#include "CollisionModule.h"

CollisionModule* CollisionModule::s_Instance = nullptr;

OctreeNode::~OctreeNode()
{
    if (IsLeaf)
    {
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        delete SubNodes[i];
    }
}

CollisionMesh::~CollisionMesh()
{
    if (OctreeHead)
    {
        delete OctreeHead;
    }
}

void OctreeNode::AddTriangle(Triangle t, int tempDepth)
{
    if (IsLeaf)
    {
        Triangles.push_back(t);

        //Engine::DEBUGPrint("Adding triangle at depth: " + std::to_string(tempDepth));

        bool ShouldNotAddLevel = false;

        if (Bounds.max.x - Bounds.min.x < 0.001f)
        {
            ShouldNotAddLevel = true;
        }

        if (Triangles.size() > MaxTriangles && !ShouldNotAddLevel)
        {
            tempDepth++;
            
            AddLevel(tempDepth);
        }
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            if (Intersects(t, SubNodes[i]->Bounds))
            {
                SubNodes[i]->AddTriangle(t, tempDepth);
            }
        }
    }
}

void OctreeNode::AddLevel(int tempDepth)
{
    IsLeaf = false;

    // Create child nodes
    for (int i = 0; i < 8; ++i)
    {
        SubNodes[i] = new OctreeNode();
        SubNodes[i]->IsLeaf = true;
    }

    Vec3f HalfDim = (Bounds.max - Bounds.min) / 2.0f;

    SubNodes[0]->Bounds = AABB( Bounds.min, 
                                Bounds.min + HalfDim);
    SubNodes[1]->Bounds = AABB( Bounds.min + HalfDim.XOnly(), 
                                Bounds.min + HalfDim + HalfDim.XOnly());
    SubNodes[2]->Bounds = AABB( Bounds.min + HalfDim.YOnly(),
                                Bounds.min + HalfDim + HalfDim.YOnly());
    SubNodes[3]->Bounds = AABB( Bounds.min + HalfDim.XYOnly(),
                                Bounds.min + HalfDim + HalfDim.XYOnly());

    SubNodes[4]->Bounds = AABB( Bounds.min + HalfDim.ZOnly(),
                                Bounds.min + HalfDim + HalfDim.ZOnly());
    SubNodes[5]->Bounds = AABB( Bounds.min + HalfDim.XOnly() + HalfDim.ZOnly(),
                                Bounds.min + HalfDim + HalfDim.XOnly() + HalfDim.ZOnly());
    SubNodes[6]->Bounds = AABB( Bounds.min + HalfDim.YOnly() + HalfDim.ZOnly(),
                                Bounds.min + HalfDim + HalfDim.YOnly() + HalfDim.ZOnly());
    SubNodes[7]->Bounds = AABB( Bounds.min + HalfDim.XYOnly() + HalfDim.ZOnly(),
                                Bounds.min + HalfDim + HalfDim.XYOnly() + HalfDim.ZOnly());

    // Add triangles to subnodes
    for (Triangle& Tri : Triangles)
    {
        for (int i = 0; i < 8; ++i)
        {
            if (Intersects(Tri, SubNodes[i]->Bounds))
            {
                SubNodes[i]->AddTriangle(Tri, tempDepth);
            }
        }
    }
    Triangles.clear();
}

CollisionModule::CollisionModule(Renderer& renderer)
    : m_Renderer(renderer)
{
    s_Instance = this;
}

CollisionModule::~CollisionModule()
{

}

CollisionMesh* CollisionModule::GetCollisionMeshFromMesh(StaticMesh mesh)
{
    StaticMesh_ID Id = mesh.Id;

    if (m_CollisionMeshMap.find(Id) == m_CollisionMeshMap.end())
    {
        GenerateCollisionMeshFromMesh(mesh);
    }
    return m_CollisionMeshMap[Id];
}

CollisionMesh* CollisionModule::GenerateCollisionMeshFromMesh(StaticMesh mesh)
{
    CollisionMesh* collMesh = new CollisionMesh();

    std::vector<Vertex*> verts = m_Renderer.MapMeshVertices(mesh.Id);
    std::vector<ElementIndex*> indices = m_Renderer.MapMeshElements(mesh.Id);

    assert(indices.size() > 0
        && verts.size() > 0
        && indices.size() % 3 == 0);

    AABB boundingBox;
    boundingBox.min = Vec3f(std::numeric_limits<float>::max(), 
        std::numeric_limits<float>::max(), 
        std::numeric_limits<float>::max());

    boundingBox.max = Vec3f(std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(), 
        std::numeric_limits<float>::lowest());

    for (int i = 0; i < verts.size(); ++i)
    {
        if (verts[i]->position.x < boundingBox.min.x) boundingBox.min.x = verts[i]->position.x;
        if (verts[i]->position.y < boundingBox.min.y) boundingBox.min.y = verts[i]->position.y;
        if (verts[i]->position.z < boundingBox.min.z) boundingBox.min.z = verts[i]->position.z;

        if (verts[i]->position.x > boundingBox.max.x) boundingBox.max.x = verts[i]->position.x;
        if (verts[i]->position.y > boundingBox.max.y) boundingBox.max.y = verts[i]->position.y;
        if (verts[i]->position.z > boundingBox.max.z) boundingBox.max.z = verts[i]->position.z;

        collMesh->points.push_back(verts[i]->position);
    }
    for (int i = 0; i < indices.size(); ++i)
    {
        collMesh->indices.push_back(*indices[i]);
    }

    collMesh->boundingBox = boundingBox;

    m_Renderer.UnmapMeshVertices(mesh.Id);
    m_Renderer.UnmapMeshElements(mesh.Id);

    collMesh->OctreeHead = new OctreeNode(boundingBox);
    
    for (int i = 0; i < collMesh->indices.size(); i += 3)
    {
        Triangle tri;
        tri.a = collMesh->points[collMesh->indices[i]];
        tri.b = collMesh->points[collMesh->indices[i + 1]];
        tri.c = collMesh->points[collMesh->indices[i + 2]];

        collMesh->OctreeHead->AddTriangle(tri, 0);
    }

    StaticMesh_ID Id = mesh.Id;

    m_CollisionMeshMap[Id] = collMesh;

    return m_CollisionMeshMap[Id];
}

void CollisionModule::InvalidateMeshCollisionData(StaticMesh_ID mesh)
{
    //CollisionMesh* colMesh = GetCollisionMeshFromMesh(mesh);
    if (m_CollisionMeshMap.find(mesh) != m_CollisionMeshMap.end())
    {
        CollisionMesh* colMesh = m_CollisionMeshMap[mesh];
        delete colMesh;
    }
    m_CollisionMeshMap.erase(mesh);
}

RayCastHit CollisionModule::RayCast(Ray ray, Model& model)
{
    return RayCast(ray, *GetCollisionMeshFromMesh(model.m_TexturedMeshes[0].m_Mesh), model.GetTransform().GetTransformMatrix());
}

RayCastHit CollisionModule::RayCast(Ray ray, const CollisionMesh& mesh, Transform& transform)
{
    return RayCast(ray, mesh, transform.GetTransformMatrix());
}

RayCastHit CollisionModule::RayCast(Ray ray, const CollisionMesh& mesh, const Mat4x4f& meshTransform)
{
    RayCastHit resultHit;

    Mat4x4f invMeshTransform = Math::inv(meshTransform);

    Ray transformedRay;
    
    transformedRay.point = ray.point * invMeshTransform;

    // TODO(fraser): Currently Vec3f Mat4x4f multiplications add a 1 as the w component (meaning a point)
    // If this workaround comes up often, might want to figure out a way of specifying point or vector in math operations
    Vec4f dirVec4 = Vec4f(ray.direction.x, ray.direction.y, ray.direction.z, 0.0f);
    dirVec4 = dirVec4 * invMeshTransform;
    transformedRay.direction = Vec3f(dirVec4.x, dirVec4.y, dirVec4.z);

    RayCastHit aabbHit = RayCast(transformedRay, mesh.boundingBox);
    if (!aabbHit.hit)
    {
        return resultHit;
    }
    
    if (OctreeEnabled)
    {
        resultHit = RayCast(transformedRay, mesh.OctreeHead, meshTransform);
    }
    else
    {
        for (int i = 0; i < mesh.indices.size(); i += 3)
        {
            Vec3f a = mesh.points[mesh.indices[i]];
            Vec3f b = mesh.points[mesh.indices[(size_t)i + 1]];
            Vec3f c = mesh.points[mesh.indices[(size_t)i + 2]];

            RayCastHit newHit = RayCastTri(transformedRay, a, b, c);

            if (newHit.hit && newHit.hitDistance < resultHit.hitDistance)
            {
                resultHit = newHit;
            }
        }
    }


   

    resultHit.hitPoint = resultHit.hitPoint * meshTransform;

    //TODO(fraser): ^^^
    Vec4f normVec4 = Vec4f(resultHit.hitNormal.x, resultHit.hitNormal.y, resultHit.hitNormal.z, 0.0f);
    normVec4 = normVec4 * meshTransform;

    resultHit.hitNormal = Math::normalize(Vec3f(normVec4.x, normVec4.y, normVec4.z));


    return resultHit;
}

void swap(float& left, float& right)
{
    float temp = left;
    left = right;
    right = temp;
}

RayCastHit CollisionModule::RayCast(Ray ray, AABB aabb)
{
    RayCastHit result;

    // Real Time Collision Detection page 181
    float tmin = 0.0f;

    float tmax = FLT_MAX;

    for (int i = 0; i < 3; i++)
    {
        if (abs(ray.direction[i]) < FLT_EPSILON)
        {
            if (ray.point[i] < aabb.min[i] || ray.point[i] > aabb.max[i])
            {
                return result;
            }
        }
        else
        {
            float ood = 1.0f / ray.direction[i];
            float t1 = (aabb.min[i] - ray.point[i]) * ood;
            float t2 = (aabb.max[i] - ray.point[i]) * ood;

            if (t1 > t2) swap(t1, t2);

            tmin = Math::Max(tmin, t1);
            tmax = Math::Min(tmax, t2);

            if (tmin > tmax) return result;
        }
    }

    result.hit = true;
    result.hitDistance = tmin;
    result.hitPoint = ray.point + (ray.direction * tmin);

    //TODO: not returning hit normal

    return result;
}

RayCastHit CollisionModule::RayCast(Ray ray, Plane plane)
{
    RayCastHit result;
    result.hit = false;

    float denom = Math::dot(plane.normal, ray.direction);
    if (abs(denom) > 0.0001f)
    {
        float t = Math::dot((plane.center - ray.point), plane.normal) / denom;
        if (t >= 0)
        {
            result.hit = true;
            result.hitDistance = t;
            result.hitPoint = ray.point + (ray.direction * t);
        }
    }

    return result;
}

RayCastHit CollisionModule::RayCast(Ray ray, Triangle tri)
{
    return RayCastTri(ray, tri.a, tri.b, tri.c);
}

RayCastHit CollisionModule::RayCast(Ray ray, OctreeNode* node, const Mat4x4f& tempTrans)
{
    if (!RayCast(ray, node->Bounds).hit)
    {
        return RayCastHit();
    }
    if (OctreeDebugDrawEnabled)
    {
        if (node->IsLeaf)
        {
            GraphicsModule::Get()->DebugDrawAABB(node->Bounds, Vec3f(0.95f, 0.35f, 0.25f), tempTrans);
        }
        else
        {
            GraphicsModule::Get()->DebugDrawAABB(node->Bounds, Vec3f(0.2f, 0.9f, 0.1f), tempTrans);
        }
    }

    RayCastHit ClosestHit;
    if (node->IsLeaf)
    {
        for (auto& Tri : node->Triangles)
        {
            RayCastHit TriHit = RayCast(ray, Tri);

            if (OctreeDebugDrawEnabled)
            {
                Triangle TransformedTri = Tri;
                TransformedTri.a = TransformedTri.a * tempTrans;
                TransformedTri.b = TransformedTri.b * tempTrans;
                TransformedTri.c = TransformedTri.c * tempTrans;

                GraphicsModule::Get()->DebugDrawLine(TransformedTri.a, TransformedTri.b, Vec3f(0.5f, 0.5f, 0.9f));
                GraphicsModule::Get()->DebugDrawLine(TransformedTri.b, TransformedTri.c, Vec3f(0.5f, 0.5f, 0.9f));
                GraphicsModule::Get()->DebugDrawLine(TransformedTri.c, TransformedTri.a, Vec3f(0.5f, 0.5f, 0.9f));

            }

            if (TriHit.hit && TriHit.hitDistance < ClosestHit.hitDistance)
            {
                ClosestHit = TriHit;
            }
        }
    }
    else
    {
        for (int i = 0; i < 8; i++)
        {
            RayCastHit SubHit = RayCast(ray, node->SubNodes[i], tempTrans);
            if (SubHit.hit && SubHit.hitDistance < ClosestHit.hitDistance)
            {
                ClosestHit = SubHit;
            }
        }
    }
    return ClosestHit;
}

const RayCastHit* CollisionModule::Closest(std::initializer_list<RayCastHit> hitList)
{
    const RayCastHit* closestHit = hitList.begin();

    for (auto& hit : hitList)
    {
        if (hit.hit && hit.hitDistance < closestHit->hitDistance)
        {
            closestHit = &hit;
        }
    }
    return closestHit;
}

inline RayCastHit CollisionModule::RayCastTri(Ray ray, Vec3f a, Vec3f b, Vec3f c)
{
    // TODO: Become better at math and understand this better :)

    RayCastHit result;
    result.hit = false;

    Vec3f vecAtoB = b - a;
    Vec3f vecAtoC = c - a;

    Vec3f pVec = Math::cross(ray.direction, vecAtoC);
    float det = Math::dot(vecAtoB, pVec);

    if (abs(det) < 0.00001f)
    {
        return result;
    }

    float invDet = 1.0f / det;
    Vec3f tVec = ray.point - a;

    float u = Math::dot(tVec, pVec) * invDet;

    if (u < 0 || u > 1) return result;

    Vec3f qVec = Math::cross(tVec, vecAtoB);
    float v = Math::dot(ray.direction, qVec) * invDet;

    if (v < 0 || u + v > 1.0f) return result;

    float t = Math::dot(vecAtoC, qVec) * invDet;

    if (t > 0.0f)
    {
        result.hit = true;
        result.hitDistance = t;
        result.hitPoint = ray.point + (ray.direction * t);
        result.hitNormal = Math::normalize(Math::cross(vecAtoC, vecAtoB));
    }

    return result;
}

bool Intersects(Triangle t, AABB aabb)
{
    // TODO: Implement correctly
    return aabb.Contains(t.a) || aabb.Contains(t.b) || aabb.Contains(t.c) || aabb.Contains((t.a + t.b + t.c) / 3.0f);
}
