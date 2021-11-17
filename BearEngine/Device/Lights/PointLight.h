#pragma once

#include <SimpleMath.h>

using namespace DirectX;

class Buffer;

class PointLight
{
public:
	PointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance, float decay);
	~PointLight() = default;
	std::shared_ptr<Buffer> GetBuffer();
	void UpdatePointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance, float decay);

	struct ConstPointLightDatas
	{
		SimpleMath::Vector3 position;
		float padding;
		SimpleMath::Color color;
		float distance;
		float decay;
	};

private:
	bool GenerateLightBuffer();
	bool UpdateLightBuffer();

private:
	std::shared_ptr<Buffer> m_LightDataBuffer;

	SimpleMath::Vector3 _position;
	SimpleMath::Color _color;
	float _distance;
	float _decay;

	bool _isUpdate;
};
