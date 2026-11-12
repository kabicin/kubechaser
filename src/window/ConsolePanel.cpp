#include "window/ConsolePanel.h"

ConsolePanel::ConsolePanel(const WindowServices& services, int x, int y, int width, int height)
	: BaseWindow(services, x, y, width, height)
{
}

ConsolePanel::~ConsolePanel()
{
}

void ConsolePanel::Render()
{
	//ClearBackground(0.19f, 0.28f, 0.37f, 1.0f);
    glViewport(GetX(), GetY(), GetWidth(), GetHeight());
    {
        ImGui::SetNextWindowPos(ImVec2(GetX(), GetY()));
        ImGui::SetNextWindowSize(ImVec2(GetWidth(), GetHeight()));
        ImGui::Begin("Console", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        
        // read console buffer here
        // TextUnformatted has no length limit, unlike Text which formats
        // into a fixed-size buffer and truncates long logs.
        const std::string& output = GetLogger()->Output();
        ImGui::TextUnformatted(output.c_str(), output.c_str() + output.size());

        // if there is a new log event, set the scrollbar to the bottom
        if (GetLogger()->GetState() != logState) {
            logState = GetLogger()->GetState();
            ImGui::SetScrollHere(1.0f);
        }
        
        ImGui::End();
    }
}

