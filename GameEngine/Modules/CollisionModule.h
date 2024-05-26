#pragma once

// :/
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include "..\Platform\RendererPlatform.h"
#include "..\Math\Geometry.h"

#include "GraphicsModule.h"

#include <limits> 
#include <unordered_map>


struct Triangle
{
    Vec3f a, b, c;
};

bool Intersects(Triangle t, AABB aabb);

struct OctreeNode
{
    OctreeNode() 
        : SubNodes{}
    {}

    OctreeNode(AABB InBounds) 
        : SubNodes{}
        , Bounds(InBounds) 
    {}

    ~OctreeNode();

    const int MaxTriangles = 25;
    const int MaxDepth = 5;

    AABB Bounds;
    std::vector<Triangle> Triangles;

    bool IsLeaf = true;
    OctreeNode* SubNodes[8];

    void AddTriangle(Triangle t, int tempDepth);
    void AddLevel(int tempDepth);
};

struct RayCastHit
{
    bool hit = false;
    Vec3f hitPoint;
    float hitDistance = std::numeric_limits<float>::max();
    Vec3f hitNormal;

    friend bool operator==(const RayCastHit& lhs, const RayCastHit& rhs)
    {
        return (lhs.hit == rhs.hit && lhs.hitPoint == rhs.hitPoint &&
            lhs.hitDistance == rhs.hitDistance && lhs.hitNormal == rhs.hitNormal);
    }
};

struct Intersection
{
    bool hit = false;

    Vec3f penetrationNormal;
    float penetrationDepth = 0.0f;
};

struct CollisionMesh
{
    CollisionMesh() : OctreeHead(nullptr) {}
    ~CollisionMesh();

    std::vector<Vec3f> points;
    std::vector<ElementIndex> indices;
    AABB boundingBox;

    OctreeNode* OctreeHead;
};

class CollisionModule
{
public:
    CollisionModule(Renderer& renderer);
    ~CollisionModule();

    CollisionMesh* GetCollisionMeshFromMesh(StaticMesh mesh);
    CollisionMesh* GenerateCollisionMeshFromMesh(StaticMesh mesh);

    void InvalidateMeshCollisionData(StaticMesh_ID mesh);

    RayCastHit RayCast(Ray ray, Model& model);
    RayCastHit RayCast(Ray ray, const CollisionMesh& mesh, Transform& transform);
    RayCastHit RayCast(Ray ray, const CollisionMesh& mesh, const Mat4x4f& meshTransform = Mat4x4f());
    RayCastHit RayCast(Ray ray, AABB aabb);
    RayCastHit RayCast(Ray ray, Plane plane);
    RayCastHit RayCast(Ray ray, Triangle tri);
    RayCastHit RayCast(Ray ray, OctreeNode* node, const Mat4x4f& tempTrans);

    Intersection SphereIntersection(Sphere sphere, Sphere other);
    Intersection SphereIntersection(Sphere sphere, Triangle tri);

    static const RayCastHit* Closest(std::initializer_list<RayCastHit> hitList);

    static CollisionModule* Get() { return s_Instance; }

private:
    inline RayCastHit RayCastTri(Ray ray, Vec3f a, Vec3f b, Vec3f c);

    // TODO: At this point the renderer is really just implementation details of the Graphics module;
    // other modules shouldn't be interacting with it, move mesh mapping to Graphics module
    Renderer& m_Renderer;

    std::unordered_map<StaticMesh_ID, CollisionMesh*> m_CollisionMeshMap;

    bool OctreeEnabled = true;
    bool OctreeDebugDrawEnabled = false;

    static CollisionModule* s_Instance;
};