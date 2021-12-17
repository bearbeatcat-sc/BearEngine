#ifndef _CAMERA_MANAGER_H_
#define _CAMERA_MANAGER_H_

#include "../Device/Singleton.h"

#include <map>
#include <memory>
#include <string>

class Camera;

class CameraManager
	:public Singleton<CameraManager>
{
public:
	friend class Singleton<CameraManager>;
	void Init();
	void Shutdown();
	void SetMainCamera(const std::string& cameraName);
	bool AddCamera(const std::string& cameraName, std::shared_ptr<Camera> camera);
	std::shared_ptr<Camera> GetMainCamera();
	std::shared_ptr<Camera> GetCamera(std::string cameraName);

protected:
	CameraManager();
	~CameraManager();

private:
	std::map<std::string,std::shared_ptr<Camera>> m_Cameras;
	std::shared_ptr<Camera> m_MainCamera;
};

#endif