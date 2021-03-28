#pragma once

#include "Math.hpp"

class Camera
{
public:
	void SetPosition(Vec3f pos);
	void SetRotation(Quaternion rot);
	void SetDirection(Vec3f dir);

	Mat4x4f GetCamMatrix();
	Mat4x4f GetViewMatrix();
	Mat4x4f GetProjectionMatrix();

private:
	Vec3f m_Position = Vec3f(0.0f, 0.0f, 0.0f);
	Vec3f m_Direction = Vec3f(0.0f, 1.0f, 0.0f);
	Vec3f m_Up = Vec3f(0.0f, 0.0f, 1.0f);

	Mat4x4f m_ViewMatrix;
	Mat4x4f m_ProjectionMatrix;

	bool m_ViewMatrixNeedsUpdate;
	bool m_ProjectionMatrixNeedsUpdate;
};

