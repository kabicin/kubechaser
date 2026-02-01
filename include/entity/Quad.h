#ifndef QUAD_H
#define QUAD_H
#include "Entity.h"
#include <algorithm>

class Quad : public Entity
{
private:
	void initialize();

	// ray casting matrices, vectors
	glm::mat4 model;
	glm::vec4 v0, v1, v2, v3;
	Eigen::Vector3d pivot, t1, t2;
public:
	Quad();
	Quad(glm::vec3 offset);
	~Quad();
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) override;
	void SetObjectPicked() override;
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t) override;
};
#endif
