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

Vec2f Vec2f::operator/(float rhs)
{
    return Vec2f(this->x / rhs, this->y / rhs);
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

Vec3f Vec3f::XOnly()
{
    return Vec3f(x, 0.0f, 0.0f);
}

Vec3f Vec3f::YOnly()
{
    return Vec3f(0.0f, y, 0.0f);
}

Vec3f Vec3f::ZOnly()
{
    return Vec3f(0.0f, 0.0f, z);
}

Vec3f Vec3f::XYOnly()
{
    return Vec3f(x, y, 0.0f);
}

Vec3f Vec3f::XZOnly()
{
    return Vec3f(x, 0.0f, y);
}

Vec3f Vec3f::YZOnly()
{
    return Vec3f(0.0f, y, z);
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


Vec3f operator*(float lhs, const Vec3f& rhs)
{
    return Vec3f(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

Vec3f Vec3f::operator*(Quaternion rhs)
{
    Vec3f u(rhs.x, rhs.y, rhs.z);

    float s = rhs.w;

    Vec3f vprime = 2.0f * Math::dot(u, *this) * u
        + (s * s - Math::dot(u, u)) * (*this)
        + 2.0f * s * Math::cross(u, *this);

    return vprime;
    //Quaternion thisAsQuat;
    //thisAsQuat.x = this->x;
    //thisAsQuat.y = this->y;
    //thisAsQuat.z = this->z;
    //thisAsQuat.w = 0;

    //Quaternion resultQuat = thisAsQuat * rhs;
    //return Vec3f(resultQuat.x, resultQuat.y, resultQuat.z);
}

Vec3f Vec3f::operator*(float rhs)
{
    return Vec3f(this->x * rhs, this->y * rhs, this->z * rhs);
}

Vec3f Vec3f::operator/(float rhs)
{
    return Vec3f(this->x / rhs, this->y / rhs, this->z / rhs);
}

bool Vec3f::operator<(const Vec3f& rhs)
{
    return x < rhs.x && y < rhs.y && z < rhs.z;
}

bool Vec3f::operator>(const Vec3f& rhs)
{
    return x > rhs.x && y > rhs.y && z > rhs.z;
}

bool Vec3f::operator<=(const Vec3f& rhs)
{
    return x <= rhs.x && y <= rhs.y && z <= rhs.z;
}

bool Vec3f::operator>=(const Vec3f& rhs)
{
    return x >= rhs.x && y >= rhs.y && z >= rhs.z;
}

std::string Vec3f::toString()
{
    std::string out;
    out = "X: [" + std::to_string(x) + "] Y: [ " + std::to_string(y) + "] Z: [" + std::to_string(z) + "]";
    return out;
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


