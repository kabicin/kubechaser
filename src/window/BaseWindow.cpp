#include "window/BaseWindow.h"
#include <iostream>

BaseWindow::BaseWindow(int x, int y, int width, int height)
    : BaseFrame(x, y, width, height)
{
}

void BaseWindow::SetCamera(std::shared_ptr<Camera> camera)
{
    this->camera = camera;
}

std::shared_ptr<Camera> BaseWindow::GetCamera()
{
    return camera;
}

void BaseWindow::KeyCallback(int key, int scancode, int action, int mods)
{
    // do nothing, meant to be overriden in derived classes
}

void BaseWindow::AttachShaderFactory(std::shared_ptr<ShaderFactory> factory)
{
    BaseWindow::factory = factory;
}

std::shared_ptr<ShaderFactory> BaseWindow::GetShaderFactory()
{
    return BaseWindow::factory;
}

void BaseWindow::AttachTime(std::shared_ptr<Time> time)
{
    BaseWindow::time = time;
}

std::shared_ptr<Time> BaseWindow::GetTime()
{
    return BaseWindow::time;
}

std::shared_ptr<ShaderFactory> BaseWindow::factory;
std::shared_ptr<Time> BaseWindow::time;




