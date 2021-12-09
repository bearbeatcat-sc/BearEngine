#ifndef _LIGHT_MANAGER_H_
#define _LIGHT_MANAGER_H_

#include <vector>
#include <memory>
#include <d3d12.h>
#include <SimpleMath.h>

#include "../../Device/Singleton.h"
#include "PointLight.h"
#include "DirectionalLight.h"

using namespace DirectX;

class DirectionalLight;
class PointLight;
class Buffer;

class LightManager
	:public Singleton<LightManager>
{
public:
	friend class Singleton<LightManager>;
	void Init();
	void Shutdown();
	void SetDirectionalLight(std::shared_ptr<DirectionalLight> light);
	void AddPointLight(std::shared_ptr<PointLight> light);
	void UpdatePointLight(int index, const SimpleMath::Vector3& position, const SimpleMath::Color& color, float distance,
		float decay);
	void Draw();
	void Update();
	void SetDebugMode(bool flag);
	std::shared_ptr<PointLight> GetPointLights(int index);
	std::shared_ptr<DirectionalLight> GetDirectionalLight();
	int GetMaxPointLightCount();
	int GetPointLightCount();
	bool AllocateDescriptor(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);


protected:
	LightManager();
	~LightManager();

private:
	void CreatePointLightResource();
	void UpdatePointLightResource();
	void DestroyPointLight();

private:
	std::vector<std::shared_ptr<PointLight>> m_PointLights;
	std::shared_ptr<DirectionalLight> m_DirectionalLight;
	bool m_IsDebugMode;
	std::shared_ptr<Buffer> _PointLightsResource;

	const int _MaxPointLightCount = 20;
};

#endif