#include "Math.h"

#include "Vector.h"
#include "Quaternion.h"

#include "../GameEngine.h"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

float Math::dot(Quaternion leftQuat, Quaternion rightQuat)
{
    return (leftQuat.x * rightQuat.x) + (leftQuat.y * rightQuat.y) + (leftQuat.z * rightQuat.z) + (leftQuat.w * rightQuat.w);
}

float Math::dot(Vec3f leftVec, Vec3f rightVec)
{
    float result = (leftVec.x * rightVec.x) + (leftVec.y * rightVec.y) + (leftVec.z * rightVec.z);
    return result;
}

float Math::dot(Vec2f leftVec, Vec2f rightVec)
{
    float result = (leftVec.x * rightVec.x) + (leftVec.y * rightVec.y);
    return result;
}

Vec3f Math::cross(Vec3f leftVec, Vec3f rightVec)
{
    Vec3f result;
    result.x = leftVec.y * rightVec.z - leftVec.z * rightVec.y;
    result.y = leftVec.z * rightVec.x - leftVec.x * rightVec.z;
    result.z = leftVec.x * rightVec.y - leftVec.y * rightVec.x;
    return result;
}

float Math::cross(Vec2f leftVec, Vec2f rightVec)
{
    return (leftVec.x * rightVec.y) - (leftVec.y * rightVec.x);
}

Vec3f Math::rotate(Vec3f inputVec, float radians, Vec3f axis)
{
    glm::vec3 glmVec = glm::vec3(inputVec.x, inputVec.y, inputVec.z);
    glmVec = glm::rotate(glmVec, radians, glm::vec3(axis.x, axis.y, axis.z));
    return Vec3f(glmVec.x, glmVec.y, glmVec.z);
}

Vec3f Math::normalize(Vec3f vec)
{
    float mag_inv = 1.0f / magnitude(vec);
    return vec * mag_inv;
}

float Math::magnitude(Vec3f vec)
{
    return sqrt(lenSquared(vec));
}

float Math::magnitude(Vec2f vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float Math::lenSquared(Vec3f vec)
{
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

Quaternion Math::normalize(Quaternion quat)
{
    float n = norm(quat);

    return Quaternion(quat.x / n, quat.y / n, quat.z / n, quat.w / n);
}

Vec3f orthogonal(Vec3f v)
{
    Vec3f X_AXIS = Vec3f(1.0f, 0.0f, 0.0f);
    Vec3f Y_AXIS = Vec3f(0.0f, 1.0f, 0.0f);
    Vec3f Z_AXIS = Vec3f(0.0f, 0.0f, 1.0f);

    float x = abs(v.x);
    float y = abs(v.y);
    float z = abs(v.z);

    Vec3f other = x < y ? (x < z ? X_AXIS : Z_AXIS) : (y < z ? Y_AXIS : Z_AXIS);
    return Math::cross(v, other);
}

Quaternion Math::VecDiffToQuat(Vec3f v1, Vec3f v2)
{
    float k_cos_theta = Math::dot(v1, v2);
    float k = sqrt(lenSquared(v1) * lenSquared(v2));

    Quaternion result;

    if (k_cos_theta / k == -1.0f)
    {
        result.w = 0.0f;
        result.vec = Math::normalize(orthogonal(v1));
    }
    else
    {
        result.w = k_cos_theta + k;
        result.vec = Math::cross(v2, v1);
    }

    return Math::normalize(result);
}

float Math::norm(Quaternion quat)
{
    return sqrt(quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w);
}

float Math::clamp(float input, float min, float max)
{
    return input < min ? min : (input > max ? max : input);
}

Mat4x4f Math::inv(Mat4x4f mat)
{
    glm::mat4 glmMat;

    glmMat[0] = glm::vec4(mat[0].x, mat[0].y, mat[0].z, mat[0].w);
    glmMat[1] = glm::vec4(mat[1].x, mat[1].y, mat[1].z, mat[1].w);
    glmMat[2] = glm::vec4(mat[2].x, mat[2].y, mat[2].z, mat[2].w);
    glmMat[3] = glm::vec4(mat[3].x, mat[3].y, mat[3].z, mat[3].w);

    glmMat = glm::inverse(glmMat);

    Mat4x4f result;
    
    result[0] = Vec4f(glmMat[0].x, glmMat[0].y, glmMat[0].z, glmMat[0].w);
    result[1] = Vec4f(glmMat[1].x, glmMat[1].y, glmMat[1].z, glmMat[1].w);
    result[2] = Vec4f(glmMat[2].x, glmMat[2].y, glmMat[2].z, glmMat[2].w);
    result[3] = Vec4f(glmMat[3].x, glmMat[3].y, glmMat[3].z, glmMat[3].w);

    return result;
}

Vec4f Math::mult(Vec4f vec, Mat4x4f mat)
{
    Vec4f result = vec;

    result[0] = mat[0][0] * vec.x + mat[1][0] * vec.y + mat[2][0] * vec.z + mat[3][0] * vec.w;
    result[1] = mat[0][1] * vec.x + mat[1][1] * vec.y + mat[2][1] * vec.z + mat[3][1] * vec.w;
    result[2] = mat[0][2] * vec.x + mat[1][2] * vec.y + mat[2][2] * vec.z + mat[3][2] * vec.w;
    result[3] = mat[0][3] * vec.x + mat[1][3] * vec.y + mat[2][3] * vec.z + mat[3][3] * vec.w;

    return result;
}

Vec3f Math::mult(Vec3f vec, Mat4x4f mat)
{
    Vec4f point = Vec4f(vec.x, vec.y, vec.z, 1.0f);

    Vec4f result;

    result[0] = mat[0][0] * point.x + mat[1][0] * point.y + mat[2][0] * point.z + mat[3][0] * point.w;
    result[1] = mat[0][1] * point.x + mat[1][1] * point.y + mat[2][1] * point.z + mat[3][1] * point.w;
    result[2] = mat[0][2] * point.x + mat[1][2] * point.y + mat[2][2] * point.z + mat[3][2] * point.w;

    return Vec3f(result.x, result.y, result.z);
}

Mat4x4f Math::Translate(Mat4x4f mat, Vec3f translation)
{
    Mat4x4f result;
    result[3][0] = translation.x;
    result[3][1] = translation.y;
    result[3][2] = translation.z;

    result = mat * result;

    return result;
}

Mat4x4f Math::Scale(Mat4x4f mat, Vec3f scale)
{
    Mat4x4f result;
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;

    result = mat * result;
    
    return result;
}

Mat4x4f Math::Rotate(Mat4x4f mat, Quaternion rotation)
{
    Mat4x4f result = mat * rotation.ToMatrix();
    return result;
}

Mat4x4f Math::GenerateTransformMatrix(Vec3f position, Vec3f scale, Quaternion rotation)
{
    Mat4x4f result;

    result = Translate(result, position);
    result = Rotate(result, rotation);
    result = Scale(result, scale);

    return result;
}

Mat4x4f Math::GenerateViewMatrix(Vec3f position, Vec3f direction, Vec3f up)
{
    Vec3f f(normalize(direction));
    Vec3f s(normalize(cross(f, up)));
    Vec3f u(cross(s, f));

    Mat4x4f result;
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;
    result[3][0] = -dot(s, position);
    result[3][1] = -dot(u, position);
    result[3][2] = dot(f, position);

    return result;
}

Mat4x4f Math::GenerateProjectionMatrix(float verticalFOV, float aspectRatio, float nearClippingPlane, float farClippingPlane)
{
    glm::mat4 glmMat;
    glmMat = glm::perspective(verticalFOV, aspectRatio, nearClippingPlane, farClippingPlane);

    Mat4x4f result;

    result[0] = Vec4f(glmMat[0].x, glmMat[0].y, glmMat[0].z, glmMat[0].w);
    result[1] = Vec4f(glmMat[1].x, glmMat[1].y, glmMat[1].z, glmMat[1].w);
    result[2] = Vec4f(glmMat[2].x, glmMat[2].y, glmMat[2].z, glmMat[2].w);
    result[3] = Vec4f(glmMat[3].x, glmMat[3].y, glmMat[3].z, glmMat[3].w);

    return result;
}

Mat4x4f Math::GenerateOrthoMatrix(float left, float right, float bottom, float top, float near, float far)
{
    Mat4x4f result;

    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = -2.0f / (far - near);
    result[3][3] = 1.0f;

    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(far + near) / (far - near);

    return result;
}


void Math::DecomposeMatrix(Mat4x4f matrix, Vec3f& OutTranslation, Quaternion& OutRotation, Vec3f& OutScale)
{
    glm::mat4 glmMat;
    glmMat[0] = glm::vec4(matrix[0].x, matrix[0].y, matrix[0].z, matrix[0].w);
    glmMat[1] = glm::vec4(matrix[1].x, matrix[1].y, matrix[1].z, matrix[1].w);
    glmMat[2] = glm::vec4(matrix[2].x, matrix[2].y, matrix[2].z, matrix[2].w);
    glmMat[3] = glm::vec4(matrix[3].x, matrix[3].y, matrix[3].z, matrix[3].w);

    glm::vec3 glmTranslation;
    glm::quat glmRotation;
    glm::vec3 glmScale;
    glm::vec3 glmSkew;
    glm::vec4 glmPerspective;

    glm::inverse(glmRotation);

    glm::decompose(glmMat, glmScale, glmRotation, glmTranslation, glmSkew, glmPerspective);

    OutTranslation.x = glmTranslation.x;
    OutTranslation.y = glmTranslation.y;
    OutTranslation.z = glmTranslation.z;

    OutRotation.x = glmRotation.x;
    OutRotation.y = glmRotation.y;
    OutRotation.z = glmRotation.z;
    OutRotation.w = glmRotation.w;

    OutScale.x = glmScale.x;
    OutScale.y = glmScale.y;
    OutScale.z = glmScale.z;
}

std::pair<Vec3f, Vec3f> Math::ClosestPointsOnLines(Line a, Line b)
{
    // Get the direction of the line perpendicular to both lines
    Vec3f n = Math::cross(a.direction, b.direction);

    Vec3f n1 = Math::cross(a.direction, n);
    Vec3f n2 = Math::cross(b.direction, n);

    Vec3f c1 = a.point + (a.direction * ((Math::dot(b.point - a.point, n2)) / Math::dot(a.direction, n2)));

    Vec3f c2 = b.point + (b.direction * ((Math::dot(a.point - b.point, n1)) / Math::dot(b.direction, n1)));

    return std::pair<Vec3f, Vec3f>(c1, c2);
}

Vec3f Math::ClosestPointOnPlaneToPoint(Plane plane, Vec3f point)
{
    float dist = Math::dot(plane.normal, point) - Math::dot(plane.normal, plane.center);

    return point - (dist * plane.normal);
}

float Math::Max(float a, float b)
{
    return a > b ? a : b;
}

float Math::Min(float a, float b)
{
    return a < b ? a : b;
}

float Math::Lerp(float a, float b, float t)
{
    return (1.0f - t) * a + b * t;
}

float Math::InvLerp(float a, float b, float v)
{
    return (v - a) / (b - a);
}

float Math::Remap(float iMin, float iMax, float oMin, float oMax, float v)
{
    float t = InvLerp(iMin, iMax, v);
    return Lerp(oMin, oMax, t);
}

float Math::SmoothStep(float in, float edge0, float edge1)
{
    in = Clamp((in - edge0) / (edge1 - edge0));
    
    return in * in * (3.0f - 2.0f * in);
}

float Math::Clamp(float in, float lower, float upper)
{
    if (in < lower) return lower;
    if (in > upper) return upper;
    return in;
}

float Math::RandomFloat(float min, float max)
{
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}

float Math::Round(float num, float multiple)
{
    num /= multiple;
    num = round(num);
    num *= multiple;

    return num;
}

Mat4x4f::Mat4x4f()
{
    m_Rows[0] = Vec4f(1.0f, 0.0f, 0.0f, 0.0f);
    m_Rows[1] = Vec4f(0.0f, 1.0f, 0.0f, 0.0f);
    m_Rows[2] = Vec4f(0.0f, 0.0f, 1.0f, 0.0f);
    m_Rows[3] = Vec4f(0.0f, 0.0f, 0.0f, 1.0f);
}

Vec4f& Mat4x4f::operator[](int index)
{
    return m_Rows[index];
}

Mat4x4f Mat4x4f::operator*(Mat4x4f rhs)
{
    Mat4x4f result;

    result[0][0] = 0;
    result[1][1] = 0;
    result[2][2] = 0;
    result[3][3] = 0;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                result[j][i] += (*this)[k][i] * rhs[j][k];
            }
        }
    }
    return result;
}