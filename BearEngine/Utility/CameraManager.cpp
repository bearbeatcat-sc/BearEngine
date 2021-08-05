#include "CameraManager.h"
#include "Camera.h"

void CameraManager::Init()
{
	m_MainCamera = std::shared_ptr<Camera>(new Camera(SimpleMath::Vector3(0, 0, -10), SimpleMath::Vector3(0, 0, 0)));

}

void CameraManager::Shutdown()
{
	m_Cameras.clear();
}

bool CameraManager::AddCamera(std::string cameraName, std::shared_ptr<Camera> camera)
{
	if (m_Cameras.find(cameraName) != m_Cameras.end())return false;

	m_Cameras.emplace(cameraName, camera);

	return true;
}

std::shared_ptr<Camera> CameraManager::GetMainCamera()
{
	return m_MainCamera;
}

std::shared_ptr<Camera> CameraManager::GetCamera(std::string cameraName)
{
	if (m_Cameras.find(cameraName) == m_Cameras.end())return nullptr;

	return m_Cameras.at(cameraName);
}

CameraManager::CameraManager()
{

}

CameraManager::~CameraManager()
{
	Shutdown();
}
