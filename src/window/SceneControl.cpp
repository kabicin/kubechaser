#include "window/SceneControl.h"

SceneControl::SceneControl(int x, int y, int width, int height)
    : BaseWindow(x, y, width, height)
{
}

SceneControl::~SceneControl()
{
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
        ImGui::SetNextWindowPos(ImVec2(GetX(), GetHeight()));
        ImGui::SetNextWindowSize(ImVec2(GetWidth(), GetHeight()));
        ImGui::Begin("Scene Control", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Scene World");
        ImGui::PopStyleColor();
        if (ImGui::Button("Open"))
        {
            changeWindow(1);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Mesh Viewer");
        ImGui::PopStyleColor();
        if (ImGui::Button("Open"))
        {
            changeWindow(2);
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
        bool addOpen = true;
        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        if (ImGui::BeginPopupModal("Add Object", &addOpen))
        {
            ImGui::PopStyleColor();
            ImGui::Text("Select an asset to add to the scene.");
            const char* items[] = { "Triangle", "Quad" };
            static const char* shape = NULL;
            ImGui::Text("Shape: ");
            ImGui::SameLine();
            if (ImGui::BeginCombo("##combo", shape))
            {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                {
                    bool is_selected = (shape == items[n]);
                    if (ImGui::Selectable(items[n], is_selected))
                    {
                        shape = items[n];
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Add") && shape != NULL)
            {
                SceneBuilder::AddEntity(shape, glm::vec3(0.0f, 0.5f, 0.0f));
                logger->LogMessage("Added " + std::string(shape));
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

        std::vector<std::shared_ptr<Entity>> entities = SceneBuilder::GetAllEntities();
        for (int i = 0; i < SceneBuilder::GetNumEntities(); i++)
        {
            if (ImGui::Button(std::string("Entity " + std::to_string(entities[i]->GetId())).c_str()))
            {
                SceneBuilder::SetActiveEntity(i);
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        ImGui::PushStyleColor(ImGuiCol_Text, imColorWhite);
        ImGui::Text("Scene Settings");
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, imColorGrey);
        ImGui::Checkbox("Grid", &SceneBuilder::showGrid);
        ImGui::PopStyleColor();

        ImGui::End();

        
        // Scene Statistics
        ImGui::SetNextWindowPos(ImVec2(200, 0));
        ImGui::SetNextWindowSize(ImVec2(200, 50));
        ImGui::Begin("Scene Statistics", NULL, 0);
        ImGui::Text("Active Objects:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(SceneBuilder::GetNumEntities()).c_str());
        ImGui::End();

        // Scene Object
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImVec2(200, 300));
        std::shared_ptr<Entity> entity = SceneBuilder::GetActiveEntity();
        if (entity != NULL)
        {
            ImGui::Begin(std::string("Active Object: [Object " + std::to_string(entity->GetId()) + "]").c_str(), NULL, 0);
            ImGui::Text("Type:");
            ImGui::SameLine();
            ImGui::Text("%s", entity->GetName().c_str());

            ImGui::Text("Texture:");
            ImGui::SameLine();
            ImGui::Text("%s", entity->GetTextureName().c_str());

            if (ImGui::Button("Delete"))                    SceneBuilder::DeleteEntity(entity);
	        if (ImGui::Button("Move Front"))                entity->MoveUp(glm::vec3(0.0f, 0.0f, SceneBuilder::GetMaxDepth()));
            ImGui::SameLine();
            if (ImGui::Button("Move Back"))                 entity->MoveBack(glm::vec3(0.0f, 0.0f, SceneBuilder::GetMinDepth()));
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

void SceneControl::changeWindow(int window)
{
    windowMode = window;
    propagateControl = true;
}

bool SceneControl::HasChanged()
{
    bool prev = propagateControl;
    propagateControl = false;
    return prev;
}

int SceneControl::GetWindowMode()
{
    return windowMode;
}