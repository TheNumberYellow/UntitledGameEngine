#include "Camera.h"

#include "Modules/GraphicsModule.h"

void Camera::SetPosition(Vec3f pos)
{
    m_Position = pos;
    m_ViewMatrixNeedsUpdate = true;
}

void Camera::SetDirection(Vec3f dir)
{
    dir = Math::normalize(dir);
    m_Direction = dir;
    m_ViewMatrixNeedsUpdate = true;
}

void Camera::Move(Vec3f move)
{
    m_Position += move;
    m_ViewMatrixNeedsUpdate = true;
}

void Camera::Rotate(Quaternion rot)
{
    m_Direction = m_Direction * rot;
    m_ViewMatrixNeedsUpdate = true;
}

Vec3f Camera::GetPosition() const
{
    return m_Position;
}

Vec3f Camera::GetDirection() const
{
    return m_Direction;
}

Vec3f Camera::GetUp() const
{
    return m_Up;
}

Vec3f Camera::GetPerpVector() const
{
    return Math::cross(m_Direction, m_Up);
}

void Camera::RotateCamBasedOnDeltaMouse(Vec2i deltaMouse, float radsPerScreenPixel)
{
    m_Direction = Math::rotate(m_Direction, -(float)(deltaMouse.x) * radsPerScreenPixel, m_Up);
    Vec3f perpVector = Math::cross(m_Direction, m_Up);
    m_Direction = Math::rotate(m_Direction, -(float)(deltaMouse.y) * radsPerScreenPixel, perpVector);

    //m_Up = -Math::cross(m_Direction, perpVector);

    m_ViewMatrixNeedsUpdate = true;
}

void Camera::SetFieldOfView(float fov)
{
    m_FieldOfView = fov;
    m_ProjectionMatrixNeedsUpdate = true;
}

float Camera::GetFieldOfView() const
{
    return m_FieldOfView;
}

void Camera::SetScreenSize(Vec2f screenSize)
{
    m_ScreenSize = screenSize;
    m_ProjectionMatrixNeedsUpdate = true;
}

void Camera::SetNearPlane(float nearPlane)
{
    m_NearClippingPlane = nearPlane;
    m_ProjectionMatrixNeedsUpdate = true;
}

void Camera::SetFarPlane(float farPlane)
{
    m_FarClippingPlane = farPlane;
    m_ProjectionMatrixNeedsUpdate = true;
}

Mat4x4f Camera::GetCamMatrix()
{
    Mat4x4f ProjMatrix = GetProjectionMatrix();
    Mat4x4f ViewMatrix = GetViewMatrix();

    if (m_ViewProjectionMatrixNeedsUpdate)
    {
        m_ViewProjectionMatrix = ProjMatrix * ViewMatrix;
        m_ViewProjectionMatrixNeedsUpdate = false;
    }
    return m_ViewProjectionMatrix;
}

Mat4x4f Camera::GetInvCamMatrix()
{
    return Math::inv(GetCamMatrix());
}

Mat4x4f Camera::GetCamTransMatrix()
{
    return Math::inv(GetViewMatrix());
}

//void Camera::SetCamTrans(Transform& InTrans)
//{
//    m_Position = InTrans.GetPosition();
//
//}

Mat4x4f Camera::GetViewMatrix()
{
    if (m_ViewMatrixNeedsUpdate)
    {
        UpdateViewMatrix();
        m_ViewMatrixNeedsUpdate = false;
    }
    return m_ViewMatrix;
}

Mat4x4f Camera::GetProjectionMatrix()
{
    if (m_ProjectionMatrixNeedsUpdate)
    {
        UpdateProjectionMatrix();
        m_ProjectionMatrixNeedsUpdate = false;
    }
    return m_ProjectionMatrix;
}

void Camera::DebugDrawCamFrustum()
{
    GraphicsModule* Graphics = GraphicsModule::Get();

    Mat4x4f InvCamMatrix = GetInvCamMatrix();

    Vec3f Corners[8];

    Corners[0] = Vec3f(-1.0f, -1.0f, -1.0f);
    Corners[1] = Vec3f(1.0f, -1.0f, -1.0f);
    Corners[2] = Vec3f(1.0f, 1.0f, -1.0f);
    Corners[3] = Vec3f(-1.0f, 1.0f, -1.0f);
    Corners[4] = Vec3f(-1.0f, -1.0f, 1.0f);
    Corners[5] = Vec3f(1.0f, -1.0f, 1.0f);
    Corners[6] = Vec3f(1.0f, 1.0f, 1.0f);
    Corners[7] = Vec3f(-1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 8; ++i)
    {
        Corners[i] = Math::testMult(Corners[i], InvCamMatrix);

        //Graphics->DebugDrawPoint(Corners[i]);
    }

    Graphics->DebugDrawLine(Corners[0], Corners[1], MakeColour(125, 255, 125));
    Graphics->DebugDrawLine(Corners[1], Corners[2], MakeColour(125, 255, 125));
    Graphics->DebugDrawLine(Corners[2], Corners[3], MakeColour(125, 255, 125));
    Graphics->DebugDrawLine(Corners[3], Corners[0], MakeColour(125, 255, 125));

    Graphics->DebugDrawLine(Corners[4], Corners[5], MakeColour(255, 125, 125));
    Graphics->DebugDrawLine(Corners[5], Corners[6], MakeColour(255, 125, 125));
    Graphics->DebugDrawLine(Corners[6], Corners[7], MakeColour(255, 125, 125));
    Graphics->DebugDrawLine(Corners[7], Corners[4], MakeColour(255, 125, 125));

    Graphics->DebugDrawLine(Corners[0], Corners[4]);
    Graphics->DebugDrawLine(Corners[1], Corners[5]);
    Graphics->DebugDrawLine(Corners[2], Corners[6]);
    Graphics->DebugDrawLine(Corners[3], Corners[7]);

}

void Camera::UpdateViewMatrix()
{
    m_ViewMatrix = Math::GenerateViewMatrix(m_Position, m_Direction, m_Up);

    m_ViewProjectionMatrixNeedsUpdate = true;
}

void Camera::UpdateProjectionMatrix()
{
    if (m_ProjectionType == Projection::Perspective)
    {
        m_ProjectionMatrix = Math::GenerateProjectionMatrix(m_FieldOfView, m_ScreenSize.x / m_ScreenSize.y, m_NearClippingPlane, m_FarClippingPlane);
    }
    else {
        m_ProjectionMatrix = Math::GenerateOrthoMatrix(-(m_ScreenSize.x / 2.0f), m_ScreenSize.x / 2.0f, -(m_ScreenSize.y / 2.0f), m_ScreenSize.y / 2.0f, m_NearClippingPlane, m_FarClippingPlane);
    }

    m_ViewProjectionMatrixNeedsUpdate = true;
}
