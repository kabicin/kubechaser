#include "window/SceneWorld.h"
#include "entity/Entity.h"
#include "scene/SceneDocument.h"
#include <GLFW/glfw3.h>
#include <limits>

SceneWorld::SceneWorld(const WindowServices& services, int x, int y, int width, int height)
    : BaseWindow(services, x, y, width, height)
{
}

SceneWorld::SceneWorld(const WindowServices& services, SceneDocument& sceneDocument, int x, int y, int width, int height)
    : BaseWindow(services, x, y, width, height), sceneDocument(&sceneDocument)
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

void SceneWorld::SetSceneDocument(SceneDocument* doc)
{
    sceneDocument = doc;
}

SceneDocument* SceneWorld::GetSceneDocument() const
{
    return sceneDocument;
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
        camera->UpdateFrame(GetTime()->GetDelta());
        camera->ClampMinY(1.0f);
    }
    if (manager) {
        manager->UpdateActiveScenes(camera ? camera->GetCameraPos() : glm::vec3(0.0f));
        manager->RenderActive(GetShaderFactory(), camera);
    } else if (scene) {
        scene->Render(GetShaderFactory(), camera);
    } else if (sceneDocument) {
        for (const auto& entity : sceneDocument->GetEntities()) {
            if (entity) {
                entity->Draw(GetShaderFactory(), camera);
            }
        }
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

void SceneWorld::OnCursorPosition(double xpos, double ypos)
{
    SetMousePosition(static_cast<int>(xpos), static_cast<int>(ypos));
}

void SceneWorld::OnMouseButton(int button, int action, double xpos, double ypos)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        SetLookActive(action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (sceneDocument && camera)
        {
            // The camera is sized in framebuffer pixels (see Render), but GLFW
            // reports the cursor in window coordinates; scale to match on
            // high-DPI (e.g. Retina) displays.
            ImGuiIO& io = ImGui::GetIO();
            double scaledX = xpos * io.DisplayFramebufferScale.x;
            double scaledY = ypos * io.DisplayFramebufferScale.y;
            Ray ray = camera->GetRay(scaledX, scaledY);
            double min_t = std::numeric_limits<double>::infinity();
            double temp_t = 0.0;
            int opt_i = -1;
            const auto& entities = sceneDocument->GetEntities();
            for (std::size_t i = 0; i < entities.size(); ++i)
            {
                if (entities[i] && entities[i]->Intersect(camera, ray, temp_t))
                {
                    if (temp_t < min_t)
                    {
                        min_t = temp_t;
                        opt_i = static_cast<int>(i);
                    }
                }
            }
            if (opt_i != -1)
            {
                sceneDocument->SetActiveEntity(opt_i);
            }
        }
    }
}

void SceneWorld::OnKey(int key, int action)
{
    KeyCallback(action, key);
}

void SceneWorld::OnDeactivate()
{
    SetLookActive(false);
}
