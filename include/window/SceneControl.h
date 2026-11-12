#ifndef SCENECONTROL_H
#define SCENECONTROL_H
#include "scene/SceneDocument.h"
#include "window/BaseWindow.h"
#include "window/SwitchableBaseWindow.h"
#include <functional>
#include <string>
#include <vector>

class Mesh;

class SceneControl : public BaseWindow
{
public:
    using WindowMode = SwitchableBaseWindow::WindowMode;
    using ModeChangeCallback = std::function<void(WindowMode)>;
    using GridToggleCallback = std::function<void(bool)>;
    using MeshAddedCallback = std::function<void(const std::shared_ptr<Mesh>&)>;

private:
    SceneDocument& sceneDocument;
    bool showGrid = false;
    WindowMode windowMode = WindowMode::MeshViewer;
    ModeChangeCallback modeChangeCallback;
    GridToggleCallback gridToggleCallback;
    MeshAddedCallback meshAddedCallback;
    void changeWindow(WindowMode window);

    // "Add Object" modal state. The model list is re-read each time the modal
    // is opened so newly added files show up.
    std::vector<std::string> modelFiles;
    std::string selectedModel;
    void refreshModelFiles();

    ImU32 imColorWhite = IM_COL32(255, 255, 255, 255);
    ImU32 imColorGrey = IM_COL32(111,111,111,255);

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
    void SetGridToggleCallback(GridToggleCallback callback);
    void SetMeshAddedCallback(MeshAddedCallback callback);
    void SetShowGrid(bool enabled);
    bool GetShowGrid() const;
};

#endif
