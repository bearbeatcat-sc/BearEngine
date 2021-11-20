#include "GUISystem.h"

#include "WindowApp.h"

void GUISystem::BeginGUI()
{
	auto windowSize = WindowApp::GetInstance().GetWindowSize();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowSize.window_Width, 30), ImGuiCond_Always);
	ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
	DrawMenuBar();
	EndGUI();
}

void GUISystem::DrawGUI()
{
}

void GUISystem::EndGUI()
{
	ImGui::End();
}

void GUISystem::DrawMenuBar()
{

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File"))
		{
			//if (ImGui::MenuItem("Save")) {

			//}
			//if (ImGui::MenuItem("Load")) {

			//}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

}
