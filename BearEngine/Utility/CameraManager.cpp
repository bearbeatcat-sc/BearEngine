#include "CameraManager.h"
#include "Camera.h"

void CameraManager::Init()
{

	auto camera = std::make_shared<Camera>(SimpleMath::Vector3(0, 0, -10), SimpleMath::Vector3(0, 0, 0));
	AddCamera("Camera0",camera);

	SetMainCamera("Camera0");
}

void CameraManager::Shutdown()
{
	m_Cameras.clear();
}

void CameraManager::SetMainCamera(const std::string& cameraName)
{
	if (m_Cameras.find(cameraName) == m_Cameras.end())
	{
		throw std::runtime_error("Not Regist Camera");
	}

	auto camera = m_Cameras.at(cameraName);
	m_MainCamera = camera;
}

bool CameraManager::AddCamera(const std::string& cameraName, std::shared_ptr<Camera> camera)
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
	Init();
}

CameraManager::~CameraManager()
{
	Shutdown();
}
