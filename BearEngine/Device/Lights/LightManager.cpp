#include "LightManager.h"
#include "../../imgui/imgui.h"
#include "DirectionalLight.h"

LightManager::LightManager()
	:m_IsDebugMode(true)
{
}

LightManager::~LightManager()
{
	Shutdown();
}

void LightManager::Init()
{
	m_SpotLights.clear();

	m_DirectionalLight = std::make_shared<DirectionalLight>();
}

void LightManager::Shutdown()
{
	m_SpotLights.clear();
}

void LightManager::SetDirectionalLight(std::shared_ptr<DirectionalLight> light)
{
	m_DirectionalLight = light;
}

void LightManager::AddSpotLight(std::shared_ptr<Light> light)
{
	m_SpotLights.push_back(light);
}

void LightManager::Draw()
{
	if (m_IsDebugMode)
	{
		auto dir = m_DirectionalLight->GetDirection();

		if (ImGui::BeginTabItem("LightProperties"))
		{
			ImGui::DragFloat("LightDirection X", &dir.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("LightDirection Y", &dir.y, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("LightDirection Z", &dir.z, 0.01f, -1.0f, 1.0f);
			ImGui::EndTabItem();
			m_DirectionalLight->SetDirection(dir);
		}


	}
}

void LightManager::SetDebugMode(bool flag)
{
	m_IsDebugMode = flag;
}

std::shared_ptr<Light> LightManager::GetSpotLights(int index)
{
	return m_SpotLights.at(index);
}

std::shared_ptr<DirectionalLight> LightManager::GetDirectionalLight()
{
	return m_DirectionalLight;
}

