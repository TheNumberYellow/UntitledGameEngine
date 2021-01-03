#include "CollisionGeometry.h"

RayCastHit RayCast(Vec3f origin, Vec3f direction, Triangle triangle)
{
	// TODO: Become better at math and understand this :)

	RayCastHit result;
	result.hit = false;

	Vec3f vecAtoB = *triangle.b - *triangle.a;
	Vec3f vecAtoC = *triangle.c - *triangle.a;

	Vec3f pVec = Math::cross(direction, vecAtoC);
	float det = Math::dot(vecAtoB, pVec);
	
	if (abs(det) < 0.00001f)
	{
		return result;
	}

	float invDet = 1.0f / det;
	Vec3f tVec = origin - *triangle.a;

	float u = Math::dot(tVec, pVec) * invDet;

	if (u < 0 || u > 1) return result;

	Vec3f qVec = Math::cross(tVec, vecAtoB);
	float v = Math::dot(direction, qVec) * invDet;
	
	if (v < 0 || u + v > 1.0f) return result;

	float t = Math::dot(vecAtoC, qVec) * invDet;

	if (t > 0.0f)
	{
		result.hit = true;
		result.hitDistance = t;
		result.hitPoint = origin + (direction * t);
	}

	return result;
}

RayCastHit RayCast(Vec3f origin, Vec3f direction, AABB cube)
{
	return RayCastHit();

}

CollisionGeometry::CollisionGeometry(AABB bounds)
{

}

CollisionGeometry::CollisionGeometry()
{
}

RayCastHit CollisionGeometry::RayCast(Vec3f origin, Vec3f direction)
{
	if (needsOctreeReset)
	{
		recreateOctree();
		needsOctreeReset = false;
	}

	RayCastHit finalRayCast;

	for (int i = 0; i < triangles.size(); i++)
	{
		RayCastHit rayHit = ::RayCast(origin, direction, triangles[i]);
		if (rayHit.hit)
		{
			if (rayHit.hitDistance < finalRayCast.hitDistance)
			{
				finalRayCast.hit = true;
				finalRayCast.hitDistance = rayHit.hitDistance;
				finalRayCast.hitPoint = rayHit.hitPoint;
			}
		}
	}

	return finalRayCast;
}

void CollisionGeometry::AddTriangle(Triangle newTriangle)
{
	triangles.push_back(newTriangle);
	needsOctreeReset = true;
}

void CollisionGeometry::recreateOctree()
{
}
