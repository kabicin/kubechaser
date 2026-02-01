#include "window/SwitchableBaseWindow.h"

SwitchableBaseWindow::SwitchableBaseWindow(int x, int y, int width, int height)
    : BaseFrame(x, y, width, height)
{
}

void SwitchableBaseWindow::AttachSceneController(std::shared_ptr<SceneControl> controller)
{
    this->controller = controller;
}

void SwitchableBaseWindow::Render()
{
    if (controller->HasChanged())
    {
        // hide previous window
        windows[selectedWindow]->Hide();
        int mode = controller->GetWindowMode();
        if (mode >= 0 && mode < windows.size()) selectedWindow = mode;
        // then show the new window
        windows[selectedWindow]->Show();
    }

    if (selectedWindow < windows.size())
    {
        windows[selectedWindow]->Render();
    }
}

void SwitchableBaseWindow::AddWindow(std::shared_ptr<BaseWindow> window)
{
    if (windows.size() != selectedWindow) {
        window->Hide();
    }
    windows.push_back(window);
}

void SwitchableBaseWindow::Resize(int x, int y, int width, int height)
{
    for(std::shared_ptr<BaseWindow> window : windows)
    {
        window->Resize(x, y, width, height);
    }
}



