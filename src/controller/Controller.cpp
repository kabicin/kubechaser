#include "controller/Controller.h"

ControllerType ControllerType_KEYBOARD = 1;
ControllerType ControllerType_MOUSE = 2;
ControllerType ControllerType_JOYSTICK = 4;

Controller::Controller(GLFWwindow* window)
{
    this->window = window;
    this->controllables = std::vector<Controllable>();
}

void Controller::AttachWindow(std::shared_ptr<BaseWindow> window, ControllerType args)
{
    this->controllables.push_back(std::make_pair(window, args));
    // if controller is not bound, bind it
    if (!(this->boundedControllers & args)) {
        if (args | ControllerType_KEYBOARD) bindKeyCallback();
        if (args | ControllerType_MOUSE) {}     // TODO: implement bindMouseCallback()
        if (args | ControllerType_JOYSTICK) {}  // TODO: implement bindJoystickCallback()
    }
}

void Controller::handlePressed(int key, int scancode, int action, int mods)
{
    if (key < 0 || key > GLFW_KEY_LAST) return;
    if (action == GLFW_PRESS) pressed.set(key, true);
    else if (action == GLFW_RELEASE) pressed.set(key, false);
}

void Controller::keyCallback(int key, int scancode, int action, int mods)
{
    handlePressed(key, scancode, action, mods);

    std::shared_ptr<BaseWindow> BaseWindow;
    std::shared_ptr<Camera> camera;

    for (Controllable controllable : this->controllables) {
        // call key callback overrides in BaseWindow classes
        BaseWindow = controllable.first;
        BaseWindow->KeyCallback(key, scancode, action, mods);
        camera = BaseWindow->GetCamera();
        if (camera == nullptr) continue;
        // call key callback based on controller configs
        ControllerType type = controllable.second;
        if (type | ControllerType_KEYBOARD) {
            if (pressed[GLFW_KEY_W]) camera->SetForward();
            if (pressed[GLFW_KEY_A]) camera->SetLeft();
            if (pressed[GLFW_KEY_S]) camera->SetBackward();
            if (pressed[GLFW_KEY_D]) camera->SetRight();
            if (pressed[GLFW_KEY_SPACE]) camera->SetUp();
            if (pressed[GLFW_KEY_LEFT_SHIFT]) camera->SetDown();
            if (pressed[GLFW_KEY_R]) camera->ResetCamPos();
            if (pressed == 0) camera->SetCenter();
        }
        if (type | ControllerType_MOUSE) {
            // TODO: implement default mouse GLFW events
        }
        if (type | ControllerType_JOYSTICK) {
            // TODO: implement default joystick GLFW events
        }
    }
}

void Controller::bindKeyCallback()
{
    glfwSetWindowUserPointer(window, this);
    auto callback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Controller* controller = static_cast<Controller*>(glfwGetWindowUserPointer(window));
        controller->keyCallback(key, scancode, action, mods);
    };
    glfwSetKeyCallback(window, callback);
}