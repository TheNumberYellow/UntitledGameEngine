#pragma once

struct Quaternion;
struct Mat4x4f;

#include <assert.h>
#include <functional>

struct Vec2f;

struct Vec2i
{
    int x, y;
    Vec2i(int x, int y) : x(x), y(y) {}
    Vec2i() : x(0), y(0) {}
    
    Vec2i operator+(Vec2i rhs);
    Vec2i& operator+=(const Vec2i& rhs);

    Vec2i operator-(Vec2i rhs);
    Vec2i& operator-=(const Vec2i& rhs);
  
    operator Vec2f() const;
};

struct Vec2f
{
    float x, y;
    Vec2f(float x, float y) : x(x), y(y) {}
    Vec2f() : x(0.0f), y(0.0f) {}

    Vec2f operator+(Vec2f rhs);
    Vec2f& operator+=(const Vec2f& rhs);

    Vec2f operator-(Vec2f rhs);
    Vec2f& operator-=(const Vec2f& rhs);

    Vec2f operator-();

    friend bool operator==(const Vec2f& lhs, const Vec2f& rhs);

    operator Vec2i() const;
};

struct Vec3f
{
    float x, y, z;

    Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}

    float& operator[] (int index);

    Vec3f operator+(Vec3f rhs);
    Vec3f& operator+=(const Vec3f& rhs);

    Vec3f operator-(Vec3f rhs);
    Vec3f& operator-=(const Vec3f& rhs);

    Vec3f operator-();

    Vec3f operator*(Vec3f rhs);
    Vec3f operator*(Quaternion rhs);
    Vec3f operator*(Mat4x4f rhs);

    Vec3f operator*(float rhs);
    Vec3f operator/(float rhs);

    friend bool operator==(const Vec3f& lhs, const Vec3f& rhs);

};

class Vec3fHash
{
public:
    size_t operator()(const Vec3f& v) const noexcept
    {
        size_t h1 = std::hash<float>{}(v.x);
        size_t h2 = std::hash<float>{}(v.y);
        size_t h3 = std::hash<float>{}(v.z);

        return (h1 ^ (h2 << 1)) ^ (h3 << 2);
    }
};

struct Vec4f
{
    float x, y, z, w;
    Vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4f() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {} // TODO: come back to this if I'm always setting w to 1.0f

    float& operator[] (int index);

    Vec4f operator*(Mat4x4f rhs);
};