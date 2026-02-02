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
        if (!windows.empty() && selectedWindow >= 0 && selectedWindow < static_cast<int>(windows.size()))
        {
            // hide previous window
            windows[selectedWindow]->Hide();
        }
        int mode = controller->GetWindowMode();
        if (mode >= 0 && mode < static_cast<int>(windows.size())) {
            selectedWindow = mode;
        } else {
            std::cout << "SwitchableBaseWindow: invalid window index " << mode
                      << " (size " << windows.size() << ")" << std::endl;
        }
        // then show the new window
        if (!windows.empty() && selectedWindow >= 0 && selectedWindow < static_cast<int>(windows.size()))
        {
            windows[selectedWindow]->Show();
        }
    }

    if (selectedWindow >= 0 && selectedWindow < static_cast<int>(windows.size()))
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

void SwitchableBaseWindow::SetActiveMeshViewer(int meshViewerIndex) {
    selectedMeshViewer = meshViewerIndex;
}

std::shared_ptr<MeshViewer> SwitchableBaseWindow::GetMeshViewer() {
    return std::dynamic_pointer_cast<MeshViewer>(windows[selectedMeshViewer]);
}
