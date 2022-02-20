#pragma once

#include "Vector.h"
#include "Geometry.h"

#include <stdlib.h>
#include <utility>

#include <assert.h>

#define _USE_MATH_DEFINES
#include <math.h>

#define Deg2Rad(deg) (deg * 0.0174533f)
#define Rad2Deg(rad) (rad * 57.2957549575f)

// Forward declares
struct Quaternion;


struct Range
{
    float min, max;
};

struct Mat4x4f
{
    Mat4x4f();
    Vec4f m_Rows[4];

    Vec4f& operator[](int index);

    Mat4x4f operator*(Mat4x4f rhs);
};

class Math
{
public:    
    static float RandomFloat(float min, float max);

    static float Round(float num, float multiple = 1.0f);

    static float dot(Vec3f leftVec, Vec3f rightVec);
    static Vec3f cross(Vec3f leftVec, Vec3f rightVec);
    static Vec3f rotate(Vec3f inputVec, float radians, Vec3f axis);

    static Vec3f normalize(Vec3f vec);
    static float magnitude(Vec3f vec);

    static Quaternion normalize(Quaternion quat);
    static float norm(Quaternion quat);

    static float clamp(float input, float min, float max);

    static float Pi() { return (float)M_PI; }

    static Mat4x4f inv(Mat4x4f mat);

    static Vec4f mult(Vec4f vec, Mat4x4f mat);
    static Vec3f mult(Vec3f vec, Mat4x4f mat);

    static Mat4x4f Translate(Mat4x4f mat, Vec3f translation);
    static Mat4x4f Scale(Mat4x4f mat, Vec3f scale);
    static Mat4x4f Rotate(Mat4x4f mat, Quaternion rotation);

    static Mat4x4f GenerateTransformMatrix(Vec3f position, Vec3f scale, Quaternion rotation);
    static Mat4x4f GenerateViewMatrix(Vec3f position, Vec3f direction, Vec3f up = Vec3f(0.0f, 0.0f, 1.0f));
    static Mat4x4f GenerateProjectionMatrix(float verticalFOV, float aspectRatio, float nearClippingPlane, float farClippingPlane);
    static Mat4x4f GenerateOrthoMatrix(float left, float right, float bottom, float top, float nearClippingPlane, float farClippingPlane);

    static std::pair<Vec3f, Vec3f> ClosestPointsOnLines(Line a, Line b);

};
