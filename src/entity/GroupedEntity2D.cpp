#include "entity/GroupedEntity2D.h"

GroupedEntity2D::GroupedEntity2D(std::vector<std::shared_ptr<Entity>> newEntities)
{
	groupedEntities = newEntities;
}

GroupedEntity2D::~GroupedEntity2D()
{
	groupedEntities.clear();
}

void GroupedEntity2D::GroupEntity(std::shared_ptr<Entity> entity)
{
	groupedEntities.push_back(std::move(entity));
}

void GroupedEntity2D::Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
{
	for (size_t i = 0; i < groupedEntities.size(); i++)
	{
		groupedEntities[i]->Draw(factory, camera);
	}
}

void GroupedEntity2D::Translate(glm::vec3 offset)
{
	for (size_t i = 0; i < groupedEntities.size(); i++)
	{
		groupedEntities[i]->Translate(offset);
	}
}

void GroupedEntity2D::Scale(double scaleFactor)
{
	for (size_t i = 0; i < groupedEntities.size(); i++)
	{
		groupedEntities[i]->Scale(scaleFactor);
	}
}

void GroupedEntity2D::SetObjectPicked()
{
	for (int i = 0; i < groupedEntities.size(); i++)
	{
		groupedEntities[i]->SetObjectPicked();
	}
}

bool GroupedEntity2D::Intersect(const std::shared_ptr<Camera>& camera, const Ray& ray, double& t)
{
	for (int i = 0; i < groupedEntities.size(); i++)
	{
		if (groupedEntities[i]->Intersect(camera, ray, t))
			return true;
	}
	return false;
}
