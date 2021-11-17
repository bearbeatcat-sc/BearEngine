#include "LightManager.h"

#include "../../imgui/imgui.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Device/DirectX/Core/Buffer.h"

LightManager::LightManager()
	:m_IsDebugMode(true)
{
	CreatePointLightResource();
}

LightManager::~LightManager()
{
	Shutdown();
}

void LightManager::CreatePointLightResource()
{
	auto buffSize = sizeof(PointLight::ConstPointLightDatas) * _MaxPointLightCount;
	
	_PointLightsResource = std::make_shared<Buffer>();
	_PointLightsResource->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void LightManager::Init()
{
	m_PointLights.clear();

	m_DirectionalLight = std::make_shared<DirectionalLight>(SimpleMath::Vector3(1, 0, 0), SimpleMath::Color(1, 1, 1, 1));
}

void LightManager::Shutdown()
{
	m_PointLights.clear();
}

void LightManager::SetDirectionalLight(std::shared_ptr<DirectionalLight> light)
{
	m_DirectionalLight = light;
}

void LightManager::AddPointLight(std::shared_ptr<PointLight> light)
{
	if(m_PointLights.size() >= _MaxPointLightCount)
	{
		throw std::runtime_error("ポイントライトの登録数上限を超えました。");
	}
	m_PointLights.push_back(light);
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
			m_DirectionalLight->UpdateDirectionalLight(dir, SimpleMath::Color(1, 1, 1, 1));
		}


	}
}

void LightManager::SetDebugMode(bool flag)
{
	m_IsDebugMode = flag;
}

std::shared_ptr<PointLight> LightManager::GetPointLights(int index)
{
	return m_PointLights.at(index);
}

std::shared_ptr<DirectionalLight> LightManager::GetDirectionalLight()
{
	return m_DirectionalLight;
}

