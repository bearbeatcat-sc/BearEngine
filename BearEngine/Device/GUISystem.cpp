#include "GUISystem.h"

#include "WindowApp.h"
#include "Components/Collsions/CollisionManager.h"
#include "DirectX/Core/Model/MeshDrawer.h"
#include "Game_Object/ActorManager.h"
#include "Lights/LightManager.h"
#include "Raytracing/DXRPipeLine.h"
#include "Utility/LogSystem.h"
#include "Utility/Time.h"

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

void GUISystem::RenderRenderingDebugWindow()
{
	if(_isShowDebugRenderingWindow)
	{
		ImGui::Begin("Rendering_System", &_isShowDebugRenderingWindow, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);

		ImGui::Text("Application Properties");
		ImGui::Text("FPS:%i", Time::FPS);
		ImGui::Text("Delta Time:%f", Time::DeltaTime);
		ImGui::Text("Elapsed Time:%f", Time::ElapsedTime);

		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, 30));

		ImGui::BeginTabBar("DebugTabs");

		CollisionManager::GetInstance().Draw();
		DirectXDevice::GetInstance().RenderDebug();
		MeshDrawer::GetInstance().DrawDebug();
		LightManager::GetInstance().Draw();
		DXRPipeLine::GetInstance().DrawDebugGUI();
		ImGui::EndTabBar();
		ImGui::End();
	}


}

void GUISystem::DrawDebug()
{

	RenderRenderingDebugWindow();	
	ActorManager::GetInstance().RenderDebug();
	
	if (_isShowLog)
	{
		ImGui::Begin("Log",&_isShowLog);
		LogSystem::DrawLog();
		ImGui::End();
	}

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
			if (ImGui::MenuItem("Rendering","",&_isShowDebugRenderingWindow))
			{
				
			}

			if (ImGui::MenuItem("Log", "", &_isShowLog))
			{

			}
			
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

}
