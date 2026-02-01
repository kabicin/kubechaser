#include "window/SceneWorld.h"

SceneWorld::SceneWorld(int x, int y, int width, int height)
    : BaseWindow(x, y, width, height)
{
}

SceneWorld::SceneWorld(int x, int y, int width, int height, std::shared_ptr<Camera> playerCamera)
    : BaseWindow(x, y, width, height)
{
    AddCamera(playerCamera);
}

SceneWorld::~SceneWorld()
{
}

void SceneWorld::Resize(int x, int y, int width, int height)
{
    // grab new dimensions to update camera..
    if (camera != nullptr) camera->Resize(width, height);
    BaseWindow::Resize(x, y, width, height);
}

void SceneWorld::Render()
{
    scene->Render(GetShaderFactory(), camera);
}

void SceneWorld::AddCamera(std::shared_ptr<Camera> playerCamera)
{
    camera = playerCamera;
}

void SceneWorld::AddScene(std::shared_ptr<Scene> scene)
{
    this->scene = scene;
}