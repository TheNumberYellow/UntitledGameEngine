#include "Math.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

float Math::dot(Vec3f leftVec, Vec3f rightVec)
{
	float glmDot = glm::dot(glm::vec3(leftVec.x, leftVec.y, leftVec.z), glm::vec3(rightVec.x, rightVec.y, rightVec.z));
	return glmDot;
}

Vec3f Math::cross(Vec3f leftVec, Vec3f rightVec)
{
	glm::vec3 glmVec = glm::cross(glm::vec3(leftVec.x, leftVec.y, leftVec.z), glm::vec3(rightVec.x, rightVec.y, rightVec.z));
	return Vec3f(glmVec.x, glmVec.y, glmVec.z);
}

Vec3f Math::rotate(Vec3f inputVec, float radians, Vec3f axis)
{
	glm::vec3 glmVec = glm::vec3(inputVec.x, inputVec.y, inputVec.z);
	glmVec = glm::rotate(glmVec, radians, glm::vec3(axis.x, axis.y, axis.z));
	return Vec3f(glmVec.x, glmVec.y, glmVec.z);
}

Vec3f Math::normalize(Vec3f vec)
{
	glm::vec3 glmVec = glm::vec3(vec.x, vec.y, vec.z);
	glmVec = glm::normalize(glmVec);

	return Vec3f(glmVec.x, glmVec.y, glmVec.z);
}

float Math::clamp(float input, float min, float max)
{
	return input < min ? min : (input > max ? max : input);
}

float RandomFloat(float min, float max)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}
