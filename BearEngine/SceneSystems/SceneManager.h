#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_

#include "../Device/Singleton.h"
#include <string>
#include <map>

class Scene;

class SceneManager
	: public Singleton<SceneManager>
{
public:
	friend class Singleton<SceneManager>;
	void Update();
	void ChangeScene(std::string sceneName);
	void ChangeScene();
	void AddScene(std::string sceneName, Scene* scene);

protected:
	SceneManager();
	~SceneManager();

private:
	std::map<std::string, Scene*> m_Scenes;
	Scene* m_CurrentScene;
	std::string m_NextScene;
	bool m_ChangeFlag;
};

#endif