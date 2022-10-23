#include "Vector.h"

#include "Math.h"
#include "Quaternion.h"

#include <tuple>

Vec2i Vec2i::operator+(Vec2i rhs)
{
    return Vec2i(this->x + rhs.x, this->y + rhs.y);
}

Vec2i& Vec2i::operator+=(const Vec2i& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;

    return *this;
}

Vec2i Vec2i::operator-(Vec2i rhs)
{
    return Vec2i(this->x - rhs.x, this->y - rhs.y);
}

Vec2i& Vec2i::operator-=(const Vec2i& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;

    return *this;
}

Vec2i::operator Vec2f() const
{
    return Vec2f((float)x, (float)y);
}

Vec2f Vec2f::operator+(Vec2f rhs)
{
    return Vec2f(this->x + rhs.x, this->y + rhs.y);
}

Vec2f& Vec2f::operator+=(const Vec2f& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;

    return *this;
}

Vec2f Vec2f::operator-(Vec2f rhs)
{
    return Vec2f(this->x - rhs.x, this->y - rhs.y);
}

Vec2f& Vec2f::operator-=(const Vec2f& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;

    return *this;
}

Vec2f Vec2f::operator-()
{
    return Vec2f(-this->x, -this->y);
}

bool operator==(const Vec2f& lhs, const Vec2f& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

Vec2f::operator Vec2i() const
{
    return Vec2i((int)x, (int)y);
}

float& Vec3f::operator[](int index)
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
        return x;
    }
}

Vec3f Vec3f::operator+(Vec3f rhs)
{
    return Vec3f(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
}

Vec3f& Vec3f::operator+=(const Vec3f& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;

    return *this;
}

Vec3f Vec3f::operator-(Vec3f rhs)
{
    return Vec3f(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
}

Vec3f& Vec3f::operator-=(const Vec3f& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;

    return *this;
}

Vec3f Vec3f::operator-()
{
    return Vec3f(-this->x, -this->y, -this->z);
}

Vec3f Vec3f::operator*(Vec3f rhs)
{
    return Vec3f(x * rhs.x, y * rhs.y, z * rhs.z);
}

Vec3f Vec3f::operator*(Mat4x4f rhs)
{
    return Math::mult(*this, rhs);
}

Vec3f Vec3f::operator*(Quaternion rhs)
{
    Quaternion thisAsQuat;
    thisAsQuat.x = this->x;
    thisAsQuat.y = this->y;
    thisAsQuat.z = this->z;
    thisAsQuat.w = 0;

    Quaternion resultQuat = thisAsQuat * rhs;
    return Vec3f(resultQuat.x, resultQuat.y, resultQuat.z);
}

Vec3f Vec3f::operator*(float rhs)
{
    return Vec3f(this->x * rhs, this->y * rhs, this->z * rhs);
}

Vec3f Vec3f::operator/(float rhs)
{
    return Vec3f(this->x / rhs, this->y / rhs, this->z / rhs);
}

bool operator==(const Vec3f& lhs, const Vec3f& rhs)
{
    return lhs.x == rhs.x
        && lhs.y == rhs.y
        && lhs.z == rhs.z;
}

float& Vec4f::operator[](int index)
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

Vec4f Vec4f::operator*(Mat4x4f rhs)
{
    return Math::mult(*this, rhs);
}


