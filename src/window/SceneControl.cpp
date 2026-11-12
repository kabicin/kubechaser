#include "window/SceneControl.h"
#include "entity/Mesh.h"
#include "logger/Logger.h"
#include "window/SwitchableBaseWindow.h"
#include <filesystem>

SceneControl::SceneControl(
    const WindowServices& services,
    SceneDocument& sceneDocument,
    int x,
    int y,
    int width,
    int height)
    : BaseWindow(services, x, y, width, height),
      sceneDocument(sceneDocument)
{
}

SceneControl::~SceneControl()
{
}

namespace fs = std::filesystem;
static std::vector<std::string> traverseDirectory() {
    fs::path meshPath = std::string(ASSET_DIR) + "/models/";

    if (!fs::exists(meshPath)) {
        std::cerr << "Mesh path doesn't exist" << std::endl;
        return {};
    }

    std::vector<std::string> files;

    for (const fs::directory_entry& entry : fs::directory_iterator(meshPath)) {
        if (entry.is_regular_file()) {
            std::cout << entry.path().filename().string() << std::endl;
            std::string fileName = entry.path().filename().string();
            files.push_back(fileName);
        }
    }

    return files;
}

void SceneControl::Render()
{
    // ClearBackground(0.19f, 0.28f, 0.37f, 1.0f);
    ClearBackground(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(GetX(), GetY(), GetWidth(), GetHeight());
    
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    {
        // Scene Control
        ImGui::SetNextWindowPos(ImVec2(GetX(), GetY() + GetHeight()));
        ImGui::SetNextWindowSize(ImVec2(GetWidth(), GetHeight()));
        ImGui::Begin("Scene Control", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Scene World");
        ImGui::PopStyleColor();
        if (ImGui::Button("Open##SceneWorld"))
        {
            changeWindow(WindowMode::SceneWorld);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Mesh Viewer");
        ImGui::PopStyleColor();
        if (ImGui::Button("Open##MeshViewer"))
        {
            changeWindow(WindowMode::MeshViewer);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Object");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::Button("New"))
        {
            ImGui::OpenPopup("Add Object");
        }
       
        // Add Modal
        static bool firstFrame = true;
        static std::vector<std::string> items;
        bool addOpen = true;
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        if (ImGui::BeginPopupModal("Add Object", &addOpen))
        {
            ImGui::PopStyleColor();
            ImGui::Text("Select an asset to add to the scene.");

            if (firstFrame) {
                items = traverseDirectory();
                firstFrame = false;
            }
            static std::string obj;
            ImGui::Text("Object: ");
            ImGui::SameLine();
            const char* preview = obj.empty() ? "<select>" : obj.c_str();
            if (ImGui::BeginCombo("##combo", preview))
            {
                for (int n = 0; n < static_cast<int>(items.size()); n++)
                {
                    bool is_selected = (obj == items[n]);
                    if (ImGui::Selectable(items[n].c_str(), is_selected))
                    {
                        obj = items[n];
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Add") && !obj.empty())
            {
                sceneDocument.AddEntity("Mesh", obj, glm::vec3(0.0f, 0.5f, 0.0f));
                GetLogger()->LogMessage("Added " + obj);

                // The viewer gets its own instance (it edits the mesh's draw state
                // and transform every frame); the OBJ itself is parsed once and
                // shared through Mesh's cache, so this only uploads the vertices.
                std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(obj);
                mesh->SetScale(glm::vec3(1, 1, 1));
                mesh->Translate(glm::vec3(0, 0, 0));
                if (meshAddedCallback) {
                    meshAddedCallback(mesh);
                }

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Close")) 
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        } 
        else 
        {
            ImGui::PopStyleColor();
        }

        const auto& entities = sceneDocument.GetEntities();
        for (std::size_t i = 0; i < sceneDocument.GetNumEntities(); ++i)
        {
            if (ImGui::Button(std::string("Entity " + std::to_string(entities[i]->GetId())).c_str()))
            {
                sceneDocument.SetActiveEntity(static_cast<int>(i));
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Scene Settings");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, imColorGrey);
        if (ImGui::Checkbox("Grid", &showGrid)) {
            if (gridToggleCallback) {
                gridToggleCallback(showGrid);
            }
        }
        ImGui::PopStyleColor();

        ImGui::End();

        
        // Scene Statistics
        ImGui::SetNextWindowPos(ImVec2(GetX() + 200, GetY()));
        ImGui::SetNextWindowSize(ImVec2(200, 50));
        ImGui::Begin("Scene Statistics", NULL, 0);
        ImGui::Text("Active Objects:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(sceneDocument.GetNumEntities()).c_str());
        ImGui::End();

        // Scene Object
        ImGui::SetNextWindowPos(ImVec2(GetX(), GetY()));
        ImGui::SetNextWindowSize(ImVec2(200, 300));
        std::shared_ptr<Entity> entity = sceneDocument.GetActiveEntity();
        if (entity != NULL)
        {
            ImGui::Begin(std::string("Active Object: [Object " + std::to_string(entity->GetId()) + "]").c_str(), NULL, 0);
            ImGui::Text("Type:");
            ImGui::SameLine();
            ImGui::Text("%s", entity->GetName().c_str());

            ImGui::Text("Texture:");
            ImGui::SameLine();
            ImGui::Text("%s", entity->GetTextureName().c_str());

            if (ImGui::Button("Delete"))                    sceneDocument.DeleteEntity(entity);
	        if (ImGui::Button("Move Front"))                entity->MoveUp(glm::vec3(0.0f, 0.0f, sceneDocument.GetMaxDepth()));
            ImGui::SameLine();
            if (ImGui::Button("Move Back"))                 entity->MoveBack(glm::vec3(0.0f, 0.0f, sceneDocument.GetMinDepth()));
            if (ImGui::Button("Cast Directional Light"))    entity->da.lightSwitch ^= 1;
            if (ImGui::Button("Cast Point Light"))          entity->da.lightSwitch ^= (1 << 1);
            if (ImGui::Button("Cast Spotlight"))            entity->da.lightSwitch ^= (1 << 2);
            if (ImGui::Button("Cast Blinn"))                entity->da.lightBlinn = !entity->da.lightBlinn; 
            ImGui::End();
        }
        else
        {
            ImGui::Begin("Active Object: [None]", NULL, 0);
            ImGui::Text("No object selected...");
            ImGui::End();
        }
       
    }
}

void SceneControl::SetModeChangeCallback(ModeChangeCallback callback)
{
    modeChangeCallback = std::move(callback);
}

void SceneControl::SetGridToggleCallback(GridToggleCallback callback)
{
    gridToggleCallback = std::move(callback);
}

void SceneControl::SetMeshAddedCallback(MeshAddedCallback callback)
{
    meshAddedCallback = std::move(callback);
}

void SceneControl::SetShowGrid(bool enabled)
{
    showGrid = enabled;
}

bool SceneControl::GetShowGrid() const
{
    return showGrid;
}

void SceneControl::changeWindow(WindowMode window)
{
    windowMode = window;
    if (modeChangeCallback) {
        modeChangeCallback(window);
    }
}

SceneControl::WindowMode SceneControl::GetWindowMode() const
{
    return windowMode;
}
