#ifndef _ACTORMANAGER_H_
#define _ACTORMANAGER_H_

#include "../Device/Singleton.h"
#include <vector>
#include <deque>
#include <memory>

class Actor;

class ActorManager
	:public Singleton<ActorManager>
{
public:
	friend class Singleton<ActorManager>;
	void Init();
	void Update();
	void Shutdown();
	void AddActor(Actor* actor);
	void DeleteActor(Actor* actor);
	void DeleteActor();
	std::vector<Actor*>& GetActors();

protected:
	ActorManager();
	~ActorManager();

private:
	void AddActor();

private:
	std::vector<Actor*> m_Actos;
	std::vector<Actor*> m_AddActors;

};

#endif
