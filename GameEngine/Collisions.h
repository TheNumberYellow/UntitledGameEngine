#pragma once
//
//#include "Math\Math.hpp"
//#include "Platform\RendererPlatform.hpp"
//
//#include <cmath>
//#include <vector>
//#include <float.h>
//
//struct Triangle
//{
//    Vec3f a, b, c;
//};
//
//struct Plane
//{
//    Vec3f center;
//    Vec3f normal;
//};
//
//struct Ray
//{
//    Ray() {}
//    Ray(Vec3f origin, Vec3f direction) : origin(origin), direction(direction) {}
//    Vec3f origin;
//    Vec3f direction;
//};
//
//struct RayCastHit
//{
//    bool hit = false;
//    float hitDistance = FLT_MAX;
//    Vec3f hitPoint;
//};
//
//struct CollisionMesh
//{
//    std::vector<Triangle> m_Triangles;
//};
//
//namespace Collisions
//{
//    RayCastHit RayCast(Ray ray, Triangle triangle);
//    RayCastHit RayCast(Ray ray, CollisionMesh collisionMesh);
//
//    RayCastHit RayCast(Ray ray, Triangle triangle, Mat4x4f transform);
//    RayCastHit RayCast(Ray ray, CollisionMesh collisionMesh, Mat4x4f transform);
//
//    RayCastHit RayCast(Ray ray, Plane plane);
//
//    RayCastHit FirstHit(RayCastHit left, RayCastHit right);
//
//    CollisionMesh GenerateCollisionGeometryFromMesh(Mesh_ID mesh, Renderer* renderer);
//
//    Triangle TransformTriangle(Triangle triangle, Mat4x4f transform);
//
//
//}
