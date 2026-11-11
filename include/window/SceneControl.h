#ifndef SCENECONTROL_H
#define SCENECONTROL_H
#include "scene/SceneDocument.h"
#include "window/BaseWindow.h"
#include "window/SceneWorld.h"
#include "window/SwitchableBaseWindow.h"
#include <functional>

class SceneControl : public BaseWindow
{
public:
    using WindowMode = SwitchableBaseWindow::WindowMode;
    using ModeChangeCallback = std::function<void(WindowMode)>;

private:
    SceneDocument& sceneDocument;
    bool showGrid = false;
    WindowMode windowMode = WindowMode::MeshViewer;
    ModeChangeCallback modeChangeCallback;
    void changeWindow(WindowMode window);

    ImU32 imColorWhite = IM_COL32(255, 255, 255, 255);
    ImU32 imColorGrey = IM_COL32(111,111,111,255);

    std::weak_ptr<SwitchableBaseWindow> sbWindow;
public:
    SceneControl(
        const WindowServices& services,
        SceneDocument& sceneDocument,
        int x,
        int y,
        int width,
        int height);
    ~SceneControl();
    void Render();
    WindowMode GetWindowMode() const;
    void SetModeChangeCallback(ModeChangeCallback callback);

    void AttachSwitchableBaseWindow(const std::shared_ptr<SwitchableBaseWindow>& sbWindow);
};

#endif
