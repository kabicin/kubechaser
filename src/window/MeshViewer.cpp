#include "window/MeshViewer.h"
#include "logger/Logger.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>

MeshViewer::MeshViewer(const WindowServices& services, int x, int y, int width, int height)
    : BaseWindow(services, x, y, width, height)
{
    camera = std::make_shared<Camera>(width, height);
    // No mesh until SetMesh is called; the viewer draws nothing until then.
}

void MeshViewer::handleInput()
{
    if (mesh == nullptr) return;

     if (GetMouseClicked()) {
        onClick();
    }

    if (holding)
    {
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
    if (mesh == nullptr) return;

    {
        float posX = std::max(static_cast<float>(GetX()), static_cast<float>(GetX()) + (GetWidth() - 720.0f) * 0.5f);
        float posY = static_cast<float>(GetY() + GetHeight() - 210);
        ImGui::SetNextWindowPos(ImVec2(posX, posY));
        ImGui::SetNextWindowSize(ImVec2(720, 210));
        ImGui::Begin("Mesh Viewer", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // checkboxes
        ImGui::Checkbox("wireframe", &wireframe);
        ImGui::SameLine();
        ImGui::Checkbox("tesselate", &tesselate);
        ImGui::SameLine();
        ImGui::Checkbox("normal", &normal);
        ImGui::SameLine();
        {
            // Only meshes that actually have a texture bound can be textured.
            const bool hasTexture = mesh->GetTexture() != 0;
            if (!hasTexture) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            ImGui::Checkbox("texture", &texture);
            if (!hasTexture) ImGui::PopStyleVar();
        }
        ImGui::SameLine();
        ImGui::Checkbox("lighting", &lighting);
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

        if (ImGui::Button("Print State")) {
            std::cout << mesh->da << std::endl;
        }
        if (ImGui::Button("Save State")) {
            saveState();
        }
        if (ImGui::Button("Load State")) {
            loadState();
        }
        ImGui::End();

        // apply configs
        mesh->da.wireframe = wireframe;
        mesh->da.tesselate = tesselate;
        mesh->da.normal = normal;
        mesh->da.texture = texture && mesh->GetTexture() != 0;
        mesh->da.color = !lighting;
        mesh->da.solidColor = glm::vec3(main_rgb[0], main_rgb[1], main_rgb[2]);
        mesh->da.materialAttributes.diffuse = mesh->da.solidColor;
        mesh->da.useVertexColor = false;
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
        double dt = GetTime()->GetDelta();
        double clamped_dt = std::min(dt, 1.0 / 15.0);
        double step = delta * scroll_speed * clamped_dt;
        mesh->Translate(glm::vec3(0.0f, 0.0f, static_cast<float>(step)));
    }
    showGUI();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (mesh != nullptr) mesh->Draw(this->GetShaderFactory(), camera);
    glDisable(GL_DEPTH_TEST);
}

void MeshViewer::SetMesh(std::shared_ptr<Mesh> mesh)
{
    this->mesh = std::move(mesh);
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
    if (mesh != nullptr && mesh->Intersect(camera, ray, temp_t)) {
        min_t = std::min(min_t, temp_t);
        mesh->SetObjectPicked();
        Eigen::Vector3d newray(ray.eye + ray.direction * min_t);
        glm::vec3 gnewray = glm::vec3(newray(0), newray(1), newray(2));
        SetClickOffset(gnewray - mesh->GetTranslate());
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
        // Toggle the same state the "wireframe" checkbox edits; showGUI()
        // copies it into mesh->da every frame.
        if (action == GLFW_PRESS)
        {
            wireframe = !wireframe;
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

void MeshViewer::OnCursorPosition(double xpos, double ypos)
{
    SetDownClickPosition(static_cast<int>(xpos), static_cast<int>(ypos));
}

void MeshViewer::OnMouseButton(int button, int action, double xpos, double ypos)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            SetDownClick(static_cast<int>(xpos), static_cast<int>(ypos));
        }
        else
        {
            SetReleaseClick();
        }
    }
}

void MeshViewer::OnKey(int key, int action)
{
    KeyCallback(action, key);
}

void MeshViewer::OnDeactivate()
{
    SetReleaseClick();
}

namespace {

std::filesystem::path meshStatePath()
{
    return std::filesystem::path(OUTPUT_DIR) / "mesh.txt";
}

} // namespace

void MeshViewer::saveState()
{
    const std::filesystem::path path = meshStatePath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream meshOut(path);
    if (!meshOut) {
        GetLogger()->LogMessage("Save State failed: could not open " + path.string());
        return;
    }

    mesh->da.serialize(meshOut);
    // The viewer's own toggles: lightSwitch is 0 whenever lighting is off,
    // so the individual light checkboxes can't be recovered from it.
    int light_mask = 0;
    if (dir_light) light_mask |= 1;
    if (point_light) light_mask |= 1 << 1;
    if (spot_light) light_mask |= 1 << 2;
    meshOut << "viewer.lighting " << lighting << '\n'
            << "viewer.lights " << light_mask << '\n'
            << "viewer.blinn " << blinn << '\n';

    if (!meshOut) {
        GetLogger()->LogMessage("Save State failed: could not write " + path.string());
        return;
    }
    GetLogger()->LogMessage("Saved mesh state to " + path.string());
}

void MeshViewer::loadState()
{
    const std::filesystem::path path = meshStatePath();
    std::ifstream meshIn(path);
    if (!meshIn) {
        GetLogger()->LogMessage("Load State failed: could not open " + path.string());
        return;
    }
    std::stringstream contents;
    contents << meshIn.rdbuf();

    DrawAttributes loaded = DrawAttributes::deserialize(contents);
    std::cout << loaded << std::endl;
    mesh->da = loaded;

    wireframe = loaded.wireframe;
    tesselate = loaded.tesselate;
    normal = loaded.normal;
    texture = loaded.texture;
    main_rgb[0] = loaded.solidColor.x;
    main_rgb[1] = loaded.solidColor.y;
    main_rgb[2] = loaded.solidColor.z;

    // Defaults for files without viewer lines (fall back to DrawAttributes).
    lighting = !loaded.color;
    int light_mask = static_cast<int>(loaded.lightSwitch);
    blinn = loaded.lightBlinn;

    contents.clear();
    contents.seekg(0);
    std::string line;
    while (std::getline(contents, line)) {
        std::istringstream ls(line);
        std::string key;
        int value;
        if (!(ls >> key >> value)) continue;
        if (key == "viewer.lighting") lighting = value != 0;
        else if (key == "viewer.lights") light_mask = value;
        else if (key == "viewer.blinn") blinn = value != 0;
    }

    dir_light = (light_mask & 1) != 0;
    point_light = (light_mask & (1 << 1)) != 0;
    spot_light = (light_mask & (1 << 2)) != 0;
    GetLogger()->LogMessage("Loaded mesh state from " + path.string());
}

void MeshViewer::SaveMeshJSON()
{
    std::cout << "Saving mesh.." << std::endl;

}
