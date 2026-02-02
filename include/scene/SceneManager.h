#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include "glm.h"
#include "scene/Scene.h"
#include "entity/Grid.h"
#include "entity/Quad.h"

struct SceneCoord {
    int x;
    int y;
    int z;
};

inline bool operator==(const SceneCoord& a, const SceneCoord& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct SceneCoordHash {
    size_t operator()(const SceneCoord& coord) const
    {
        size_t hx = std::hash<int>()(coord.x);
        size_t hy = std::hash<int>()(coord.y);
        size_t hz = std::hash<int>()(coord.z);
        return (hx << 1) ^ (hy << 2) ^ (hz << 3);
    }
};

class SceneManager
{
private:
    std::unordered_map<SceneCoord, std::shared_ptr<Scene>, SceneCoordHash> scenes;
    std::vector<SceneCoord> activeCoords;
    float sceneSize = 10.0f;
    int activeRadius = 12;
    bool showGrid = true;
    std::shared_ptr<Grid> grid;
    std::shared_ptr<Quad> floorQuad;
    std::shared_ptr<Scene> dummyScene;

public:
    SceneManager() = default;
    SceneManager(float sceneSize, int activeRadius);

    void AddScene(const SceneCoord& coord, const std::shared_ptr<Scene>& scene);
    std::shared_ptr<Scene> GetScene(const SceneCoord& coord) const;

    void SetSceneSize(float size);
    void SetActiveRadius(int radius);

    void UpdateActiveScenes(const glm::vec3& cameraPos);
    void RenderActive(const std::shared_ptr<ShaderFactory>& factory,
                      const std::shared_ptr<Camera>& camera);
    void SetShowGrid(bool enabled, const std::shared_ptr<Camera>& camera);
};
#endif
