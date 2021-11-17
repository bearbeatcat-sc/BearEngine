#ifndef _LIGHT_MANAGER_H_
#define _LIGHT_MANAGER_H_

#include "../../Device/Singleton.h"
#include <map>
#include <vector>
#include <memory>

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
	void Draw();
	void SetDebugMode(bool flag);
	std::shared_ptr<PointLight> GetPointLights(int index);
	std::shared_ptr<DirectionalLight> GetDirectionalLight();

protected:
	LightManager();
	~LightManager();

private:
	void CreatePointLightResource();

private:
	std::vector<std::shared_ptr<PointLight>> m_PointLights;
	std::shared_ptr<DirectionalLight> m_DirectionalLight;
	bool m_IsDebugMode;
	std::shared_ptr<Buffer> _PointLightsResource;

	const int _MaxPointLightCount = 20;
};

#endif