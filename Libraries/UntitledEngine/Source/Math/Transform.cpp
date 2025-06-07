#include "Transform.h"

#include "Math/Math.h"

void Transform::SetPosition(Vec3f newPos)
{
    m_Position = newPos;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::SetScale(Vec3f newScale)
{
    m_Scale = newScale;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::SetScale(float newScale)
{
    m_Scale = Vec3f(newScale, newScale, newScale);
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::SetRotation(Quaternion newRotation)
{
    m_Rotation = newRotation;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::Move(Vec3f move)
{
    m_Position += move;
    m_TransformMatrixNeedsUpdate = true;
}

void Transform::Scale(Vec3f scale)
{
    m_Scale.x *= scale.x;
    m_Scale.y *= scale.y;
    m_Scale.z *= scale.z;

    m_TransformMatrixNeedsUpdate = true;
}

void Transform::Rotate(Quaternion rotation)
{
    m_Rotation = m_Rotation * rotation;

    m_TransformMatrixNeedsUpdate = true;
}

void Transform::RotateAroundPoint(Vec3f point, Quaternion rotation)
{
    Mat4x4f RotMat = rotation.ToMatrix();

    Mat4x4f TransMat = Math::Translate(Mat4x4f(), point);
    Mat4x4f RevTransMat = Math::Translate(Mat4x4f(), -point);

    Mat4x4f TotalTrans = TransMat * RotMat * RevTransMat;

    SetTransformMatrix(TransMat * RotMat * RevTransMat * GetTransformMatrix());
}

void Transform::SetParent(Transform* inParent)
{
    m_Parent = inParent;
}

Mat4x4f Transform::GetTransformMatrix()
{
    if (m_TransformMatrixNeedsUpdate)
    {
        UpdateTransformMatrix();
        m_TransformMatrixNeedsUpdate = false;
    }
    if (m_Parent)
    {
        m_Transform = m_Transform * m_Parent->GetTransformMatrix();
    }

    return m_Transform;
}

void Transform::SetTransformMatrix(Mat4x4f mat)
{
    m_Transform = mat;

    Math::DecomposeMatrix(m_Transform, m_Position, m_Rotation, m_Scale);

    m_TransformMatrixNeedsUpdate = false;
}

void Transform::UpdateTransformMatrix()
{
    m_Transform = Math::GenerateTransformMatrix(m_Position, m_Scale, m_Rotation);
}
