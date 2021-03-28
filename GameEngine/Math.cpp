#include "Math.hpp"

#include <glm/gtx/rotate_vector.hpp>

float Math::dot(Vec3f leftVec, Vec3f rightVec)
{
	float result = (leftVec.x * rightVec.x) + (leftVec.y * rightVec.y) + (leftVec.z * rightVec.z);
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
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

Vec3f Math::mult(Vec3f vec, Mat4x4f mat)
{
	glm::vec4 glmVec = glm::vec4(vec.x, vec.y, vec.z, 1.0f);
	glm::mat4 glmMat;
	
	glmMat[0] = glm::vec4(mat[0].x, mat[0].y, mat[0].z, mat[0].w);
	glmMat[1] = glm::vec4(mat[1].x, mat[1].y, mat[1].z, mat[1].w);
	glmMat[2] = glm::vec4(mat[2].x, mat[2].y, mat[2].z, mat[2].w);
	glmMat[3] = glm::vec4(mat[3].x, mat[3].y, mat[3].z, mat[3].w);

	glmVec = glmVec * glmMat;

	return Vec3f(glmVec.x, glmVec.y, glmVec.z);
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

Mat4x4f Math::GenerateViewMatrix(Vec3f position, Vec3f direction, Vec3f up)
{
	Vec3f f(normalize(-direction));
	Vec3f s(normalize(cross(up, f)));
	Vec3f u(cross(f, s));

	Mat4x4f result;
	result[0][0] = s.x;
	result[1][0] = s.y;
	result[2][0] = s.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = f.x;
	result[1][2] = f.y;
	result[2][2] = f.z;
	result[3][0] = -dot(s, position);
	result[3][1] = -dot(u, position);
	result[3][2] = -dot(f, position);

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

float RandomFloat(float min, float max)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

Mat4x4f Mat4x4f::operator*(Mat4x4f rhs)
{
	glm::mat4 glmMat;

	glmMat[0] = glm::vec4((*this)[0].x, (*this)[0].y, (*this)[0].z, (*this)[0].w);
	glmMat[1] = glm::vec4((*this)[1].x, (*this)[1].y, (*this)[1].z, (*this)[1].w);
	glmMat[2] = glm::vec4((*this)[2].x, (*this)[2].y, (*this)[2].z, (*this)[2].w);
	glmMat[3] = glm::vec4((*this)[3].x, (*this)[3].y, (*this)[3].z, (*this)[3].w);

	glm::mat4 rhsMat;
	
	rhsMat[0] = glm::vec4(rhs[0].x, rhs[0].y, rhs[0].z, rhs[0].w);
	rhsMat[1] = glm::vec4(rhs[1].x, rhs[1].y, rhs[1].z, rhs[1].w);
	rhsMat[2] = glm::vec4(rhs[2].x, rhs[2].y, rhs[2].z, rhs[2].w);
	rhsMat[3] = glm::vec4(rhs[3].x, rhs[3].y, rhs[3].z, rhs[3].w);

	glm::mat4 glmResult = glmMat * rhsMat;

	Mat4x4f result;

	result[0] = Vec4f(glmResult[0].x, glmResult[0].y, glmResult[0].z, glmResult[0].w);
	result[1] = Vec4f(glmResult[1].x, glmResult[1].y, glmResult[1].z, glmResult[1].w);
	result[2] = Vec4f(glmResult[2].x, glmResult[2].y, glmResult[2].z, glmResult[2].w);
	result[3] = Vec4f(glmResult[3].x, glmResult[3].y, glmResult[3].z, glmResult[3].w);

	return result;
}
