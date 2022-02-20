#pragma once

// :/
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include "..\Platform\RendererPlatform.hpp"
#include "..\Math\Geometry.h"

#include <limits> 

struct RayCastHit
{
    bool hit = false;
    Vec3f hitPoint;
    float hitDistance = std::numeric_limits<float>::max();
    Vec3f hitNormal;
};

struct CollisionMesh
{
    std::vector<Vec3f> points;
    std::vector<ElementIndex> indices;
    AABB boundingBox;
};

class CollisionModule
{
public:
    CollisionModule(Renderer& renderer);
    ~CollisionModule();

    CollisionMesh GenerateCollisionMeshFromMesh(Mesh_ID mesh);

    RayCastHit RayCast(Ray ray, const CollisionMesh& mesh, const Mat4x4f& meshTransform = Mat4x4f());
    RayCastHit RayCast(Ray ray, AABB aabb);
    RayCastHit RayCast(Ray ray, Plane plane);

private:
    inline RayCastHit RayCastTri(Ray ray, Vec3f a, Vec3f b, Vec3f c);
    
    Renderer& m_Renderer;
};