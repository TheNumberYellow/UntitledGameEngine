#pragma once

#include "Vector.h"

#define _USE_MATH_DEFINES
#include <math.h>

struct Mat4x4f;

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
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Quaternion(Vec3f axis, float rotation);

    Quaternion operator*(Quaternion rhs);

    Mat4x4f ToMatrix();

    Quaternion Conjugate();
    Quaternion Inverse();

};
