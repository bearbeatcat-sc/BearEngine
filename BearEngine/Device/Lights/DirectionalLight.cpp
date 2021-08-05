#include "DirectionalLight.h"

#include "../../Device/DirectX/Core/Buffer.h"

DirectionalLight::DirectionalLight()
{
	GenerateLightBuffer();
}

DirectionalLight::~DirectionalLight()
{
}

std::shared_ptr<Buffer> DirectionalLight::GetBuffer()
{
	// 更新前のデータなら更新する
	if (m_DirtyFlag)
	{
		UpdateLightBuffer();
	}

	return m_LightDataBuffer;
}

bool DirectionalLight::GenerateLightBuffer()
{
	m_LightDataBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(ConstLightDatas) + 0xff) & ~0xff;
	m_LightDataBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto constBuff = m_LightDataBuffer->getBuffer();

	ConstLightDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->LightColor = XMFLOAT3(m_LightColor.R(), m_LightColor.G(), m_LightColor.B());
	constMap->LightDir = m_Direction;
	constBuff->Unmap(0, nullptr);


	return true;
}

bool DirectionalLight::UpdateLightBuffer()
{
	auto constBuff = m_LightDataBuffer->getBuffer();
	ConstLightDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->LightColor = XMFLOAT3(m_LightColor.R(), m_LightColor.G(), m_LightColor.B());
	constMap->LightDir = m_Direction;
	constBuff->Unmap(0, nullptr);

	m_DirtyFlag = false;

	return true;
}

