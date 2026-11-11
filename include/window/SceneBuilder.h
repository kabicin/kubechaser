#ifndef SCENEBUILDER_H
#define SCENEBUILDER_H
#include "BaseWindow.h"
#include "scene/SceneDocument.h"
#include "utility/Camera.h"
#include "shader/ShaderFactory.h"
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include "scene/Scene.h"

class SceneBuilder : public BaseWindow
{
private:
    SceneDocument& sceneDocument;

    // Scene Builder specific object picking and ray casting members
    int prev_x = 0;
    int prev_y = 0;
    int mouse_x = 0;
    int mouse_y = 0;
    bool clicked = false;
    bool holding = false;
    bool scaling = false;
    bool rotating = false;

    void onClick(int x, int y);
    void onClick();

    glm::vec3 clickOffset;
    double lastT;

    std::shared_ptr<Scene> scene;

public:
    SceneBuilder(
        const WindowServices& services,
        SceneDocument& sceneDocument,
        int x,
        int y,
        int width,
        int height);
    ~SceneBuilder() = default;
    void Render() override;
    void Resize(int x, int y, int width, int height) override;

    void OnScroll(std::shared_ptr<Entity> entity, double yoffset);

    // Scene Builder specific object picking and ray casting functions
    bool GetMouseClicked();
    glm::vec3 GetMouseLocation();
    void SetDownClick(int x, int y);
    void SetDownClickPosition(int x, int y);
    void SetReleaseClick();
    void SetHandledClick(bool handled);

    void SetClickOffset(glm::vec3 offset);
    void SetLastT(double t);

    // button press functions
    void KeyCallback(int key, int scancode, int action, int mods) override;
};
#endif
