#pragma once
#include <vector>

#include "Math.hpp"

// TODO: move these definitions when/if I want to use them elsewhere

inline float Center(float lhs, float rhs)
{
	return (lhs + rhs) * 0.5f;
}

inline Vec3f Center(Vec3f lhs, Vec3f rhs)
{
	return (lhs + rhs) * 0.5f;
}

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

struct Triangle
{
	Vec3f *a;
	Vec3f *b;
	Vec3f *c;
};

struct AABB
{
	AABB(Vec3f min, Vec3f max) : min(min), max(max)
	{
	}
	Vec3f min;
	Vec3f max;
};

struct OctreeNode
{
	OctreeNode(AABB nodeBounds) : nodeBounds(nodeBounds), childNodes(), isLeaf(true)
	{
	}

	OctreeNode(Vec3f min, Vec3f max) : OctreeNode(AABB(min, max))
	{

	}

	void AddLevel()
	{
		isLeaf = false;

		Vec3f middle = Center(nodeBounds.min, nodeBounds.max);
		
		Vec3f maxXCenter = Vec3f(nodeBounds.max.x, Center(nodeBounds.min.y, nodeBounds.max.y), Center(nodeBounds.min.z, nodeBounds.max.z));
		Vec3f minXCenter = Vec3f(nodeBounds.min.x, Center(nodeBounds.min.y, nodeBounds.max.y), Center(nodeBounds.min.z, nodeBounds.max.z));
		
		Vec3f maxYCenter = Vec3f(Center(nodeBounds.min.x, nodeBounds.max.x), nodeBounds.max.y, Center(nodeBounds.min.z, nodeBounds.max.z));
		Vec3f minYCenter = Vec3f(Center(nodeBounds.min.x, nodeBounds.max.x), nodeBounds.min.y, Center(nodeBounds.min.z, nodeBounds.max.z));

		Vec3f maxZCenter = Vec3f(Center(nodeBounds.min.x, nodeBounds.max.x), Center(nodeBounds.min.y, nodeBounds.max.y), nodeBounds.max.z);
		Vec3f minZCenter = Vec3f(Center(nodeBounds.min.x, nodeBounds.max.x), Center(nodeBounds.min.y, nodeBounds.max.y), nodeBounds.min.z);

		Vec3f minYminXEdgeCenter = Vec3f(Center(nodeBounds.min, nodeBounds.min + Vec3f(0.0f, 0.0f, nodeBounds.max.z - nodeBounds.min.z)));


		//childNodes[0] = new OctreeNode(nodeBounds.min, middle);
		//childNodes[1] = new OctreeNode();
		//childNodes[2] = new OctreeNode(minXCenter, maxYCenter);
		//childNodes[3] = new OctreeNode(minXCenter, maxZCenter);
		//
		//childNodes[4] = new OctreeNode();
		//childNodes[5] = new OctreeNode();
		//childNodes[6] = new OctreeNode();
		//childNodes[7] = new OctreeNode();

	}
	
	AABB nodeBounds;
	bool isLeaf;
	OctreeNode* childNodes[8];
};

class CollisionGeometry
{
public:
	CollisionGeometry(AABB bounds);
	CollisionGeometry();
	RayCastHit RayCast(Vec3f origin, Vec3f direction);
	void AddTriangle(Triangle newTriangle);
private:

	

	std::vector<Triangle> triangles;

	bool needsOctreeReset = true;
	void recreateOctree();
};

