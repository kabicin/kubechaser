#include "window/BaseWindow.h"
#include <iostream>

BaseWindow::BaseWindow(const WindowServices& services, int x, int y, int width, int height)
    : BaseFrame(x, y, width, height), services(services)
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

void BaseWindow::OnCursorPosition(double, double)
{
}

void BaseWindow::OnMouseButton(int, int, double, double)
{
}

void BaseWindow::OnKey(int, int)
{
}

void BaseWindow::OnScroll(double)
{
}

void BaseWindow::OnDeactivate()
{
}

std::shared_ptr<Logger> BaseWindow::GetLogger() const
{
    return services.logger;
}

std::shared_ptr<ShaderFactory> BaseWindow::GetShaderFactory() const
{
    return services.shaderFactory;
}

std::shared_ptr<Time> BaseWindow::GetTime() const
{
    return services.time;
}
