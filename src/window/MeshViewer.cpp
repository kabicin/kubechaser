#include "window/MeshViewer.h"
#include <algorithm>

MeshViewer::MeshViewer(int x, int y, int width, int height) 
    : BaseWindow(x, y, width, height)
{
    camera = std::make_shared<Camera>(width, height);

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>("statefulset.obj");
    mesh->Scale(glm::vec3(1, 1, 1));
    mesh->Translate(glm::vec3(0, 0, 0));
    
    MeshViewer::SetMesh(mesh);
}

void MeshViewer::handleInput()
{
     if (GetMouseClicked()) {
        onClick();
    }

    if (MeshViewer::holding)
    {
        // handle mouse offset
        int xOffset = mouse_x - prev_x;
        int yOffset = prev_y - mouse_y; // flip because of GLFW coords 
        // has it moved...
        if (xOffset != 0 || yOffset != 0)
        {
            // if shift button pressed - scale
            if (MeshViewer::scaling)
            {
                double scaleFactor = 0.05;
                if (xOffset > 0)
                {
                    mesh->Scale(1.0 + scaleFactor);
                }
                else
                {
                    mesh->Scale(1.0 - scaleFactor);
                }
            }
            else
            {
                mesh->RotateXInc(yOffset / 5.0);
                mesh->RotateYInc(xOffset);
            }
        }
        // update mouse position
        prev_x = mouse_x;
        prev_y = mouse_y;
    }

}

void MeshViewer::showGUI()
{
    {
        ImGui::SetNextWindowPos(ImVec2(GetWidth() / 2 - 360, GetHeight() - 210));
        ImGui::SetNextWindowSize(ImVec2(720, 210));
        ImGui::Begin("Mesh Viewer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // checkboxes
        static bool wireframe = true;
        ImGui::Checkbox("wireframe", &wireframe);
        ImGui::SameLine();
        static bool tesselate = false;
        ImGui::Checkbox("tesselate", &tesselate);
        ImGui::SameLine();
        static bool normal = false;
        ImGui::Checkbox("normal", &normal);
        ImGui::SameLine();
        static bool texture = true;
        ImGui::SameLine();
        static bool lighting = true;
        ImGui::SameLine();
        ImGui::Checkbox("lighting", &lighting);
        static bool dir_light = true;
        static bool point_light = true;
        static bool spot_light = true;
        static bool blinn = false;
        static float main_rgb[3] = { 0.8f, 0.8f, 0.8f };
        ImGui::ColorEdit3("color", main_rgb);
        if (lighting)
        {
            ImGui::Checkbox("dir", &dir_light);
            ImGui::SameLine();
            ImGui::Checkbox("point", &point_light);
            ImGui::SameLine();
            ImGui::Checkbox("spot", &spot_light);
            ImGui::SameLine();
            ImGui::Checkbox("blinn", &blinn);
        }
        ImGui::Text("solid: %.2f %.2f %.2f", main_rgb[0], main_rgb[1], main_rgb[2]);
        ImGui::Text("diffuse: %.2f %.2f %.2f",
            mesh->da.materialAttributes.diffuse.x,
            mesh->da.materialAttributes.diffuse.y,
            mesh->da.materialAttributes.diffuse.z);
        ImGui::Text("lightSwitch: %u texture: %d",
            mesh->da.lightSwitch,
            mesh->da.texture ? 1 : 0);
        ImGui::End();

        // apply configs
        mesh->da.wireframe = wireframe;
        mesh->da.tesselate = tesselate;
        mesh->da.normal = normal;
        mesh->da.texture = false;
        mesh->da.color = !lighting;
        mesh->da.solidColor = glm::vec3(main_rgb[0], main_rgb[1], main_rgb[2]);
        mesh->da.materialAttributes.diffuse = mesh->da.solidColor;
        mesh->da.useVertexColor = false;
        mesh->da.wireframe = wireframe;
        if (!lighting)
        {
            mesh->da.lightSwitch = 0;
        }
        else
        {
            int light_mask = 0;
            if (dir_light) light_mask |= 1;
            if (point_light) light_mask |= 1 << 1;
            if (spot_light) light_mask |= 1 << 2;
            mesh->da.lightSwitch = light_mask;
        }
        mesh->da.lightBlinn = lighting && blinn;
        
    }
}

void MeshViewer::Render()
{
    ClearBackground(0.3f, 0.3f, 0.3f, 1.0f);
    ImGuiIO& io = ImGui::GetIO();
    int fbWidth = static_cast<int>(GetWidth() * io.DisplayFramebufferScale.x);
    int fbHeight = static_cast<int>(GetHeight() * io.DisplayFramebufferScale.y);
    glViewport(0, 0, fbWidth, fbHeight);
    camera->Resize(fbWidth, fbHeight);
    handleInput();
    if (mesh != nullptr && scroll_offset != 0.0)
    {
        double delta = scroll_offset;
        scroll_offset = 0.0;
        double dt = GetTime()->GetLastDelta();
        double clamped_dt = std::min(dt, 1.0 / 15.0);
        double step = delta * scroll_speed * clamped_dt;
        mesh->Translate(glm::vec3(0.0f, 0.0f, static_cast<float>(step)));
    }
    showGUI();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mesh->Draw(this->GetShaderFactory(), camera);
    glDisable(GL_DEPTH_TEST);
}

void MeshViewer::SetMesh(std::shared_ptr<Mesh> mesh)
{
    MeshViewer::mesh = mesh;
}


void MeshViewer::onClick()
{
    // Picking code
    onClick(mouse_x, mouse_y);
}

void MeshViewer::onClick(int x, int y)
{
    // Picking code
    ImGuiIO& io = ImGui::GetIO();
    double scaledX = x * io.DisplayFramebufferScale.x;
    double scaledY = y * io.DisplayFramebufferScale.y;
    Ray ray = camera->GetRay(scaledX, scaledY);
    double min_t = std::numeric_limits<double>::infinity(), temp_t;
    if (mesh->Intersect(camera, ray, temp_t)) {
        min_t = std::min(min_t, temp_t);
        mesh->SetObjectPicked();
        Eigen::Vector3d newray(ray.eye + ray.direction * min_t);
        glm::vec3 gnewray = glm::vec3(newray(0), newray(1), newray(2));
        SetClickOffset(gnewray - mesh->translate);
        SetLastT(min_t);
        holding = true;
    }
    SetHandledClick(true);
}

void MeshViewer::KeyCallback(int action, int key)
{
    bool val = action == GLFW_REPEAT || action == GLFW_PRESS;
    if (key == GLFW_KEY_LEFT_SHIFT)
    {
        scaling = val;
    }
    else if (key == GLFW_KEY_LEFT_CONTROL)
    {
        rotating = val;
    }
    else if (key == GLFW_KEY_Q)
    {
        if (action == GLFW_PRESS)
        {
            mesh->da.wireframe = true;
        }
        else if (action == GLFW_RELEASE)
        {
            mesh->da.wireframe = false;
        }
    }
}

// click stuff
bool MeshViewer::GetMouseClicked()
{
    return clicked;
}

glm::vec3 MeshViewer::GetMouseLocation()
{
    ImGuiIO& io = ImGui::GetIO();
    double scaledX = mouse_x * io.DisplayFramebufferScale.x;
    double scaledY = mouse_y * io.DisplayFramebufferScale.y;
    double fbWidth = GetWidth() * io.DisplayFramebufferScale.x;
    double fbHeight = GetHeight() * io.DisplayFramebufferScale.y;
    return glm::vec3((2.0 * scaledX) / fbWidth - 1.0, 1.0 - (2.0 * scaledY) / fbHeight, 1.0);
}

void MeshViewer::SetDownClick(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
    prev_x = mouse_x;
    prev_y = mouse_y;
    clicked = true;
}

void MeshViewer::SetDownClickPosition(int x, int y)
{
    if (holding)
    {
        mouse_x = x;
        mouse_y = y;
    }
}

void MeshViewer::SetReleaseClick()
{
    if (holding) holding = false;
}

void MeshViewer::SetHandledClick(bool handled)
{
    if (handled) {
        clicked = false;
    }
}

void MeshViewer::SetClickOffset(glm::vec3 offset)
{
    clickOffset = offset;
}

void MeshViewer::SetLastT(double last)
{
    this->lastT = last;
}

void MeshViewer::OnScroll(double yOffset)
{
    scroll_offset += yOffset;
}

void MeshViewer::SaveMeshJSON()
{
    std::cout << "Saving mesh.." << std::endl;

}

std::shared_ptr<Mesh> MeshViewer::mesh;
int MeshViewer::mouse_x, MeshViewer::mouse_y, MeshViewer::prev_x, MeshViewer::prev_y;
bool MeshViewer::clicked, MeshViewer::holding, MeshViewer::scaling, MeshViewer::rotating, MeshViewer::wireframe;
double MeshViewer::scroll_offset = 0.0;
float MeshViewer::scroll_speed = 3.0f;
