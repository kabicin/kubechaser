#ifndef SCENEWORLD_H
#define SCENEWORLD_H
#include "BaseWindow.h"
#include "scene/Scene.h"
#include "utility/Camera.h"
#include <unordered_map>
#include <vector>

class SceneWorld : public BaseWindow
{
private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
    
public:
    SceneWorld(int x, int y, int width, int height);
    SceneWorld(int x, int y, int width, int height, std::shared_ptr<Camera> playerCamera);
    ~SceneWorld();
    void Render() override;
    void Resize(int x, int y, int width, int height) override;

    void AddCamera(std::shared_ptr<Camera> playerCamera);
    void AddScene(std::shared_ptr<Scene> scene);
};
#endif
