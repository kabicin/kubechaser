#include "scene/SceneDocument.h"

#include "entity/Entity.h"
#include "entity/Mesh.h"
#include "entity/Quad.h"
#include "entity/Triangle.h"

#include <algorithm>
#include <iterator>

std::shared_ptr<Entity> SceneDocument::AddEntity(
    std::shared_ptr<Entity> entity,
    glm::vec3 offset)
{
    if (!entity) {
        return nullptr;
    }

    entity->Translate(offset);
    entities.push_back(entity);
    activeEntityIndex = entities.size() - 1;
    return entity;
}

std::shared_ptr<Entity> SceneDocument::AddEntity(
    const std::string& className,
    glm::vec3 offset)
{
    if (className == "Triangle") {
        return AddEntity(std::make_shared<Triangle>(), offset);
    }
    if (className == "Quad") {
        return AddEntity(std::make_shared<Quad>(), offset);
    }
    return nullptr;
}

std::shared_ptr<Entity> SceneDocument::AddEntity(
    const std::string& className,
    const std::string& objectFile,
    glm::vec3 offset)
{
    if (className != "Mesh") {
        return nullptr;
    }
    return AddEntity(std::make_shared<Mesh>(objectFile), offset);
}

std::shared_ptr<Entity> SceneDocument::AddEntity(
    const std::string& className,
    const std::string& objectFile,
    const std::string& textureFile,
    glm::vec3 offset)
{
    if (className != "Mesh") {
        return nullptr;
    }
    return AddEntity(std::make_shared<Mesh>(objectFile, "", textureFile), offset);
}

const std::vector<std::shared_ptr<Entity>>& SceneDocument::GetEntities() const
{
    return entities;
}

std::size_t SceneDocument::GetNumEntities() const
{
    return entities.size();
}

void SceneDocument::SetActiveEntity(int index)
{
    if (index >= 0 && static_cast<std::size_t>(index) < entities.size()) {
        activeEntityIndex = static_cast<std::size_t>(index);
    } else {
        activeEntityIndex.reset();
    }
}

std::shared_ptr<Entity> SceneDocument::GetActiveEntity() const
{
    if (!activeEntityIndex || *activeEntityIndex >= entities.size()) {
        return nullptr;
    }
    return entities[*activeEntityIndex];
}

void SceneDocument::DeleteEntity(int index)
{
    if (index < 0 || static_cast<std::size_t>(index) >= entities.size()) {
        return;
    }

    entities.erase(entities.begin() + index);
    if (entities.empty()) {
        activeEntityIndex.reset();
    } else {
        activeEntityIndex = entities.size() - 1;
    }
}

void SceneDocument::DeleteEntity(const std::shared_ptr<Entity>& entity)
{
    const auto it = std::find(entities.begin(), entities.end(), entity);
    if (it == entities.end()) {
        return;
    }
    DeleteEntity(static_cast<int>(std::distance(entities.begin(), it)));
}

void SceneDocument::Clear()
{
    entities.clear();
    activeEntityIndex.reset();
}

double SceneDocument::GetMaxDepth() const
{
    double maxDepth = 0.0;
    for (const auto& entity : entities) {
        if (entity->translate.z >= maxDepth) {
            maxDepth = entity->translate.z + 0.01;
        }
    }
    return maxDepth;
}

double SceneDocument::GetMinDepth() const
{
    double minDepth = 0.0;
    for (const auto& entity : entities) {
        if (entity->translate.z <= minDepth) {
            minDepth = entity->translate.z - 0.01;
        }
    }
    return minDepth;
}
