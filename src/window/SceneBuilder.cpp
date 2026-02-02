#include "window/SceneBuilder.h"
#include "utility/Reader.h"


SceneBuilder::SceneBuilder(int x, int y, int width, int height)
    : BaseWindow(x, y, width, height)
{
    std::cout << "Scene Builder: " << x << " " << y << " " << width << " " << height << std::endl;
    this->SetCamera(std::make_shared<Camera>(width, height));
    
    std::shared_ptr<Entity> mesh = AddEntity("Mesh", "statefulset.obj");
    mesh->Scale(glm::vec3(0.005, 0.005, 0.005));
    mesh->Translate(glm::vec3(0, -90, 250));

    // create scene node root
    std::shared_ptr<SceneNode> root = std::make_shared<SceneNode>();
    for (std::shared_ptr<Entity> entity : entities) {
        root->AddDynamicObject(entity);
    }

    // repeat for each node on boundary to test..
    std::array<std::shared_ptr<SceneNode>, 8> boundary;
    for (int i = 0; i < 8; i++) {
        std::shared_ptr<SceneNode> node = std::make_shared<SceneNode>();
        for (std::shared_ptr<Entity> entity : entities) {
            node->AddDynamicObject(entity);
        }
        boundary[i] = node;
    }
    root->SetBoundary(boundary);

    // initialize scene based on root
    scene = std::make_shared<Scene>(root);
}

SceneBuilder::~SceneBuilder()
{
    for (int i = 0; i < entities.size(); i++)
    {
        entities[i] = nullptr;
    }
    entities.clear();
}

void SceneBuilder::Resize(int x, int y, int width, int height)
{
    GetCamera()->Resize(width, height);
    BaseWindow::Resize(x, y, width, height);
}

std::vector<std::shared_ptr<Entity>> SceneBuilder::GetAllEntities()
{
    return entities;
}

size_t SceneBuilder::GetNumEntities()
{
    return entities.size();
}

void SceneBuilder::onClick(int x, int y)
{
    // Picking code
    Ray ray = GetCamera()->GetRay(x, y);
    double min_t = std::numeric_limits<double>::infinity(), temp_t;
    int opt_i = -1;
    int n = entities.size();
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
        SetActiveEntity(opt_i);
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

double SceneBuilder::GetMaxDepth()
{
    double temp = 0.0;
    for (int i = 0; i < GetNumEntities(); i++)
    {
        if (entities[i]->translate.z >= temp)
        {
            temp = entities[i]->translate.z + 0.01;
        }
    }
    maxDepth = temp;
    return maxDepth;
}

double SceneBuilder::GetMinDepth()
{
    double temp = 0.0;
    for (int i = 0; i < GetNumEntities(); i++)
    {
        if (entities[i]->translate.z <= temp)
            temp = entities[i]->translate.z - 0.01;
    }
    minDepth = temp;
    return minDepth;
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
    if (SceneBuilder::holding)
    {
        // handle mouse offset
        int xOffset = mouse_x - prev_x;
        int yOffset = prev_y - mouse_y; // flip because of GLFW coords 
        // has it moved...
        if (xOffset != 0 || yOffset != 0)
        {
            // if shift button pressed - scale
            if (SceneBuilder::scaling)
            {
                double scaleFactor = 0.05;
                if (xOffset > 0) entities[activeEntityIndex]->Scale(1.0 + scaleFactor);
                else entities[activeEntityIndex]->Scale(1.0 - scaleFactor);
            }
            // if ctrl button pressed - rotate
            else if (SceneBuilder::rotating)
            {
                float rotateFactor = 4.0f;
                if (xOffset > 0) entities[activeEntityIndex]->RotateZInc(rotateFactor);
                else entities[activeEntityIndex]->RotateZInc(-rotateFactor);
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
                        std::shared_ptr<Entity> currEntity = this->entities[activeEntityIndex];
                        this->entities[activeEntityIndex]->Translate(Linalg::Eigen2GLM(offset) * currEntity->ndcRatio);
                    }
                }
            }
        }
        // update mouse position
        prev_x = mouse_x;
        prev_y = mouse_y;
    }

    // draw scene
    scene->SetShowGrid(SceneBuilder::showGrid, GetCamera());
    scene->Render(GetShaderFactory(), GetCamera());
}

std::shared_ptr<Entity> SceneBuilder::AddEntity(const std::string& className, glm::vec3 offset)
{
    std::shared_ptr<Entity> entity;
    if (className == "Triangle")
        entity = std::make_shared<Triangle>();
    else if (className == "Quad")
        entity = std::make_shared<Quad>();
    else
        return nullptr;
    return AddEntity(entity, offset);
}

std::shared_ptr<Entity> SceneBuilder::AddEntity(const std::string& className, const std::string& objectFile, glm::vec3 offset)
{
    if (className != "Mesh") return nullptr;
    std::shared_ptr<Entity> entity = std::make_shared<Mesh>(objectFile);
    return AddEntity(entity, offset);
}

std::shared_ptr<Entity> SceneBuilder::AddEntity(
    const std::string& className, 
    const std::string& objectFile, 
    const std::string& textureFile, 
    glm::vec3 offset)
{
    if (className != "Mesh") return nullptr;
    std::shared_ptr<Entity> entity = std::make_shared<Mesh>(objectFile, "", textureFile);
    return AddEntity(entity, offset);
}

std::shared_ptr<Entity> SceneBuilder::AddEntity(std::shared_ptr<Entity> entity, glm::vec3 offset)
{
    entity->Translate(offset);
    entities.push_back(entity);
    SetActiveEntity(static_cast<int>(entities.size()) - 1);
    return entity;
}

void SceneBuilder::DeleteEntity(int index)
{
    if (index < entities.size())
    {
        entities[index] = nullptr;
        entities.erase(entities.begin() + index);
    }
}

void SceneBuilder::DeleteEntity(std::shared_ptr<Entity> entity)
{
    for (int i = 0; i < entities.size(); i++)
    {
        if (entities[i] == entity)
        {
            entities[i] = nullptr;
            entities.erase(entities.begin() + i);
            if (entities.size() > 0)
            {
                SetActiveEntity(static_cast<int>(entities.size()) - 1);
            }
            else
            {
                SetActiveEntity(-1);
            }
            return;
        }
    }
}

void SceneBuilder::SetActiveEntity(int index)
{
    activeEntityIndex = index;
}

std::shared_ptr<Entity> SceneBuilder::GetActiveEntity()
{
    if (activeEntityIndex >= 0 && activeEntityIndex < entities.size())
        return entities[activeEntityIndex];
    return nullptr;
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
        if (action == GLFW_PRESS)
            for (int i = 0; i < entities.size(); i++) entities[i]->da.wireframe = true;
        else if (action == GLFW_RELEASE) 
            for (int i = 0; i < entities.size(); i++) entities[i]->da.wireframe = false;
    }
}

std::vector<std::shared_ptr<Entity>> SceneBuilder::entities;
size_t SceneBuilder::activeEntityIndex;
int SceneBuilder::mouse_x, SceneBuilder::mouse_y, SceneBuilder::prev_x, SceneBuilder::prev_y;
bool SceneBuilder::clicked, SceneBuilder::holding, SceneBuilder::scaling, SceneBuilder::rotating, SceneBuilder::wireframe;
glm::vec3 SceneBuilder::prev, SceneBuilder::next;
double SceneBuilder::maxDepth = 0.0, SceneBuilder::minDepth = 0.0;

bool SceneBuilder::showGrid;
