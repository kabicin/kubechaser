#ifndef SCENEDOCUMENT_H
#define SCENEDOCUMENT_H

#include "glm.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Entity;

class SceneDocument
{
public:
    std::shared_ptr<Entity> AddEntity(
        std::shared_ptr<Entity> entity,
        glm::vec3 offset = glm::vec3(0.0f));
    std::shared_ptr<Entity> AddEntity(
        const std::string& className,
        glm::vec3 offset = glm::vec3(0.0f));
    std::shared_ptr<Entity> AddEntity(
        const std::string& className,
        const std::string& objectFile,
        glm::vec3 offset = glm::vec3(0.0f));
    std::shared_ptr<Entity> AddEntity(
        const std::string& className,
        const std::string& objectFile,
        const std::string& textureFile,
        glm::vec3 offset = glm::vec3(0.0f));

    const std::vector<std::shared_ptr<Entity>>& GetEntities() const;
    std::size_t GetNumEntities() const;
    void SetActiveEntity(int index);
    std::shared_ptr<Entity> GetActiveEntity() const;
    void DeleteEntity(int index);
    void DeleteEntity(const std::shared_ptr<Entity>& entity);
    void Clear();

    double GetMaxDepth() const;
    double GetMinDepth() const;

private:
    std::vector<std::shared_ptr<Entity>> entities;
    std::optional<std::size_t> activeEntityIndex;
};

#endif
