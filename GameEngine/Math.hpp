#pragma once

#include <stdlib.h>
#include <utility>


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

	Vec3f operator-()
	{
		return Vec3f(-this->x, -this->y, -this->z);
	}

	Vec3f operator*(Vec3f rhs)
	{
		return Vec3f(x * rhs.x, y * rhs.y, z * rhs.z);
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

struct Mat4x4f
{
	Mat4x4f()
	{
		m_Rows[0] = Vec4f(1.0f, 0.0f, 0.0f, 0.0f);
		m_Rows[1] = Vec4f(0.0f, 1.0f, 0.0f, 0.0f);
		m_Rows[2] = Vec4f(0.0f, 0.0f, 1.0f, 0.0f);
		m_Rows[3] = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
	}
	Vec4f m_Rows[4];

	Vec4f& operator[](int index)
	{
		return m_Rows[index];
	}

	Mat4x4f operator*(Mat4x4f rhs);
		

};

struct Line
{
	Vec3f point;
	Vec3f direction;
};

class Math
{
public:
	static float dot(Vec3f leftVec, Vec3f rightVec);
	static Vec3f cross(Vec3f leftVec, Vec3f rightVec);
	static Vec3f rotate(Vec3f inputVec, float radians, Vec3f axis);
	static Vec3f normalize(Vec3f vec);
	static float magnitude(Vec3f vec);

	static Vec3f mult(Vec3f vec, Mat4x4f mat);

	static float clamp(float input, float min, float max);

	static std::pair<Vec3f, Vec3f> ClosestPointsOnLines(Line a, Line b);
};

