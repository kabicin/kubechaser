#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Entity.h"
#include "utility/Linalg.h"

class Triangle : public Entity
{
private:
	void initialize();
	glm::vec4 v0, v1, v2;

public:
	Triangle();
	Triangle(glm::vec3 offset);
	~Triangle();

	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) override;
	void SetObjectPicked() override;
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double &t) override;	
};
#endif
