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

    int dy = 0;
    for (int dz = -activeRadius; dz <= activeRadius; dz++)
    {
        for (int dx = -activeRadius; dx <= activeRadius; dx++)
        {
            SceneCoord coord{baseX + dx, dy, baseZ + dz};
            activeCoords.push_back(coord);
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
    const float viewCosThreshold = 0.0f;

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

    // Floor tiles are batched by shade and drawn instanced (one draw per shade)
    // instead of one draw call + full uniform setup per tile. Tiles outside the
    // view frustum are skipped.
    const glm::mat4 viewProj = camera->GetProjection() * camera->GetView();
    glm::vec4 frustumPlanes[6];
    for (int i = 0; i < 3; i++) {
        glm::vec4 row(viewProj[0][i], viewProj[1][i], viewProj[2][i], viewProj[3][i]);
        glm::vec4 w(viewProj[0][3], viewProj[1][3], viewProj[2][3], viewProj[3][3]);
        frustumPlanes[2 * i] = w + row;
        frustumPlanes[2 * i + 1] = w - row;
    }
    auto boxInFrustum = [&](const glm::vec3& minCorner, const glm::vec3& maxCorner) {
        for (const auto& plane : frustumPlanes) {
            glm::vec3 positive(plane.x >= 0.0f ? maxCorner.x : minCorner.x,
                               plane.y >= 0.0f ? maxCorner.y : minCorner.y,
                               plane.z >= 0.0f ? maxCorner.z : minCorner.z);
            if (glm::dot(glm::vec3(plane), positive) + plane.w < 0.0f) {
                return false;
            }
        }
        return true;
    };

    constexpr int floorShadeCount = 6;
    std::vector<glm::vec3> floorOffsets[floorShadeCount];
    const float invScale = sceneSize != 0.0f ? 1.0f / sceneSize : 0.0f;
    const float floorY = -0.01f;
    const float halfTile = 0.5f * sceneSize;

    for (const auto& coord : activeCoords)
    {
        if (coord.y != 0) {
            continue;
        }
        glm::vec3 floorCenter(coord.x * sceneSize, floorY, coord.z * sceneSize);
        glm::vec3 extent(halfTile, 0.0f, halfTile);
        if (!boxInFrustum(floorCenter - extent, floorCenter + extent)) {
            continue;
        }
        // The quad lies in its local XY plane and is rotated 90 degrees about X
        // onto the XZ plane, so world (x, z) maps to local (x, y).
        int shadeIndex = std::abs((coord.x + coord.y + coord.z) % floorShadeCount);
        floorOffsets[shadeIndex].push_back(glm::vec3(coord.x, coord.z, 0.0f));
    }
    camera->cma.translate = glm::vec3(0.0f);
    floorQuad->SetScale(glm::vec3(sceneSize, sceneSize, sceneSize));
    floorQuad->SetTranslate(glm::vec3(0.0f, floorY * invScale, 0.0f));
    for (int shadeIndex = 0; shadeIndex < floorShadeCount; shadeIndex++) {
        float shade = 0.65f + 0.05f * static_cast<float>(shadeIndex);
        floorQuad->da.solidColor = glm::vec3(0.1f, 0.2f, shade);
        floorQuad->DrawInstanced(factory, camera, floorOffsets[shadeIndex]);
    }

    std::vector<glm::vec3> gridLineOffsets;
    gridLineOffsets.reserve(activeCoords.size());
    for (const auto& coord : activeCoords)
    {
        auto scene = GetScene(coord);
        if (scene == nullptr) {
            scene = dummyScene;
        }
        glm::vec3 tileOffset = glm::vec3(coord.x * sceneSize, coord.y * sceneSize, coord.z * sceneSize);
        glm::vec3 toTile = tileOffset - cameraPos;
        bool inView = true;
        if (glm::length(viewDir) > 0.0f && glm::length(toTile) > 0.0f) {
            inView = glm::dot(glm::normalize(toTile), viewDir) >= viewCosThreshold;
        }
        bool renderScene = inView;
        bool drawFloor = (coord.y == 0);
        camera->cma.translate = tileOffset;
        if (scene != dummyScene && renderScene) {
            scene->Render(factory, camera);
        }
        if (showGrid && grid != nullptr) {
            grid->SetScale(sceneSize);
            int axisMask = 0;
            if (coord.y == 0 && coord.z == 0) axisMask |= Grid::GridAxis_X;
            if (coord.y == 0 && coord.x == 0) axisMask |= Grid::GridAxis_Z;
            if (coord.x == 0 && coord.z == 0) axisMask |= Grid::GridAxis_Y;
            bool onAxisLine = (axisMask != 0);
            bool onXZPlane = drawFloor;
            bool verticalStack = (coord.x == 0 && coord.z == 0);
            if (onXZPlane) {
                gridLineOffsets.push_back(glm::vec3(tileOffset.x * invScale, tileOffset.y * invScale, tileOffset.z * invScale));
            }
            if (onAxisLine || verticalStack) {
                bool drawNeutralAxes = !verticalStack;
                glDisable(GL_DEPTH_TEST);
                grid->Draw(factory, camera, onAxisLine, axisMask, false, drawNeutralAxes);
                glEnable(GL_DEPTH_TEST);
            }
        }
    }

    if (showGrid && grid != nullptr && !gridLineOffsets.empty()) {
        camera->cma.translate = oldTranslate;
        glDisable(GL_DEPTH_TEST);
        grid->DrawLinesInstanced(factory, camera, gridLineOffsets);
        glEnable(GL_DEPTH_TEST);
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
