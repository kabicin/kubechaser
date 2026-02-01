#include "scene/Scene.h"

Scene::Scene() 
{
}

Scene::Scene(const std::shared_ptr<SceneNode>& root)
{
    this->root = root;
}

Scene::~Scene()
{
}

void Scene::Render(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera)
{
    // render scene
    std::shared_ptr<SceneNode> node = root;
    if (node == nullptr) {
        std::cout << "Scene root is not initialized..." << std::endl;
        return;
    }
    // render grid first under root scenenode
    bool gridEnabled = showGrid && grid != nullptr;
    if (gridEnabled) grid->Draw(factory, camera, true);
    node->Render(factory, camera);
    std::array<std::shared_ptr<SceneNode>, 8> boundary = root->GetBoundary();

    glm::vec3 oldTranslate = camera->cma.translate;
    for (int i = 0; i < 8; i++)
    {   
        node = boundary[i];
        // translate the entire scene respective to the current scene
        if (node != nullptr) {
            camera->cma.translate = getModelTranslate(i) * 10.0f; 
            // render grid before rendering new node if enabled
            if (gridEnabled) grid->Draw(factory, camera);
            node->Render(factory, camera);
        }
    }
    camera->cma.translate = oldTranslate;
}

glm::vec3 Scene::getModelTranslate(int direction) {
    glm::vec3 translate = glm::vec3(0, 0, 0);
    switch (static_cast<SceneDirection>(direction)) {
        case SceneDirection::NORTHWEST: 
            translate.x = -1;
            translate.z = -1;
            break;
        case SceneDirection::NORTH:
            translate.x = 0;
            translate.z = -1;
            break;
        case SceneDirection::NORTHEAST:
            translate.x = 1;
            translate.z = -1;
            break;
        case SceneDirection::WEST:
            translate.x = -1;
            translate.z = 0;
            break;
        case SceneDirection::EAST:
            translate.x = 1;
            translate.z = 0;
            break;
        case SceneDirection::SOUTHWEST:
            translate.x = -1;
            translate.z = 1;
            break;
        case SceneDirection::SOUTH:
            translate.x = 0;
            translate.z = 1;
            break;
        case SceneDirection::SOUTHEAST:
            translate.x = 1;
            translate.z = 1;
            break;
    }
    return translate;
}

void Scene::SetShowGrid(bool cond, const std::shared_ptr<Camera>& camera)
{
    showGrid = cond;
    if (showGrid && grid == nullptr) {
        grid = std::make_shared<Grid>(camera);
    }
}