#ifndef SCENECONTROL_H
#define SCENECONTROL_H
#include "window/BaseWindow.h"
#include "window/SceneBuilder.h"
#include "window/SceneWorld.h"

class SwitchableBaseWindow;

class SceneControl : public BaseWindow
{
private:
    bool propagateControl = false;
    int windowMode = 0;
    void changeWindow(int window);

    ImU32 imColorWhite = IM_COL32(255, 255, 255, 255);
    ImU32 imColorGrey = IM_COL32(111,111,111,255);

    std::weak_ptr<SwitchableBaseWindow> sbWindow;
public:
    SceneControl(int x, int y, int width, int height);
    ~SceneControl();
    void Render();
    bool HasChanged();
    int GetWindowMode();

    void AttachSwitchableBaseWindow(const std::shared_ptr<SwitchableBaseWindow>& sbWindow);
};

#endif
