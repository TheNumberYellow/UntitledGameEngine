#include "Camera.h"

void Camera::SetPosition(Vec3f pos)
{
	m_Position = pos;
}

void Camera::SetRotation(Quaternion rot)
{
}

void Camera::SetDirection(Vec3f dir)
{
	dir = Math::normalize(dir);
	m_Direction = dir;
}

Mat4x4f Camera::GetCamMatrix()
{
	return Mat4x4f();
}
