#pragma once

#include "Math.hpp"

#include <cmath>
#include <float.h>

struct Triangle
{
    Vec3f a, b, c;
};

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};

struct RayCastHit
{
	bool hit = false;
	float hitDistance = FLT_MAX;
	Vec3f hitPoint;
};


RayCastHit RayCast(Ray ray, Triangle triangle);
