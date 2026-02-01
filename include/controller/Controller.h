#ifndef GLFWCONTROLLER_H
#define GLFWCONTROLLER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "utility/Camera.h"
#include "window/BaseWindow.h"
#include <memory>
#include <vector>
#include <utility>
#include <bitset>

typedef unsigned int ControllerType;
extern ControllerType ControllerType_KEYBOARD;
extern ControllerType ControllerType_MOUSE;
extern ControllerType ControllerType_JOYSTICK;

typedef std::pair<std::shared_ptr<BaseWindow>, ControllerType> Controllable;

class Controller
{
private:
    GLFWwindow* window;

    std::vector<Controllable> controllables;
    ControllerType boundedControllers = 0x000;

    std::bitset<GLFW_KEY_LAST> pressed;
    
    void handlePressed(int key, int scancode, int action, int mods);

    // callback functions
    void keyCallback(int key, int scancode, int action, int mods);

    // binding functions
    void bindKeyCallback();

public:
    Controller(GLFWwindow* window);
    void AttachWindow(std::shared_ptr<BaseWindow> window, ControllerType args = ControllerType_KEYBOARD | ControllerType_MOUSE);
};
#endif
