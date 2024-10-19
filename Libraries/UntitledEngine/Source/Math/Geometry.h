#pragma once

// :/
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include "Math.h"

#include <limits> 
#include <sstream>  

struct Ray
{
    Ray() {}
    Ray(Vec3f point, Vec3f direction) : point(point), direction(direction) {}
    Vec3f point;
    Vec3f direction;
};

struct Line
{
    Line() {}
    Line(Vec3f point, Vec3f direction) : point(point), direction(direction) {}
    Vec3f point;
    Vec3f direction;
};

struct Plane
{
    Plane() {}
    Plane(Vec3f center, Vec3f normal) : center(center), normal(normal) {}
    Vec3f center;
    Vec3f normal;
};

struct Sphere
{
    Vec3f position;
    float radius;
};

struct Capsule
{
    Vec3f top, bottom;
    float radius;
};

struct LineSegment
{
    LineSegment() {}
    LineSegment(Vec3f a, Vec3f b) : a(a), b(b) {}
    Vec3f a, b;
};

struct Rect
{
    Rect() : location(Vec2f(0.0f, 0.0f)), size(Vec2f(0.0f, 0.0f)) {}

    Rect(Vec2f position, Vec2f size)
    {
        this->location = position;
        this->size = size;
    }

    Rect(Vec2f size)
    {
        this->location = Vec2f(0.0f, 0.0f);
        this->size = size;
    }

    bool Contains(Vec2f point)
    {
        return point.x >= location.x && point.y >= location.y &&
            point.x <= (location + size).x && point.y <= (location + size).y;
    }

    bool Contains(Rect other)
    {
        return (Contains(other.location) && Contains(other.location + other.size));
    }

    bool Overlaps(Rect other)
    {
        return location.x < other.location.x + other.size.x
            && location.y < other.location.y + other.size.y
            && location.x + size.x > other.location.x
            && location.y + size.y > other.location.y;
    }

    Vec2f Center()
    {
        return this->location + (this->size * 0.5f);
    }

    bool IsZero()
    {
        return location.IsZero() && size.IsZero();
    }

    // Expand the rect to include this point
    void expand(Vec2f point)
    {
        if (point.x < location.x) location.x = point.x;
        if (point.y < location.y) location.y = point.y;

        if (point.x > (location.x + size.x)) size.x = point.x - location.x;
        if (point.y > (location.y + size.y)) size.y = point.y - location.y;
    }

    friend bool operator==(const Rect& lhs, const Rect& rhs)
    {
        return lhs.location == rhs.location && lhs.size == rhs.size;
    }

    Vec2f location, size;
};

struct Recti
{
    Recti() : location(Vec2i(0, 0)), size(Vec2i(0, 0)) {}
    Recti(Vec2i bottomLeft, Vec2i topRight)
    {
        location = bottomLeft;
        size = topRight - bottomLeft;
    }

    bool contains(Vec2i point)
    {
        return point.x > location.x && point.y > location.y &&
            point.x < (location + size).x&& point.y < (location + size).y;
    }

    Vec2i location, size;
};

struct Quad
{

};

struct AABB
{
    AABB() {}
    AABB(Vec3f min, Vec3f max) { this->min = min; this->max = max; }

    bool Contains(Vec3f p)
    {
        return p >= min && p <= max;
    }

    Vec3f Center()
    {
        return min + (0.5f * (max - min));
    }

    float XSize()
    {
        return max.x - min.x;
    }

    float YSize()
    {
        return max.y - min.y;
    }

    float ZSize()
    {
        return max.z - min.z;
    }

    void Expand(Vec3f point)
    {
        if (point.x < min.x) min.x = point.x;
        if (point.y < min.y) min.y = point.y;
        if (point.z < min.z) min.z = point.z;

        if (point.x > max.x) max.x = point.x;
        if (point.y > max.y) max.y = point.y;
        if (point.z > max.z) max.z = point.z;

    }

    //AABB AABBFromTransformedAABB(AABB& InAABB, Transform& InTransform);

    Vec3f min;
    Vec3f max;
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