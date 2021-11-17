#include "PointLight.h"

#include "../../Device/DirectX/Core/Buffer.h"

PointLight::PointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance, float decay)
	:_position(position),_color(color),_distance(distance),_decay(decay)
{
}

std::shared_ptr<Buffer> PointLight::GetBuffer()
{
	// 更新前のデータなら更新する
	if (_isUpdate)
	{
		UpdateLightBuffer();
	}

	return m_LightDataBuffer;
}

void PointLight::UpdatePointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance,
	float decay)
{
	_position = position;
	_color = color;
	_distance = distance;
	_decay = decay;
	
	_isUpdate = true;
}

bool PointLight::GenerateLightBuffer()
{
	m_LightDataBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(ConstPointLightDatas) + 0xff) & ~0xff;
	m_LightDataBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto constBuff = m_LightDataBuffer->getBuffer();

	ConstPointLightDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->position = _position;
	constMap->color = _color;
	constMap->distance = _distance;
	constMap->decay = _decay;
	constBuff->Unmap(0, nullptr);

	return true;
}

bool PointLight::UpdateLightBuffer()
{
	auto constBuff = m_LightDataBuffer->getBuffer();
	
	ConstPointLightDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->position = _position;
	constMap->color = _color;
	constMap->distance = _distance;
	constMap->decay = _decay;
	constBuff->Unmap(0, nullptr);

	return true;
}
