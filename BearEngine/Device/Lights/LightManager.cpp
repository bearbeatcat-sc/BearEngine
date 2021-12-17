#include "LightManager.h"

#include "../../imgui/imgui.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/Core/Buffer.h"
#include <stdexcept>

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

void LightManager::UpdatePointLightResource()
{
	DestroyPointLight();
	
	std::vector<PointLight::ConstPointLightDatas> constPointLightDatas;

	for (auto light : m_PointLights)
	{
		constPointLightDatas.push_back(light->CreateLightData());
	}

	CD3DX12_RANGE readRange(0, 0);

	void* data;
	_PointLightsResource->getBuffer()->Map(0, &readRange, &data);
	memcpy(data, constPointLightDatas.data(), constPointLightDatas.size() * sizeof(PointLight::ConstPointLightDatas));
	_PointLightsResource->getBuffer()->Unmap(0, nullptr);
}

void LightManager::DestroyPointLight()
{
	for (auto itr = m_PointLights.begin(); itr != m_PointLights.end();)
	{
		if ((*itr)->IsDestroy())
		{
			//delete (*itr);
			itr = m_PointLights.erase(itr);
			continue;
		}

		++itr;
	}
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
	UpdatePointLightResource();
}

void LightManager::UpdatePointLight(int index, const SimpleMath::Vector3& position, const SimpleMath::Color& color,
	float distance, float decay)
{
	if(m_PointLights.size() <= index)
	{
		throw std::runtime_error("登録したポイントライトのIndexを超えています。");
	}

	m_PointLights[index]->UpdatePointLight(position, color, distance, decay);

	UpdatePointLightResource();
}

void LightManager::Draw()
{
	if (m_IsDebugMode)
	{
		bool isUpdate = false;
		auto dir = m_DirectionalLight->GetDirection();
		float dir_[3] = { dir.x,dir.y,dir.z };
		
		if (ImGui::BeginTabItem("LightProperties"))
		{
			ImGui::Text("DirectionalLight");
			ImGui::DragFloat3("LightDirection X", dir_, 0.01f, -1.0f, 1.0f);
			if (ImGui::Button("Add PointLight", ImVec2(30, 30)))
			{
				auto spotLight = std::make_shared<PointLight>(SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1, 1, 1, 1), 10.0f, 1.0f);
				AddPointLight(spotLight);

				isUpdate = true;
			}
			
			ImGui::Text("PointLights");


			for(int i = 0; i < m_PointLights.size(); ++i)
			{
				if(m_PointLights[i]->DebugRender(i))
				{
					isUpdate = true;
				}
			}

			ImGui::EndTabItem();

			auto vec3_dir = SimpleMath::Vector3(dir_);
			vec3_dir.Normalize();
			m_DirectionalLight->UpdateDirectionalLight(vec3_dir, SimpleMath::Color(1, 1, 1, 1));



		}

		if(isUpdate)
		{
			UpdatePointLightResource();
		}
	}
}

void LightManager::Update()
{
	bool isUpdate = false;

	for (int i = 0; i < m_PointLights.size(); ++i)
	{
		if (m_PointLights[i]->_isUpdate)
		{
			isUpdate = true;
		}
	}

	if(isUpdate)
	{
		UpdatePointLightResource();
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

 int LightManager::GetMaxPointLightCount()
{
	return _MaxPointLightCount;
}

int LightManager::GetPointLightCount()
{
	return m_PointLights.size();
}

bool LightManager::AllocateDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC pointLightSRVDesc = {};

	pointLightSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	pointLightSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	pointLightSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	pointLightSRVDesc.Buffer.FirstElement = 0;
	pointLightSRVDesc.Buffer.NumElements = _MaxPointLightCount;
	pointLightSRVDesc.Buffer.StructureByteStride = sizeof(PointLight::ConstPointLightDatas);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(_PointLightsResource->getBuffer(), &pointLightSRVDesc, handle);

	return true;
}

