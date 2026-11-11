#ifndef SWITCHABLEBASEWINDOW_H
#define SWITCHABLEBASEWINDOW_H

#include "window/BaseFrame.h"
#include "window/BaseWindow.h"
#include <map>
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
                         std::shared_ptr<BaseWindow> meshViewer,
                         std::shared_ptr<BaseWindow> sceneWorld);

    void Render() override;
    void Resize(int x, int y, int width, int height) override;

    void RegisterWindow(WindowMode mode, std::shared_ptr<BaseWindow> window);
    std::shared_ptr<BaseWindow> GetWindow(WindowMode mode) const;

    void SetActiveMode(WindowMode mode);
    WindowMode GetActiveMode() const;
    std::shared_ptr<BaseWindow> GetActiveWindow() const;

    void OnCursorPosition(double xpos, double ypos);
    void OnKey(int key, int action);
    void OnMouseButton(int button, int action, double xpos, double ypos);
    void OnScroll(double yoffset);

private:
    std::map<WindowMode, std::shared_ptr<BaseWindow>> windows;
    std::shared_ptr<BaseWindow> activeWindow;
    WindowMode activeMode = WindowMode::MeshViewer;
};

#endif
