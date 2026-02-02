#ifndef SCENEWORLD_H
#define SCENEWORLD_H
#include "BaseWindow.h"
#include "scene/Scene.h"
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
    void handleInput();
    static std::bitset<GLFW_KEY_LAST + 1> pressed;
    static bool look_active;
    static bool mouse_initialized;
    static int mouse_x, mouse_y, prev_x, prev_y;
    static float yaw_deg, pitch_deg;
    static float look_sensitivity;
    
public:
    SceneWorld(int x, int y, int width, int height);
    SceneWorld(int x, int y, int width, int height, std::shared_ptr<Camera> playerCamera);
    ~SceneWorld();
    void Render() override;
    void Resize(int x, int y, int width, int height) override;
    static void KeyCallback(int action, int key);
    static void SetMousePosition(int x, int y);
    static void SetLookActive(bool active);

    void AddCamera(std::shared_ptr<Camera> playerCamera);
    void AddScene(std::shared_ptr<Scene> scene);
};
#endif
