#ifndef SCENECAMERA_H
#define SCENECAMERA_H
#include <glad/glad.h>
#include "glm.h"
#include "Linalg.h"
#include <memory>
#include <utility>

struct CameraAttributes
{
	CameraAttributes() {}
	CameraAttributes(glm::vec3 translate, glm::vec3 scale, float rotateX, float rotateY, float rotateZ)
		: translate(translate), scale(scale), rotateXdeg(rotateX), rotateYdeg(rotateY), rotateZdeg(rotateZ) {}
	CameraAttributes(const CameraAttributes& attribs)
		: translate(attribs.translate), scale(attribs.scale), rotateXdeg(attribs.rotateXdeg), rotateYdeg(attribs.rotateYdeg), rotateZdeg(attribs.rotateZdeg) {}
	
	// local space translations
	glm::vec3 translate;
	glm::vec3 scale;
	float rotateXdeg;
	float rotateYdeg;
	float rotateZdeg;
};

struct CameraModelAttributes {
	CameraModelAttributes() {
		translate = glm::vec3(0, 0, 0);
	}
	glm::vec3 translate; // translates entire model from origin
};

class Camera
{
private:
	int m_width, m_height;

	const float n = 0.1;
	const float f = 100;
	const float cameraSpeed = 0.1;

	const glm::vec3 initialCameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	std::shared_ptr<glm::vec3> cameraPos = std::make_shared<glm::vec3>(initialCameraPos);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 getClipVector(int x, int y);

public:
	const float MAX_SCALE = 200.0f;
	Camera();
	Camera(int width, int height);

	glm::vec3 translateOffset = glm::vec3(0.0f, 0.0f, -1.0f);
	void Render(int program, const CameraAttributes& ca);
	void Resize(int width, int height);

	glm::mat4 GetProjection();
	glm::mat4 GetView();

	glm::mat4 GetProjectionInverse();
	glm::mat4 GetViewInverse();

	CameraModelAttributes cma = CameraModelAttributes(); // different from entity's ca. model attributes transforms the entire model itself.
	glm::mat4 GenerateModel(const CameraAttributes& ca);
	glm::mat4 GetModel();

	glm::vec3 GetLookDirection();
	Ray GetRay(double xPos, double yPos);
	glm::vec3 GetDirection(double xPos, double yPos);
	glm::vec3 GetRawDirection(double xPos, double yPos);
	std::pair<int, int> GetInverseDirection(glm::vec3 rayWorldInt);

	void SetForward();
	void SetBackward();
	void SetRight();
	void SetLeft();
	void SetUp();
	void SetDown();
	void SetCenter();

	void UpdateFrame(double multiplier = 1);
	void GenerateView();
	void ResetCamPos();
	void SetLookDirection(const glm::vec3& direction);

	glm::vec3 GetCameraPos();

};
#endif
