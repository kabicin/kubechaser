#ifndef SCENEBUILDER_H
#define SCENEBUILDER_H
#include "BaseWindow.h"
#include "entity/Shapes.h"
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
    static std::vector<std::shared_ptr<Entity>> entities;
    static size_t activeEntityIndex;

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
    SceneBuilder(int x, int y, int width, int height);
    ~SceneBuilder();
    void Render() override;
    void Resize(int x, int y, int width, int height) override;

    static std::shared_ptr<Entity> AddEntity(std::shared_ptr<Entity> entity, glm::vec3 offset = glm::vec3(0, 0, 0));
    static std::shared_ptr<Entity> AddEntity(const std::string& className, glm::vec3 offset = glm::vec3(0, 0, 0));
    static std::shared_ptr<Entity> AddEntity(const std::string& className, const std::string& objectFile, glm::vec3 offset = glm::vec3(0, 0, 0));
    static std::shared_ptr<Entity> AddEntity(const std::string& className, const std::string& objectFile, const std::string& textureFile, glm::vec3 offset = glm::vec3(0, 0, 0));
    static std::vector<std::shared_ptr<Entity>> GetAllEntities();
    static size_t GetNumEntities();
    static void SetActiveEntity(int index);
    static std::shared_ptr<Entity> GetActiveEntity();
    static void DeleteEntity(int index);
    static void DeleteEntity(std::shared_ptr<Entity> entity);

    static void OnScroll(std::shared_ptr<Entity> entity, double yoffset);
    static double maxDepth;
    static double minDepth;
    static double GetMaxDepth();
    static double GetMinDepth();

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