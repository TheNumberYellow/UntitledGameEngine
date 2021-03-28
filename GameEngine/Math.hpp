#pragma once

#include <stdlib.h>
#include <utility>

#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

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

	float& operator[] (int index)
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			assert(false && "Vec3f access out of range.");
		}
	}

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

	float& operator[] (int index)
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		default:
			assert(false && "Vec4f access out of range.");
			return x;
		}
	}
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

	static float Pi() { return (float)M_PI; }

	static Mat4x4f inv(Mat4x4f mat);

	static Mat4x4f GenerateViewMatrix(Vec3f position, Vec3f direction, Vec3f up = Vec3f(0.0f, 0.0f, 1.0f));
	static Mat4x4f GenerateProjectionMatrix(float verticalFOV, float aspectRatio, float nearClippingPlane, float farClippingPlane);

	static std::pair<Vec3f, Vec3f> ClosestPointsOnLines(Line a, Line b);
};

struct Quaternion
{
	union
	{
		struct
		{
			float x, y, z;
		};
		Vec3f vec;
	};
	union
	{
		float scalar;
		float w;
	};

	Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {} // Identity quaternion
	Quaternion(Vec3f axis, float rotation)
	{
		x = axis.x * (float)sin(rotation / 2.0f);
		y = axis.y * (float)sin(rotation / 2.0f);
		z = axis.z * (float)sin(rotation / 2.0f);
		w = (float)cos(rotation / 2.0f);
	}

	Quaternion operator*(Quaternion rhs)
	{
		Quaternion result;

		result.vec = rhs.vec * this->scalar + this->vec * rhs.scalar + Math::cross(this->vec, rhs.vec);
		result.scalar = this->scalar * rhs.scalar - Math::dot(this->vec, rhs.vec);

		return result;
	}

	Mat4x4f ToMatrix()
	{
		Mat4x4f result;
		result[0] = Vec4f(1 - 2 * y * y - 2 * z * z, 2 * x * y + 2 * z * w, 2 * x * z - 2 * y * w, 0);
		result[1] = Vec4f(2 * x * y - 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z + 2 * x * w, 0);
		result[2] = Vec4f(2 * x * z + 2 * y * w, 2 * y * z - 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0);
		result[3] = Vec4f(0, 0, 0, 1);
		return result;
	}
};

