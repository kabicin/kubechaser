#include "window/SceneWorld.h"
#include <GLFW/glfw3.h>

SceneWorld::SceneWorld(const WindowServices& services, int x, int y, int width, int height)
    : BaseWindow(services, x, y, width, height)
{
}

SceneWorld::SceneWorld(const WindowServices& services, int x, int y, int width, int height, std::shared_ptr<Camera> playerCamera)
    : BaseWindow(services, x, y, width, height)
{
    AddCamera(playerCamera);
}

SceneWorld::~SceneWorld()
{
}

void SceneWorld::Resize(int x, int y, int width, int height)
{
    // grab new dimensions to update camera..
    if (camera != nullptr) camera->Resize(width, height);
    BaseWindow::Resize(x, y, width, height);
}

void SceneWorld::Render()
{
    ClearBackground(0.1f, 0.2f, 0.65f, 1.0f);
    ImGuiIO& io = ImGui::GetIO();
    int fbWidth = static_cast<int>(GetWidth() * io.DisplayFramebufferScale.x);
    int fbHeight = static_cast<int>(GetHeight() * io.DisplayFramebufferScale.y);
    glViewport(0, 0, fbWidth, fbHeight);
    if (camera) camera->Resize(fbWidth, fbHeight);
    handleInput();
    if (camera) {
        camera->UpdateFrame(GetTime()->GetLastDeltaFrames());
        camera->ClampMinY(1.0f);
    }
    if (manager) {
        manager->UpdateActiveScenes(camera ? camera->GetCameraPos() : glm::vec3(0.0f));
        manager->RenderActive(GetShaderFactory(), camera);
    } else if (scene) {
        scene->Render(GetShaderFactory(), camera);
    }
}

void SceneWorld::AddCamera(std::shared_ptr<Camera> playerCamera)
{
    camera = playerCamera;
}

void SceneWorld::AddScene(std::shared_ptr<Scene> scene)
{
    this->scene = scene;
}

void SceneWorld::AddSceneManager(std::shared_ptr<SceneManager> manager)
{
    this->manager = manager;
}

void SceneWorld::SetShowGrid(bool enabled)
{
    if (manager) {
        manager->SetShowGrid(enabled, camera);
    } else if (scene) {
        scene->SetShowGrid(enabled, camera);
    }
}

void SceneWorld::handleInput()
{
    if (camera == nullptr) {
        return;
    }

    if (pressed[GLFW_KEY_W]) camera->SetForward();
    if (pressed[GLFW_KEY_A]) camera->SetLeft();
    if (pressed[GLFW_KEY_S]) camera->SetBackward();
    if (pressed[GLFW_KEY_D]) camera->SetRight();
    if (pressed[GLFW_KEY_SPACE]) camera->SetUp();
    if (pressed[GLFW_KEY_LEFT_SHIFT]) camera->SetDown();
    if (pressed[GLFW_KEY_R]) camera->ResetCamPos();
    if (pressed == 0) camera->SetCenter();

    if (!look_active) {
        mouse_initialized = false;
        return;
    }

    if (!mouse_initialized) {
        prev_x = mouse_x;
        prev_y = mouse_y;
        mouse_initialized = true;
        return;
    }

    float xoffset = static_cast<float>(mouse_x - prev_x);
    float yoffset = static_cast<float>(prev_y - mouse_y);
    prev_x = mouse_x;
    prev_y = mouse_y;

    yaw_deg += xoffset * look_sensitivity;
    pitch_deg += yoffset * look_sensitivity;
    if (pitch_deg > 89.0f) pitch_deg = 89.0f;
    if (pitch_deg < -89.0f) pitch_deg = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw_deg)) * cos(glm::radians(pitch_deg));
    direction.y = sin(glm::radians(pitch_deg));
    direction.z = sin(glm::radians(yaw_deg)) * cos(glm::radians(pitch_deg));
    camera->SetLookDirection(direction);
}

void SceneWorld::KeyCallback(int action, int key)
{
    if (key < 0 || key > GLFW_KEY_LAST) return;
    if (action == GLFW_PRESS) pressed.set(key, true);
    else if (action == GLFW_RELEASE) pressed.set(key, false);
}

void SceneWorld::SetMousePosition(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
}

void SceneWorld::SetLookActive(bool active)
{
    look_active = active;
    mouse_initialized = false;
}
