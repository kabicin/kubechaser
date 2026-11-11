#ifndef SCENECONTROL_H
#define SCENECONTROL_H
#include "scene/SceneDocument.h"
#include "window/BaseWindow.h"
#include "window/SceneWorld.h"

class SwitchableBaseWindow;

class SceneControl : public BaseWindow
{
private:
    SceneDocument& sceneDocument;
    bool propagateControl = false;
    bool showGrid = false;
    int windowMode = 0;
    void changeWindow(int window);

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
    bool HasChanged();
    int GetWindowMode();

    void AttachSwitchableBaseWindow(const std::shared_ptr<SwitchableBaseWindow>& sbWindow);
};

#endif
