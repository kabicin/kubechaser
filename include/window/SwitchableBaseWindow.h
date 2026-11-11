#ifndef SWITCHABLEBASEWINDOW_H
#define SWITCHABLEBASEWINDOW_H

#include "window/BaseFrame.h"
#include "window/BaseWindow.h"
#include "window/MeshViewer.h"
#include "window/SceneWorld.h"
#include <memory>

enum class WindowMode
{
    MeshViewer,
    SceneWorld
};

class SwitchableBaseWindow : public BaseFrame
{
public:
    using WindowMode = ::WindowMode;

    SwitchableBaseWindow(int x, int y, int width, int height);
    SwitchableBaseWindow(int x, int y, int width, int height,
                         std::shared_ptr<MeshViewer> meshViewer,
                         std::shared_ptr<SceneWorld> sceneWorld);

    void Render() override;
    void Resize(int x, int y, int width, int height) override;

    void SetMeshViewer(std::shared_ptr<MeshViewer> viewer);
    void SetSceneWorld(std::shared_ptr<SceneWorld> world);

    std::shared_ptr<MeshViewer> GetMeshViewer() const;
    std::shared_ptr<SceneWorld> GetSceneWorld() const;

    void SetActiveMode(WindowMode mode);
    WindowMode GetActiveMode() const;
    std::shared_ptr<BaseWindow> GetActiveWindow() const;

    void OnCursorPosition(double xpos, double ypos);
    void OnKey(int key, int action);
    void OnMouseButton(int button, int action, double xpos, double ypos);
    void OnScroll(double yoffset);

private:
    std::shared_ptr<MeshViewer> meshViewer;
    std::shared_ptr<SceneWorld> sceneWorld;
    std::shared_ptr<BaseWindow> activeWindow;
    WindowMode activeMode = WindowMode::MeshViewer;
};

#endif
