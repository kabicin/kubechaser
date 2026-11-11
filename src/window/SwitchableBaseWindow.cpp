#include "window/SwitchableBaseWindow.h"
#include "window/SceneWorld.h"
#include "window/MeshViewer.h"

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
