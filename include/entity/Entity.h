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
#include <set>
#include "shader/DrawAttributes.h"

class Entity
{
private:
	int _id;
	// IDs held by live entities; a new entity takes the smallest unused ID
	static std::set<int> usedIds;
	static int acquireId();
	static void releaseId(int id);

	// camera
	static std::shared_ptr<Camera> camera;
	CameraAttributes ca = CameraAttributes(glm::vec3(0,0,0), glm::vec3(1,1,1), 0, 0, 0);

	// true when TEX was created by AddTexture (and must be deleted by this entity);
	// false when TEX is borrowed from an Asset via SetTexture
	bool ownsTexture = false;
	void releaseTexture();

public:
	GLuint VBO = 0, VAO = 0, EBO = 0, TEX = 0;
	std::string textureName = "N/A";
	bool usingTexture = false;

	DrawAttributes da = DrawAttributes();

	Entity();
	virtual ~Entity();

	// owns GPU handles: non-copyable
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;
	int GetId();
	
	// subclasses must override
	virtual void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) = 0;
	virtual void SetObjectPicked() = 0;
	virtual bool Intersect(const std::shared_ptr<Camera>& camera, const Ray &ray, double &t) = 0;

	// Helpers for SceneControl
	virtual std::string GetName();
	virtual GLuint GetTexture();
	virtual std::string GetTextureName();
	// Loads imagePath into a new owned texture. Returns false (and leaves the
	// current texture untouched) if the image could not be loaded.
	virtual bool AddTexture(const std::string& imagePath);
	virtual void SetTexture(Asset asset);


	// object changing
	float ndcRatio = 1.0;
	virtual void Translate(glm::vec3 trans);
	virtual void SetTranslate(glm::vec3 trans);
	glm::vec3 GetTranslate() const;
	virtual void MoveUp(glm::vec3 trans);
	virtual void MoveBack(glm::vec3 trans);
	// Scale multiplies the current scale; SetScale replaces it
	virtual void Scale(double scaleFactor);
	virtual void Scale(glm::vec3 givenScaleVector);
	virtual void SetScale(glm::vec3 newScale);
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

