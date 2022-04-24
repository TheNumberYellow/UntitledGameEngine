#pragma once

#include "Math.h"

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
    Line(Vec3f point, Vec3f direction) : point(point), direction(direction) {}
    Vec3f point;
    Vec3f direction;
};

struct Plane
{
    Vec3f center;
    Vec3f normal;
};

struct LineSegment
{
    LineSegment(Vec3f a, Vec3f b) : a(a), b(b) {}
    Vec3f a, b;
};

struct Rect
{
    Rect() : location(Vec2f(0.0f, 0.0f)), size(Vec2f(0.0f, 0.0f)) {}
    //Rect(Vec2f bottomLeft, Vec2f topRight)
    //{
    //    location = bottomLeft;
    //    size = topRight - bottomLeft;
    //}

    Rect(Vec2f position, Vec2f size)
    {
        this->location = position;
        this->size = size;
    }

    bool contains(Vec2f point)
    {
        return point.x > location.x && point.y > location.y &&
            point.x < (location + size).x && point.y < (location + size).y;
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

    Vec3f min;
    Vec3f max;
};