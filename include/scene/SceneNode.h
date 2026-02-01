#ifndef SCENENODE_H
#define SCENENODE_H
#include "entity/Entity.h"
#include "entity/Grid.h"
#include "utility/Camera.h"
#include "scene/SceneDirection.h"
#include <array>


class SceneNode 
{
private:
    std::vector<std::shared_ptr<Entity>> staticObjects;
    std::vector<std::shared_ptr<Entity>> dynamicObjects;

    // row major layout of Scenes surrounding the current scene
    // scenes are along the xz-plane
    //          +x
    //     - - - - - > 
    //    | [NW] [N] [NE]
    // +z | [W]  [X] [E]  <-- X is location of the current scene, NORTH is UP  ^^^^
    //    v [SW] [S] [SE]
    //
    // indexable by SceneDirection enum
    std::array<std::shared_ptr<SceneNode>, 8> boundary;

public:
    SceneNode();
    ~SceneNode();
    void AddStaticObject(std::shared_ptr<Entity> entity);
    void AddDynamicObject(std::shared_ptr<Entity> entity);
    void Render(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera);
    std::array<std::shared_ptr<SceneNode>, 8> GetBoundary();
    void SetBoundary(const std::array<std::shared_ptr<SceneNode>, 8>&  boundary);
};
#endif