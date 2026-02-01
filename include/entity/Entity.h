#ifndef ENTITY_H
#define ENTITY_H
#include <glad/glad.h>
#include "glm.h"
#include "stb_image.h"
#include "Vertex.h"
#include "window/Asset.h"
#include "shader/Shader.h"
#include "shader/ShaderFactory.h"
#include "utility/Linalg.h"
#include "utility/Camera.h"
#include <string>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <bitset>
#include "shader/DrawAttributes.h"
#define MAX_ENTITIES 100

class Entity
{
private:
	int _id;
	static std::bitset<MAX_ENTITIES> entityBitmap;

	// bitmap functions
	static int getFreeBit();
	static void setBit(int bit);
	static void clearBit(int bit);

	// camera
	static std::shared_ptr<Camera> camera;
	CameraAttributes ca = CameraAttributes(glm::vec3(0,0,0), glm::vec3(1,1,1), 0, 0, 0);

public:
	GLuint VBO, VAO, EBO, TEX;
	std::string textureName = "N/A";
	bool usingTexture = false;

	DrawAttributes da = DrawAttributes();

	Entity();
	virtual ~Entity();
	int GetId();
	
	// subclasses must override
	virtual void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) = 0;
	virtual void SetObjectPicked() = 0;
	virtual bool Intersect(const std::shared_ptr<Camera>& camera, const Ray &ray, double &t) = 0;

	// Helpers for SceneControl
	virtual std::string GetName();
	virtual GLuint GetTexture();
	virtual std::string GetTextureName();
	virtual void AddTexture(const std::string& imagePath);
	virtual void SetTexture(Asset asset);


	// object changing
	float ndcRatio = 1.0;
	glm::vec3 translate = glm::vec3(0.0f, 0.0f, 0.0f);
	virtual void Translate(glm::vec3 trans);
	virtual void SetTranslate(glm::vec3 trans);
	virtual void MoveUp(glm::vec3 trans);
	virtual void MoveBack(glm::vec3 trans);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	virtual void Scale(double scale);
	virtual void Scale(glm::vec3 givenScaleVector);
	void RotateX(float degrees);
	void RotateY(float degrees);
	void RotateZ(float degrees);
	void RotateXInc(float degrees);
	void RotateYInc(float degrees);
	void RotateZInc(float degrees);

	// camera
	CameraAttributes GetCameraAttributes();
};
#endif

