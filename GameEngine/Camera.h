#pragma once

#include "Math\Math.h"
#include "Math\Quaternion.h"

enum class Projection
{
    Perspective,
    Orthographic
};

class Camera
{
public:
    Camera(Projection projType = Projection::Perspective)
        : m_ViewProjectionMatrixNeedsUpdate(true)
        , m_ViewMatrixNeedsUpdate(true)
        , m_ProjectionMatrixNeedsUpdate(true)
        , m_FieldOfView(Deg2Rad(80.0f))
        , m_NearClippingPlane(0.01f)
        , m_FarClippingPlane(5000.0f)
        , m_ProjectionType(projType)
    {
    }

    void SetPosition(Vec3f pos);
    void SetDirection(Vec3f dir);

    void Move(Vec3f move);
    void Rotate(Quaternion rot);

    Vec3f GetPosition() const;
    Vec3f GetDirection() const;
    Vec3f GetPerpVector() const;

    void RotateCamBasedOnDeltaMouse(Vec2i deltaMouse, float radsPerScreenPixel);

    void SetFieldOfView(float fov);
    float GetFieldOfView() const;

    void SetScreenSize(Vec2f screenSize);
    
    void SetNearPlane(float nearPlane);
    void SetFarPlane(float farPlane);

    Mat4x4f GetCamMatrix();
    Mat4x4f GetInvCamMatrix();
    
    Mat4x4f GetViewMatrix();
    Mat4x4f GetProjectionMatrix();

private:
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

    Vec3f m_Position = Vec3f(0.0f, 0.0f, 0.0f);
    Vec3f m_Direction = Vec3f(0.0f, 1.0f, 0.0f);
    Vec3f m_Up = Vec3f(0.0f, 0.0f, 1.0f);

    float m_FieldOfView;
    Vec2f m_ScreenSize;
    float m_NearClippingPlane;
    float m_FarClippingPlane;

    Mat4x4f m_ViewProjectionMatrix;
    Mat4x4f m_ViewMatrix;
    Mat4x4f m_ProjectionMatrix;

    bool m_ViewProjectionMatrixNeedsUpdate;
    bool m_ViewMatrixNeedsUpdate;
    bool m_ProjectionMatrixNeedsUpdate;

    Projection m_ProjectionType;
};