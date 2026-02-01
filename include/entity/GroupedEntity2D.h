#ifndef GROUPEDENTITY2D_H
#define GROUPEDENTITY2D_H
#include "Entity.h"
#include <vector>
#include <memory>

class GroupedEntity2D : public Entity
{
private:
	std::vector<std::shared_ptr<Entity>> groupedEntities;

public:
	GroupedEntity2D(std::vector<std::shared_ptr<Entity>> newEntities);
	~GroupedEntity2D();
	void GroupEntity(std::shared_ptr<Entity> entity);
	void Draw(const std::shared_ptr<ShaderFactory>& factory,const std::shared_ptr<Camera>& camera) override;
	void Translate(glm::vec3 offset) override;
	void Scale(double scaleFactor) override;
	void SetObjectPicked() override;
	bool Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t) override;
};
#endif
