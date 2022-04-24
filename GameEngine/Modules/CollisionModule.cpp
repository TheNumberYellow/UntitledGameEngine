#include "CollisionModule.h"

CollisionModule::CollisionModule(Renderer& renderer)
    : m_Renderer(renderer)
{

}

CollisionModule::~CollisionModule()
{

}

CollisionMesh& CollisionModule::GetCollisionMeshFromMesh(Mesh_ID mesh)
{
    if (m_CollisionMeshMap.find(mesh) == m_CollisionMeshMap.end())
    {
        GenerateCollisionMeshFromMesh(mesh);
    }
    return m_CollisionMeshMap[mesh];
}

CollisionMesh& CollisionModule::GenerateCollisionMeshFromMesh(Mesh_ID mesh)
{
    CollisionMesh collMesh;

    std::vector<Vertex*> verts = m_Renderer.MapMeshVertices(mesh);
    std::vector<ElementIndex*> indices = m_Renderer.MapMeshElements(mesh);

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

        collMesh.points.push_back(verts[i]->position);
    }
    for (int i = 0; i < indices.size(); ++i)
    {
        collMesh.indices.push_back(*indices[i]);
    }

    collMesh.boundingBox = boundingBox;

    m_Renderer.UnmapMeshVertices(mesh);
    m_Renderer.UnmapMeshElements(mesh);

    m_CollisionMeshMap[mesh] = collMesh;

    return m_CollisionMeshMap[mesh];
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

    float tmin = (aabb.min.x - ray.point.x) / ray.direction.x;
    float tmax = (aabb.max.x - ray.point.x) / ray.direction.x;

    if (tmin > tmax) swap(tmin, tmax);

    float tymin = (aabb.min.y - ray.point.y) / ray.direction.y;
    float tymax = (aabb.max.y - ray.point.y) / ray.direction.y;

    if (tymin > tymax) swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
    {
        return result;
    }

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (aabb.min.z - ray.point.z) / ray.direction.z;
    float tzmax = (aabb.max.z - ray.point.z) / ray.direction.z;

    if (tzmin > tzmax) swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return result;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    result.hit = true;
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
