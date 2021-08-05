#include "LogSystem.h"

#include "../imgui/imgui.h"
#include "../Utility/Time.h"

std::vector<std::string> LogSystem::m_Logs;

LogSystem::LogSystem()
{
}

LogSystem::~LogSystem()
{
}

void LogSystem::Init()
{
	m_Logs.clear();
}

void LogSystem::AddLog(const std::string & text)
{
	std::string time = std::to_string(Time::GetNow());
	std::string message = time + ":" + text;

	m_Logs.push_back(message);
}

void LogSystem::DrawLog()
{
#ifdef _DEBUG

	if (ImGui::BeginTabItem("OutputLog"))
	{
		ImGui::BeginChild("Scrolling");
		int size = m_Logs.size();

		for (int i = size - 1; i >= 0; --i)
		{
			ImGui::Text(m_Logs[i].c_str());
		}
		ImGui::EndChild();
		ImGui::EndTabItem();
	}


#endif
}
