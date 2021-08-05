#ifndef _LIGHT_MANAGER_H_
#define _LIGHT_MANAGER_H_

#include "../../Device/Singleton.h"
#include <map>
#include <vector>
#include <memory>

class Light;
class DirectionalLight;

class LightManager
	:public Singleton<LightManager>
{
public:
	friend class Singleton<LightManager>;
	void Init();
	void Shutdown();
	void SetDirectionalLight(std::shared_ptr<DirectionalLight> light);
	void AddSpotLight(std::shared_ptr<Light> light);
	void Draw();
	void SetDebugMode(bool flag);
	std::shared_ptr<Light> GetSpotLights(int index);
	std::shared_ptr<DirectionalLight> GetDirectionalLight();

protected:
	LightManager();
	~LightManager();

private:
	std::vector<std::shared_ptr<Light>> m_SpotLights;
	std::shared_ptr<DirectionalLight> m_DirectionalLight;
	bool m_IsDebugMode;
};

#endif