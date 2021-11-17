#ifndef _DIRECTIONAL_LIGHT_H_
#define _DIRECTIONAL_LIGHT_H_

#include <memory>
#include <SimpleMath.h>

using namespace DirectX;

class Buffer;

class DirectionalLight
{
public:
	DirectionalLight(const SimpleMath::Vector3& direction,const SimpleMath::Color& color);
	~DirectionalLight();
	std::shared_ptr<Buffer> GetBuffer();
	void UpdateDirectionalLight(const SimpleMath::Vector3& direction, const SimpleMath::Color& color);
	const SimpleMath::Vector3& GetDirection();

	struct ConstLightDatas
	{
		SimpleMath::Color LightColor;
		SimpleMath::Vector3 LightDir;
	};

private:
	bool GenerateLightBuffer();
	bool UpdateLightBuffer();

private:
	std::shared_ptr<Buffer> m_LightDataBuffer;


	SimpleMath::Vector3 _direction;
	SimpleMath::Color _color;

	bool _isUpdate;
};

#endif