#include "Collisions.hpp"

RayCastHit RayCast(Ray ray, Triangle triangle)
{
	// TODO: Become better at math and understand this :)

	RayCastHit result;
	result.hit = false;

	Vec3f vecAtoB = triangle.b - triangle.a;
	Vec3f vecAtoC = triangle.c - triangle.a;

	Vec3f pVec = Math::cross(ray.direction, vecAtoC);
	float det = Math::dot(vecAtoB, pVec);

	if (abs(det) < 0.00001f)
	{
		return result;
	}

	float invDet = 1.0f / det;
	Vec3f tVec = ray.origin - triangle.a;

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
		result.hitPoint = ray.origin + (ray.direction * t);
	}

	return result;
}
