#include "window/SwitchableBaseWindow.h"

SwitchableBaseWindow::SwitchableBaseWindow(int x, int y, int width, int height)
    : BaseFrame(x, y, width, height)
{
}

SwitchableBaseWindow::SwitchableBaseWindow(int x, int y, int width, int height,
                                           std::shared_ptr<BaseWindow> meshViewer,
                                           std::shared_ptr<BaseWindow> sceneWorld)
    : BaseFrame(x, y, width, height)
{
    RegisterWindow(WindowMode::MeshViewer, meshViewer);
    RegisterWindow(WindowMode::SceneWorld, sceneWorld);
    SetActiveMode(WindowMode::MeshViewer);
}

void SwitchableBaseWindow::Render()
{
    if (activeWindow)
    {
        activeWindow->Render();
    }
}

void SwitchableBaseWindow::Resize(int x, int y, int width, int height)
{
    BaseFrame::Resize(x, y, width, height);
    for (const auto& [mode, window] : windows)
    {
        if (window)
        {
            window->Resize(x, y, width, height);
        }
    }
}

void SwitchableBaseWindow::RegisterWindow(WindowMode mode, std::shared_ptr<BaseWindow> window)
{
    windows[mode] = window;
    if (activeMode == mode)
    {
        activeWindow = window;
        if (activeWindow)
        {
            activeWindow->Show();
        }
    }
    else if (window)
    {
        window->Hide();
    }
}

std::shared_ptr<BaseWindow> SwitchableBaseWindow::GetWindow(WindowMode mode) const
{
    auto it = windows.find(mode);
    if (it != windows.end())
    {
        return it->second;
    }
    return nullptr;
}

void SwitchableBaseWindow::SetActiveMode(WindowMode mode)
{
    if (activeWindow)
    {
        activeWindow->OnDeactivate();
        activeWindow->Hide();
    }

    activeMode = mode;
    activeWindow = GetWindow(mode);

    if (activeWindow)
    {
        activeWindow->Show();
    }
}

WindowMode SwitchableBaseWindow::GetActiveMode() const
{
    return activeMode;
}

std::shared_ptr<BaseWindow> SwitchableBaseWindow::GetActiveWindow() const
{
    return activeWindow;
}

void SwitchableBaseWindow::OnCursorPosition(double xpos, double ypos)
{
    if (activeWindow)
    {
        activeWindow->OnCursorPosition(xpos, ypos);
    }
}

void SwitchableBaseWindow::OnKey(int key, int action)
{
    if (activeWindow)
    {
        activeWindow->OnKey(key, action);
    }
}

void SwitchableBaseWindow::OnMouseButton(int button, int action, double xpos, double ypos)
{
    if (activeWindow)
    {
        activeWindow->OnMouseButton(button, action, xpos, ypos);
    }
}

void SwitchableBaseWindow::OnScroll(double yoffset)
{
    if (activeWindow)
    {
        activeWindow->OnScroll(yoffset);
    }
}
