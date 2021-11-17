#pragma once

#include <SimpleMath.h>

using namespace DirectX;

class Buffer;

class PointLight
{
public:
	PointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance, float decay);
	~PointLight() = default;
	void UpdatePointLight(const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance, float decay);
	
	struct ConstPointLightDatas
	{
		SimpleMath::Vector3 position;
		float padding;
		SimpleMath::Color color;
		float distance;
		float decay;
	};

	const ConstPointLightDatas CreateLightData();
	bool DebugRender();

private:

private:

	SimpleMath::Vector3 _position;
	SimpleMath::Color _color;
	float _distance;
	float _decay;

	bool _isUpdate;
};
