#pragma once

#include <stdlib.h>

float RandomFloat(float min, float max);

struct Vec2i
{
	int x, y;
	Vec2i(int x, int y) : x(x), y(y) {}
	Vec2i() : x(0), y(0) {}
};

struct Vec2f
{
	float x, y;
	Vec2f(float x, float y) : x(x), y(y) {}
	Vec2f() : x(0.0f), y(0.0f) {}
};

struct Vec3f
{
	float x, y, z;
	Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}

	Vec3f operator+(Vec3f rhs)
	{
		return Vec3f(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
	}

	Vec3f& operator+=(const Vec3f& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;

		return *this;
	}
	Vec3f operator-(Vec3f rhs)
	{
		return Vec3f(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	}

	Vec3f& operator-=(const Vec3f& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		
		return *this;
	}

	Vec3f operator*(float rhs)
	{
		return Vec3f(this->x * rhs, this->y * rhs, this->z * rhs);
	}
	Vec3f operator/(float rhs)
	{
		return Vec3f(this->x / rhs, this->y / rhs, this->z / rhs);
	}
};

struct Vec4f
{
	float x, y, z, w;
	Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Vec4f() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {} // TODO: come back to this if I'm always setting w to 1.0f
};

class Math
{
public:
	static float dot(Vec3f leftVec, Vec3f rightVec);
	static Vec3f cross(Vec3f leftVec, Vec3f rightVec);
	static Vec3f rotate(Vec3f inputVec, float radians, Vec3f axis);
	static Vec3f normalize(Vec3f vec);

	static float clamp(float input, float min, float max);
};

