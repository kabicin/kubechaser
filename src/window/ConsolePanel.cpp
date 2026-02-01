#include "window/ConsolePanel.h"

ConsolePanel::ConsolePanel(int x, int y, int width, int height)
	: BaseWindow(x, y, width, height)
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
        ImGui::SetNextWindowPos(ImVec2(GetX(), GetHeight()));
        ImGui::SetNextWindowSize(ImVec2(GetWidth(), GetHeight()));
        ImGui::Begin("Console", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        
        // read console buffer here
        ImGui::Text("%s", logger->Output().c_str());

        // if there is a new log event, set the scrollbar to the bottom
        if (logger->GetState() != logState) {
            logState = logger->GetState();
            ImGui::SetScrollHere(1.0f);
        }
        
        ImGui::End();
    }
}

