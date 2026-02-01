#ifndef SCENECONTROL_H
#define SCENECONTROL_H
#include "window/BaseWindow.h"
#include "window/SceneBuilder.h"
#include "window/SceneWorld.h"

class SceneControl : public BaseWindow
{
private:
    bool propagateControl = false;
    int windowMode = 0;
    void changeWindow(int window);

    ImU32 imColorWhite = IM_COL32(255, 255, 255, 255);
    ImU32 imColorGrey = IM_COL32(111,111,111,255);
public:
    SceneControl(int x, int y, int width, int height);
    ~SceneControl();
    void Render();
    bool HasChanged();
    int GetWindowMode();
};

#endif
