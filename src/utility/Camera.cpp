#include "utility/Camera.h"
#include <iostream>

Camera::Camera()
{
}

Camera::Camera(int width, int height)
	:m_width(width), m_height(height)
{
	projection = glm::perspective(glm::radians(45.0f), m_width / static_cast<float>(m_height), this->n, this->f);
	view = glm::lookAt(GetCameraPos(), cameraFront, cameraUp);
}

void Camera::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (m_height > 0) {
		projection = glm::perspective(glm::radians(45.0f), m_width / static_cast<float>(m_height), this->n, this->f);
	}
}

glm::mat4 Camera::GetProjection()
{
	return projection;
}

glm::mat4 Camera::GetView()
{
	return view;
}

glm::mat4 Camera::GetProjectionInverse()
{
	return glm::inverse(projection);
}

glm::mat4 Camera::GetViewInverse()
{
	return glm::inverse(view);
}

glm::mat4 Camera::GenerateModel(const CameraAttributes& ca)
{
	// apply local space translations
	model = glm::rotate(glm::rotate(
		glm::translate(glm::scale(glm::mat4(1.0f), ca.scale), ca.translate), 
		glm::radians(ca.rotateXdeg), glm::vec3(1,0,0)), glm::radians(ca.rotateYdeg), glm::vec3(0,1,0));
	// apply entire model translations
	glm::vec3 scaledTranslate = glm::vec3(cma.translate.x / ca.scale.x, cma.translate.y / ca.scale.y, cma.translate.z / ca.scale.z);
	model = glm::translate(model, scaledTranslate);
	return model;
}

glm::mat4 Camera::GetModel()
{
	return model;
}

Ray Camera::GetRay(double xPos, double yPos)
{
	Ray ray(*cameraPos, Camera::GetDirection(xPos, yPos));
	return ray;
}

void Camera::Render(int program, const CameraAttributes& ca)
{
	int modelLoc = glGetUniformLocation(program, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(GenerateModel(ca)));

	int viewLoc = glGetUniformLocation(program, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	int projectionLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

glm::vec3 Camera::GetLookDirection()
{
	return cameraFront - *cameraPos;
}

glm::vec3 Camera::GetRawDirection(double xPos, double yPos)
{
	glm::vec3 rayNds = glm::vec3((2.0 * xPos) / m_width - 1.0, 1.0 - (2.0 * yPos) / m_height, 1.0);
	glm::vec4 rayClip = glm::vec4(rayNds.x, rayNds.y, rayNds.z, 1.0);
	glm::vec4 rayEye = GetProjectionInverse() * rayClip;
	glm::vec4 rayWorldInt = GetViewInverse() * rayEye;
	return glm::vec3(rayWorldInt.x, rayWorldInt.y, rayWorldInt.z);
}

glm::vec3 Camera::GetDirection(double xPos, double yPos)
{
	return glm::normalize(GetRawDirection(xPos, yPos));
}

std::pair<int, int> Camera::GetInverseDirection(glm::vec3 rayWorldInt)
{
	glm::vec4 rayClip = GetProjection() * GetView() * glm::vec4(rayWorldInt.x, rayWorldInt.y, rayWorldInt.z, 1.0);
	return std::make_pair((rayClip.x + 1.0) * m_width / 2.0, -(m_height / 2.0) * (rayClip.y - 1.0));
}

// converts screen coordinate [screenWidth, screenHeight] to NDC [-1, 1]^3
glm::vec3 Camera::getClipVector(int x, int y)
{
	return glm::vec3((2.0 * x) / m_width - 1.0, 1.0 - (2.0 * y) / m_height, 0.0);
}

glm::vec3 Camera::GetCameraPos()
{
	return *cameraPos;
}

void Camera::SetForward()
{
	cameraDirection -= cameraFront;
	cameraDirection = glm::normalize(cameraDirection);
}

void Camera::SetBackward()
{
	cameraDirection += cameraFront;
	cameraDirection = glm::normalize(cameraDirection);
}

void Camera::SetRight()
{
	cameraDirection -= glm::normalize(glm::cross(cameraFront, cameraUp));
	cameraDirection = glm::normalize(cameraDirection);
}

void Camera::SetLeft()
{
	cameraDirection += glm::normalize(glm::cross(cameraFront, cameraUp));
	cameraDirection = glm::normalize(cameraDirection);
}

void Camera::SetUp()
{
	cameraDirection += cameraUp;
	cameraDirection = glm::normalize(cameraDirection);
}

void Camera::SetDown()
{
	cameraDirection -= cameraUp;
	cameraDirection = glm::normalize(cameraDirection);
}

void Camera::SetCenter()
{
	cameraDirection = glm::vec3(0, 0, 0);
}

void Camera::GenerateView()
{ 
	view = glm::lookAt(*cameraPos, *cameraPos - cameraFront, cameraUp);
}

void Camera::UpdateFrame(double multiplier)
{
	*cameraPos += cameraDirection * (float)multiplier;
	GenerateView();
}

void Camera::ResetCamPos()
{
	*cameraPos = initialCameraPos;
	cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	GenerateView();
}

void Camera::SetLookDirection(const glm::vec3& direction)
{
	if (glm::length(direction) <= 0.0f) {
		return;
	}
	cameraFront = -glm::normalize(direction);
	GenerateView();
}
