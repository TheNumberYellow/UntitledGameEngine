#include "Quaternion.h"

#include "Math.h"

Quaternion::Quaternion(Vec3f axis, float rotation)
{
    axis = Math::normalize(axis);
    x = axis.x * (float)sin(rotation / 2.0f);
    y = axis.y * (float)sin(rotation / 2.0f);
    z = axis.z * (float)sin(rotation / 2.0f);
    w = (float)cos(rotation / 2.0f);
}

Quaternion Quaternion::operator*(Quaternion rhs)
{
    Quaternion result;

    result.vec =    rhs.vec * this->scalar + 
                    this->vec * rhs.scalar + 
                    Math::cross(this->vec, rhs.vec);
    
    result.scalar = this->scalar * rhs.scalar - 
                    Math::dot(this->vec, rhs.vec);

    return result;
}

Mat4x4f Quaternion::ToMatrix()
{
    Mat4x4f result;
    result[0] = Vec4f(1 - 2 * y * y - 2 * z * z, 2 * x * y + 2 * z * w, 2 * x * z - 2 * y * w, 0);
    result[1] = Vec4f(2 * x * y - 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z + 2 * x * w, 0);
    result[2] = Vec4f(2 * x * z + 2 * y * w, 2 * y * z - 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0);
    result[3] = Vec4f(0, 0, 0, 1);
    return result;
}

Quaternion Quaternion::Conjugate()
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inverse()
{
    Quaternion Result = Conjugate();
    float Scale = Math::dot(*this, *this);
    return Quaternion(Result.x / Scale, Result.y / Scale, Result.z / Scale, Result.w / Scale);
}

float Quaternion::Norm()
{
    return sqrt(x * x + y * y + z * z + w * w);
}

Quaternion Quaternion::GetNormalized()
{
    float n = Norm();
    return Quaternion(x / n, y / n, z / n, w / n);
}

Quaternion Quaternion::FromEuler(float x, float y, float z)
{
    Quaternion QuatAroundX = Quaternion(Vec3f(1.0, 0.0, 0.0), x);
    Quaternion QuatAroundY = Quaternion(Vec3f(0.0, 1.0, 0.0), y);
    Quaternion QuatAroundZ = Quaternion(Vec3f(0.0, 0.0, 1.0), z);

    return QuatAroundZ * QuatAroundY * QuatAroundX;
}
