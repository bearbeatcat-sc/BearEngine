#include "SceneManager.h"
#include "Scene.h"

void SceneManager::Update()
{
	if (m_CurrentScene != nullptr)
	{
		m_CurrentScene->Update();
	}

	if (m_ChangeFlag)
	{
		ChangeScene();
		m_ChangeFlag = false;
	}
}

void SceneManager::ChangeScene(std::string sceneName)
{
	m_NextScene = sceneName;
	m_ChangeFlag = true;
}

void SceneManager::ChangeScene()
{
	if (m_CurrentScene != nullptr)
	{
		m_CurrentScene->ShutDown();
	}
	m_CurrentScene = m_Scenes.at(m_NextScene);
	m_CurrentScene->Init();
}

void SceneManager::AddScene(std::string sceneName, Scene* scene)
{
	m_Scenes.emplace(sceneName, scene);
}

SceneManager::SceneManager()
{
	m_ChangeFlag = false;
}

SceneManager::~SceneManager()
{
	for (auto itr = m_Scenes.begin(); itr != m_Scenes.end(); itr++)
	{
		delete (*itr).second;
	}

	m_Scenes.clear();
}
