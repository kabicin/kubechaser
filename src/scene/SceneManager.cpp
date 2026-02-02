#include "scene/SceneManager.h"
#include <cmath>

SceneManager::SceneManager(float sceneSize, int activeRadius)
    : sceneSize(sceneSize), activeRadius(activeRadius)
{
}

void SceneManager::AddScene(const SceneCoord& coord, const std::shared_ptr<Scene>& scene)
{
    scenes[coord] = scene;
}

std::shared_ptr<Scene> SceneManager::GetScene(const SceneCoord& coord) const
{
    auto it = scenes.find(coord);
    if (it == scenes.end()) {
        return nullptr;
    }
    return it->second;
}

void SceneManager::SetSceneSize(float size)
{
    sceneSize = size;
}

void SceneManager::SetActiveRadius(int radius)
{
    activeRadius = radius;
}

void SceneManager::UpdateActiveScenes(const glm::vec3& cameraPos)
{
    if (sceneSize <= 0.0f) {
        return;
    }

    const int baseX = static_cast<int>(std::floor(cameraPos.x / sceneSize));
    const int baseY = static_cast<int>(std::floor(cameraPos.y / sceneSize));
    const int baseZ = static_cast<int>(std::floor(cameraPos.z / sceneSize));

    activeCoords.clear();
    activeCoords.reserve((2 * activeRadius + 1) * (2 * activeRadius + 1));

    for (int dy = -activeRadius; dy <= activeRadius; dy++)
    {
        for (int dz = -activeRadius; dz <= activeRadius; dz++)
        {
            for (int dx = -activeRadius; dx <= activeRadius; dx++)
            {
                SceneCoord coord{baseX + dx, baseY + dy, baseZ + dz};
                activeCoords.push_back(coord);
            }
        }
    }
}

void SceneManager::RenderActive(const std::shared_ptr<ShaderFactory>& factory,
                                const std::shared_ptr<Camera>& camera)
{
    if (camera == nullptr) {
        return;
    }

    glm::vec3 cameraPos = camera->GetCameraPos();
    glm::vec3 viewDir = camera->GetLookDirection();
    if (glm::length(viewDir) > 0.0f) {
        viewDir = glm::normalize(viewDir);
    }

    if (dummyScene == nullptr) {
        dummyScene = std::make_shared<Scene>();
    }
    if (floorQuad == nullptr) {
        floorQuad = std::make_shared<Quad>();
        floorQuad->RotateX(90.0f);
        floorQuad->da.color = true;
        floorQuad->da.useVertexColor = false;
        floorQuad->da.lightSwitch = 0;
        floorQuad->da.lightBlinn = false;
        floorQuad->da.texture = false;
    }

    glm::vec3 oldTranslate = camera->cma.translate;
    for (const auto& coord : activeCoords)
    {
        auto scene = GetScene(coord);
        if (scene == nullptr) {
            scene = dummyScene;
        }
        glm::vec3 tileOffset = glm::vec3(coord.x * sceneSize, coord.y * sceneSize, coord.z * sceneSize);
        glm::vec3 toTile = tileOffset - cameraPos;
        bool inFront = true;
        if (glm::length(viewDir) > 0.0f && glm::length(toTile) > 0.0f) {
            inFront = glm::dot(glm::normalize(toTile), viewDir) >= 0.0f;
        }
        camera->cma.translate = glm::vec3(0.0f);
        floorQuad->Scale(glm::vec3(sceneSize, sceneSize, sceneSize));
        float invScale = sceneSize != 0.0f ? 1.0f / sceneSize : 0.0f;
        floorQuad->SetTranslate(glm::vec3(tileOffset.x * invScale, (tileOffset.y - 0.01f) * invScale, tileOffset.z * invScale));
        float shade = 0.65f + 0.05f * static_cast<float>(std::abs((coord.x + coord.y + coord.z) % 6));
        floorQuad->da.solidColor = glm::vec3(0.1f, 0.2f, shade);
        if (coord.y == 0) {
            floorQuad->Draw(factory, camera);
        }
        camera->cma.translate = tileOffset;
        if (showGrid && grid != nullptr) {
            grid->SetScale(sceneSize);
            int axisMask = 0;
            if (coord.y == 0 && coord.z == 0) axisMask |= Grid::GridAxis_X;
            if (coord.y == 0 && coord.x == 0) axisMask |= Grid::GridAxis_Z;
            if (coord.x == 0 && coord.z == 0) axisMask |= Grid::GridAxis_Y;
            bool onAxisLine = (axisMask != 0);
            bool onXZPlane = (coord.y == 0);
            bool verticalStack = (coord.x == 0 && coord.z == 0);
            if (onXZPlane || verticalStack) {
                bool drawLines = onXZPlane;
                bool drawNeutralAxes = !verticalStack;
                grid->Draw(factory, camera, onAxisLine, axisMask, drawLines, drawNeutralAxes);
            }
        }
        if (scene != dummyScene) {
            scene->Render(factory, camera);
        }
    }
    camera->cma.translate = oldTranslate;
}

void SceneManager::SetShowGrid(bool enabled, const std::shared_ptr<Camera>& camera)
{
    showGrid = enabled;
    if (showGrid && grid == nullptr && camera != nullptr) {
        grid = std::make_shared<Grid>(camera);
        grid->SetNeutralColor(glm::vec4(0.85f, 0.85f, 0.85f, 0.3f));
    }
}
