#ifndef SCENE_H
#define SCENE_H
#include "entity/Entity.h"
#include "utility/Camera.h"
#include <vector>
#include "scene/SceneNode.h"

class Scene
{
private:
    std::shared_ptr<SceneNode> root;
    bool showGrid = false;
    std::shared_ptr<Grid> grid;

public:
    Scene();
    Scene(const std::shared_ptr<SceneNode>& root);
    ~Scene();
    void Render(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera);
    void SetShowGrid(bool cond, const std::shared_ptr<Camera>& camera);
};
#endif
