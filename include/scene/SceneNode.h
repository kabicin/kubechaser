#ifndef SCENENODE_H
#define SCENENODE_H
#include "entity/Entity.h"
#include "entity/Grid.h"
#include "utility/Camera.h"


class SceneNode 
{
private:
    std::vector<std::shared_ptr<Entity>> staticObjects;
    std::vector<std::shared_ptr<Entity>> dynamicObjects;

public:
    SceneNode();
    ~SceneNode();
    void AddStaticObject(std::shared_ptr<Entity> entity);
    void AddDynamicObject(std::shared_ptr<Entity> entity);
    void Render(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera);
};
#endif
