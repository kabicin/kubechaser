#ifndef APPLICATION_H
#define APPLICATION_H

#include "glm.h"

#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

class BaseFrame;
class Logger;
class ShaderFactory;
class Time;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run();

private:
    struct ShowcaseItem
    {
        std::string filename;
        std::string title;
        glm::vec3 color;
    };

    bool Initialize();
    bool InitializeImGui();
    void InitializeServices();
    void InitializeWindows();
    void InstallCallbacks();
    void RenderFrame();
    void RenderShowcase();
    void Shutdown();

    void OnCursorPosition(double xpos, double ypos);
    void OnKey(int key, int action);
    void OnMouseButton(int button, int action);
    void OnScroll(double yoffset);
    void OnWindowSize(int width, int height);

    static Application* FromWindow(GLFWwindow* window);
    static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void WindowSizeCallback(GLFWwindow* window, int width, int height);

    int screenWidth = 1500;
    int screenHeight = 800;
    GLFWwindow* window = nullptr;

    bool glfwInitialized = false;
    bool imguiContextCreated = false;
    bool imguiGlfwInitialized = false;
    bool imguiOpenGLInitialized = false;

    std::vector<std::shared_ptr<BaseFrame>> windows;
    std::vector<ShowcaseItem> showcase;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<ShaderFactory> shaderFactory;
    std::shared_ptr<Time> time;
};

#endif
