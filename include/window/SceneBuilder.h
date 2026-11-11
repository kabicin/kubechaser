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

#define SCENEBUILDER_MAX_ENTITIES 50

class SceneBuilder : public BaseWindow
{
private:
    SceneDocument& sceneDocument;

    // Scene Builder specific object picking and ray casting members
    static int prev_x, prev_y, mouse_x, mouse_y;
    static bool clicked, holding, scaling, rotating, wireframe;
    static glm::vec3 prev;
    static glm::vec3 next;

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

    static void OnScroll(std::shared_ptr<Entity> entity, double yoffset);

    // Scene Builder specific object picking and ray casting functions
    bool GetMouseClicked();
    glm::vec3 GetMouseLocation();
    static void SetDownClick(int x, int y);
    static void SetDownClickPosition(int x, int y);
    static void SetReleaseClick();
    void SetHandledClick(bool handled);

    void SetClickOffset(glm::vec3 offset);
    void SetLastT(double t);

    // button press functions
    void KeyCallback(int key, int scancode, int action, int mods) override;

    // light
    static bool triggerBlinn;
    static int triggerLightSwitch;

    static void SetLightingEnvironment(std::shared_ptr<Shader> shader);
    static void AddDirectionLight(std::shared_ptr<Shader> shader);
    static void AddPointLight(std::shared_ptr<Shader> shader);
    static void AddSpotLight(std::shared_ptr<Shader> shader);
    static void AddMultiplePointLights(std::shared_ptr<Shader> shader);
    static void SwitchOffTheLights();


    // scene control variables
    static bool showGrid;
};
#endif
