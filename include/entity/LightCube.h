#ifndef LIGHTCUBE_H
#define LIGHTCUBE_H
#include "Entity.h"

class LightCube : public Entity
{
private:
	void initialize();
	// ray casting matrices, vectors
	glm::mat4 model;
	glm::vec4 v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20, v21, v22, v23, v24, v25, v26, v27, v28, v29, v30, v31, v32, v33, v34, v35;
	Eigen::Vector3d pivot, t1, t2;
public:
	LightCube();
	LightCube(glm::vec3 offset);
	~LightCube();
	void Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera) override;
	void SetObjectPicked() override;
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t) override;

};
#endif
