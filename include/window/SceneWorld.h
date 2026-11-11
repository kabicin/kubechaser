#ifndef SCENEWORLD_H
#define SCENEWORLD_H
#include "BaseWindow.h"
#include "scene/Scene.h"
#include "scene/SceneManager.h"
#include "utility/Camera.h"
#include <unordered_map>
#include <vector>
#include <bitset>
#include <GLFW/glfw3.h>

class SceneWorld : public BaseWindow
{
private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
    std::shared_ptr<SceneManager> manager;
    void handleInput();
    std::bitset<GLFW_KEY_LAST + 1> pressed;
    bool look_active = false;
    bool mouse_initialized = false;
    int mouse_x = 0;
    int mouse_y = 0;
    int prev_x = 0;
    int prev_y = 0;
    float yaw_deg = -90.0f;
    float pitch_deg = 0.0f;
    float look_sensitivity = 0.12f;
    
public:
    SceneWorld(const WindowServices& services, int x, int y, int width, int height);
    SceneWorld(const WindowServices& services, int x, int y, int width, int height, std::shared_ptr<Camera> playerCamera);
    ~SceneWorld();
    void Render() override;
    void Resize(int x, int y, int width, int height) override;
    void KeyCallback(int action, int key);
    void SetMousePosition(int x, int y);
    void SetLookActive(bool active);

    void OnCursorPosition(double xpos, double ypos) override;
    void OnMouseButton(int button, int action, double xpos, double ypos) override;
    void OnKey(int key, int action) override;
    void OnDeactivate() override;

    void AddCamera(std::shared_ptr<Camera> playerCamera);
    void AddScene(std::shared_ptr<Scene> scene);
    void AddSceneManager(std::shared_ptr<SceneManager> manager);
    void SetShowGrid(bool enabled);
};
#endif
