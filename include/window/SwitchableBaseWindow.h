#ifndef SWITCHABLEBASEWINDOW_H
#define SWITCHABLEBASEWINDOW_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "dearimgui.h"
#include "shader/Shader.h"
#include "logger/Logger.h"
#include "entity/Entity.h"
#include "Asset.h"
#include <memory>
#include "window/BaseFrame.h"
#include "window/BaseWindow.h"
#include "window/SceneControl.h"

class SwitchableBaseWindow : public BaseFrame
{
private:
    std::vector<std::shared_ptr<BaseWindow>> windows;
    std::shared_ptr<SceneControl> controller;
    int selectedWindow = 0;
public:
    SwitchableBaseWindow(int x, int y, int width, int height);
    void Render();
    void AttachSceneController(std::shared_ptr<SceneControl> controller);
    void AddWindow(std::shared_ptr<BaseWindow> window);
    void Resize(int x, int y, int width, int height);
};

#endif
