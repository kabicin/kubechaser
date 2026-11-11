#include "window/SceneBuilder.h"
#include "utility/Reader.h"

SceneBuilder::SceneBuilder(
    const WindowServices& services,
    SceneDocument& sceneDocument,
    int x,
    int y,
    int width,
    int height)
    : BaseWindow(services, x, y, width, height),
      sceneDocument(sceneDocument)
{
    std::cout << "Scene Builder: " << x << " " << y << " " << width << " " << height << std::endl;
    this->SetCamera(std::make_shared<Camera>(width, height));
    
    std::shared_ptr<Entity> mesh = this->sceneDocument.AddEntity("Mesh", "statefulset.obj");
    mesh->Scale(glm::vec3(0.005, 0.005, 0.005));
    mesh->Translate(glm::vec3(0, -90, 250));

    // create scene node root
    std::shared_ptr<SceneNode> root = std::make_shared<SceneNode>();
    for (const auto& entity : this->sceneDocument.GetEntities()) {
        root->AddDynamicObject(entity);
    }

    // initialize scene based on root
    scene = std::make_shared<Scene>(root);
}

void SceneBuilder::Resize(int x, int y, int width, int height)
{
    GetCamera()->Resize(width, height);
    BaseWindow::Resize(x, y, width, height);
}

void SceneBuilder::onClick(int x, int y)
{
    // Picking code
    Ray ray = GetCamera()->GetRay(x, y);
    double min_t = std::numeric_limits<double>::infinity(), temp_t;
    int opt_i = -1;
    const auto& entities = sceneDocument.GetEntities();
    int n = static_cast<int>(entities.size());
    for (int i = 0; i < n; i++)
    {
        if (entities[i]->Intersect(GetCamera(), ray, temp_t)) {
            min_t = std::min(min_t, temp_t);
            opt_i = i;
        }
    }
    if (opt_i >= 0 && opt_i < n)
    {
        entities[opt_i]->SetObjectPicked();
        sceneDocument.SetActiveEntity(opt_i);
        Eigen::Vector3d newray(ray.eye + ray.direction * min_t);
        glm::vec3 gnewray = glm::vec3(newray(0), newray(1), newray(2));
        SetClickOffset(gnewray - entities[opt_i]->translate);
        SetLastT(min_t);
        holding = true;
    }
    SetHandledClick(true);
}

void SceneBuilder::onClick()
{
    // Picking code
    onClick(mouse_x, mouse_y);
}

void SceneBuilder::SetClickOffset(glm::vec3 offset)
{
    clickOffset = offset;
}

void SceneBuilder::SetLastT(double last)
{
    this->lastT = last;
}

void SceneBuilder::OnScroll(std::shared_ptr<Entity> entity, double yoffset)
{
    if (entity != NULL)
    {
        if (yoffset > 0)
        {
            entity->Translate(glm::vec3(0.0f, 0.0f, 0.01f));
        }
        else if (yoffset < 0)
        {
            entity->Translate(glm::vec3(0.0f, 0.0f, -0.01f));
        }
    }
}

void SceneBuilder::Render()
{
    ClearBackground(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, GetWidth(), GetHeight());

    // update camera position per frame
    GetCamera()->UpdateFrame(GetTime()->GetLastDeltaFrames());
    
    // Picking Code
    if (GetMouseClicked()) {
        onClick();
    }

    // Holding Code
    if (holding)
    {
        std::shared_ptr<Entity> activeEntity = sceneDocument.GetActiveEntity();
        if (!activeEntity) {
            holding = false;
        } else {
            // handle mouse offset
            int xOffset = mouse_x - prev_x;
            int yOffset = prev_y - mouse_y; // flip because of GLFW coords
            // has it moved...
            if (xOffset != 0 || yOffset != 0)
            {
                // if shift button pressed - scale
                if (scaling)
                {
                    double scaleFactor = 0.05;
                    if (xOffset > 0) activeEntity->Scale(1.0 + scaleFactor);
                    else activeEntity->Scale(1.0 - scaleFactor);
                }
                // if ctrl button pressed - rotate
                else if (rotating)
                {
                    float rotateFactor = 4.0f;
                    if (xOffset > 0) activeEntity->RotateZInc(rotateFactor);
                    else activeEntity->RotateZInc(-rotateFactor);
                }
                // else - translate via ray casting
                else
                {
                    if (xOffset != 0 || yOffset != 0)
                    {
                        Ray prevRay = GetCamera()->GetRay(prev_x, prev_y);
                        Ray currRay = GetCamera()->GetRay(mouse_x, mouse_y);
                        Eigen::Vector3d point = prevRay.Point(this->lastT);
                        Eigen::Vector3d normal = Linalg::GLM2Eigen(GetCamera()->GetLookDirection()); // camera direction
                        Plane2 plane(point, normal);
                        Eigen::Vector3d poi, n;
                        double t;
                        if (Linalg::RayPlaneIntersect(currRay, plane, poi, n, t))
                        {
                            Eigen::Vector3d offset = poi - point;
                            activeEntity->Translate(Linalg::Eigen2GLM(offset) * activeEntity->ndcRatio);
                        }
                    }
                }
            }
            // update mouse position
            prev_x = mouse_x;
            prev_y = mouse_y;
        }
    }

    // draw scene
    scene->Render(GetShaderFactory(), GetCamera());
}

void SceneBuilder::SetDownClick(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
    prev_x = mouse_x;
    prev_y = mouse_y;
    clicked = true;
}

void SceneBuilder::SetDownClickPosition(int x, int y)
{
    if (holding)
    {
        mouse_x = x;
        mouse_y = y;
    }
}

void SceneBuilder::SetReleaseClick()
{
    if (holding) holding = false;
}

bool SceneBuilder::GetMouseClicked()
{
    return clicked;
}

glm::vec3 SceneBuilder::GetMouseLocation()
{
    return glm::vec3((2.0 * mouse_x) / GetWidth() - 1.0, 1.0 - (2.0 * mouse_y) / GetHeight(), 1.0);
}

void SceneBuilder::SetHandledClick(bool handled)
{
    if (handled) clicked = false;
}

void SceneBuilder::KeyCallback(int key, int scancode, int action, int mods)
{
    bool val = action == GLFW_REPEAT || action == GLFW_PRESS;
    if (key == GLFW_KEY_LEFT_SHIFT)
    {
        scaling = val;
    }
    if (key == GLFW_KEY_LEFT_CONTROL)
    {
        rotating = val;
    }
    if (key == GLFW_KEY_Q)
    {
        const auto& entities = sceneDocument.GetEntities();
        if (action == GLFW_PRESS)
            for (const auto& entity : entities) entity->da.wireframe = true;
        else if (action == GLFW_RELEASE) 
            for (const auto& entity : entities) entity->da.wireframe = false;
    }
}
