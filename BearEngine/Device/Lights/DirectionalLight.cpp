#include "DirectionalLight.h"

#include "../../Device/DirectX/Core/Buffer.h"


DirectionalLight::DirectionalLight(const SimpleMath::Vector3& direction, const SimpleMath::Color& color)
	:_isUpdate(true),_direction(direction),_color(color)
{
	GenerateLightBuffer();
}

DirectionalLight::~DirectionalLight()
{
}

std::shared_ptr<Buffer> DirectionalLight::GetBuffer()
{
	// 更新前のデータなら更新する
	if (_isUpdate)
	{
		UpdateLightBuffer();
	}

	return m_LightDataBuffer;
}

void DirectionalLight::UpdateDirectionalLight(const SimpleMath::Vector3& direction,const SimpleMath::Color& color)
{
	_direction = direction;
	_color = color;

	_isUpdate = true;
}

const SimpleMath::Vector3& DirectionalLight::GetDirection()
{
	return _direction;
}

bool DirectionalLight::GenerateLightBuffer()
{
	m_LightDataBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(ConstLightDatas) + 0xff) & ~0xff;
	m_LightDataBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto constBuff = m_LightDataBuffer->getBuffer();

	ConstLightDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->LightColor = _color;
	constMap->LightDir = _direction;
	constBuff->Unmap(0, nullptr);


	return true;
}

bool DirectionalLight::UpdateLightBuffer()
{
	auto constBuff = m_LightDataBuffer->getBuffer();

	ConstLightDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->LightColor = _color;
	constMap->LightDir = _direction;
	constBuff->Unmap(0, nullptr);
	
	_isUpdate = false;

	return true;
}

