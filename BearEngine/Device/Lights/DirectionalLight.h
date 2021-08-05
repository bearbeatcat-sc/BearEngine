#ifndef _DIRECTIONAL_LIGHT_H_
#define _DIRECTIONAL_LIGHT_H_

#include "Light.h"
#include <memory>

class Buffer;

class DirectionalLight
	:public Light
{
public:
	DirectionalLight();
	~DirectionalLight();
	std::shared_ptr<Buffer> GetBuffer();

	struct ConstLightDatas
	{
		XMFLOAT3 LightColor;
		XMFLOAT3 LightDir;
	};

private:
	bool GenerateLightBuffer();
	bool UpdateLightBuffer();

private:
	std::shared_ptr<Buffer> m_LightDataBuffer;
};

#endif