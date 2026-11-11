#include "window/SwitchableBaseWindow.h"
#include "window/SceneWorld.h"
#include "window/MeshViewer.h"
#include <GLFW/glfw3.h>

SwitchableBaseWindow::SwitchableBaseWindow(int x, int y, int width, int height)
    : BaseFrame(x, y, width, height)
{
}

SwitchableBaseWindow::SwitchableBaseWindow(int x, int y, int width, int height,
                                           std::shared_ptr<MeshViewer> meshViewer,
                                           std::shared_ptr<SceneWorld> sceneWorld)
    : BaseFrame(x, y, width, height)
{
    SetMeshViewer(meshViewer);
    SetSceneWorld(sceneWorld);
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
    if (meshViewer)
    {
        meshViewer->Resize(x, y, width, height);
    }
    if (sceneWorld)
    {
        sceneWorld->Resize(x, y, width, height);
    }
}

void SwitchableBaseWindow::SetMeshViewer(std::shared_ptr<MeshViewer> viewer)
{
    meshViewer = viewer;
    if (activeMode == WindowMode::MeshViewer)
    {
        activeWindow = meshViewer;
        if (activeWindow)
        {
            activeWindow->Show();
        }
    }
    else if (meshViewer)
    {
        meshViewer->Hide();
    }
}

void SwitchableBaseWindow::SetSceneWorld(std::shared_ptr<SceneWorld> world)
{
    sceneWorld = world;
    if (activeMode == WindowMode::SceneWorld)
    {
        activeWindow = sceneWorld;
        if (activeWindow)
        {
            activeWindow->Show();
        }
    }
    else if (sceneWorld)
    {
        sceneWorld->Hide();
    }
}

std::shared_ptr<MeshViewer> SwitchableBaseWindow::GetMeshViewer() const
{
    return meshViewer;
}

std::shared_ptr<SceneWorld> SwitchableBaseWindow::GetSceneWorld() const
{
    return sceneWorld;
}

void SwitchableBaseWindow::SetActiveMode(WindowMode mode)
{
    if (activeWindow)
    {
        activeWindow->Hide();
    }

    // Reset interaction state on deactivate
    if (activeMode == WindowMode::MeshViewer && meshViewer)
    {
        meshViewer->SetReleaseClick();
    }
    else if (activeMode == WindowMode::SceneWorld && sceneWorld)
    {
        sceneWorld->SetLookActive(false);
    }

    activeMode = mode;
    switch (activeMode)
    {
        case WindowMode::MeshViewer:
            activeWindow = meshViewer;
            break;
        case WindowMode::SceneWorld:
            activeWindow = sceneWorld;
            break;
    }

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
    const int x = static_cast<int>(xpos);
    const int y = static_cast<int>(ypos);
    if (activeMode == WindowMode::MeshViewer && meshViewer)
    {
        meshViewer->SetDownClickPosition(x, y);
    }
    else if (activeMode == WindowMode::SceneWorld && sceneWorld)
    {
        sceneWorld->SetMousePosition(x, y);
    }
}

void SwitchableBaseWindow::OnKey(int key, int action)
{
    if (activeMode == WindowMode::MeshViewer && meshViewer)
    {
        meshViewer->KeyCallback(action, key);
    }
    else if (activeMode == WindowMode::SceneWorld && sceneWorld)
    {
        sceneWorld->KeyCallback(action, key);
    }
}

void SwitchableBaseWindow::OnMouseButton(int button, int action, double xpos, double ypos)
{
    if (activeMode == WindowMode::MeshViewer && meshViewer)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                meshViewer->SetDownClick(static_cast<int>(xpos), static_cast<int>(ypos));
            }
            else
            {
                meshViewer->SetReleaseClick();
            }
        }
    }
    else if (activeMode == WindowMode::SceneWorld && sceneWorld)
    {
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            sceneWorld->SetLookActive(action == GLFW_PRESS || action == GLFW_REPEAT);
        }
    }
}

void SwitchableBaseWindow::OnScroll(double yoffset)
{
    if (activeMode == WindowMode::MeshViewer && meshViewer)
    {
        meshViewer->OnScroll(yoffset);
    }
}
