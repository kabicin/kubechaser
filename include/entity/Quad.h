#ifndef QUAD_H
#define QUAD_H
#include "Entity.h"
#include <algorithm>
#include <vector>

class Quad : public Entity
{
private:
	void initialize();

	// ray casting matrices, vectors
	glm::mat4 model;
	glm::vec4 v0, v1, v2, v3;
	Eigen::Vector3d pivot, t1, t2;

	GLuint instanceVBO = 0; // per-instance offsets for DrawInstanced
public:
	Quad();
	Quad(glm::vec3 offset);
	~Quad();
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) override;
	// Draws one copy of the quad per offset in a single call. Offsets are in the
	// quad's local (pre-scale, pre-rotation) space and added to each vertex.
	// Tessellation is not used for instanced draws.
	void DrawInstanced(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera, const std::vector<glm::vec3>& offsets);
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t) override;
};
#endif
