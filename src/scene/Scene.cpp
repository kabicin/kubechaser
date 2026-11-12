#include "scene/Scene.h"
#include "logger/Logger.h"

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
        Log::Warning("Scene root is not initialized...");
        return;
    }
    // render grid first under root scenenode
    bool gridEnabled = showGrid && grid != nullptr;
    if (gridEnabled) grid->Draw(factory, camera, true);
    node->Render(factory, camera);
}

void Scene::SetShowGrid(bool cond, const std::shared_ptr<Camera>& camera)
{
    showGrid = cond;
    if (showGrid && grid == nullptr) {
        grid = std::make_shared<Grid>(camera);
    }
}
