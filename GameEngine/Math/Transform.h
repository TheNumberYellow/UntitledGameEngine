#pragma once
#include "Math.h"
#include "Quaternion.h"
#include "Vector.h"

class Transform
{
public:
    void SetPosition(Vec3f newPos);
    void SetScale(Vec3f newScale);
    void SetScale(float newScale);
    void SetRotation(Quaternion newRotation);

    Vec3f GetPosition() { return m_Position; }
    Vec3f GetScale() { return m_Scale; }
    Quaternion GetRotation() { return m_Rotation; }

    void Move(Vec3f move);
    void Scale(Vec3f scale);
    void Rotate(Quaternion rotation);

    void RotateAroundPoint(Vec3f point, Quaternion rotation);

    Mat4x4f GetTransformMatrix();
    void SetTransformMatrix(Mat4x4f mat);

private:

    void UpdateTransformMatrix();

    Vec3f m_Position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f m_Scale = Vec3f(1.0f, 1.0f, 1.0f);
    Quaternion m_Rotation;

    Mat4x4f m_Transform;
    bool m_TransformMatrixNeedsUpdate = false;
};