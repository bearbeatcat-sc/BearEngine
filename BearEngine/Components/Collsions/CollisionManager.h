#ifndef _COLLISION_MANAGER_H_
#define _COLLISION_MANAGER_H_

#include "../../Device/Singleton.h"
#include "CollisionTree_Manager.h"
#include <SimpleMath.h>

#include <vector>

class CollisionComponent;

class CollisionManager
	:public Singleton<CollisionManager>
{
public:
	friend class Singleton<CollisionManager>;
	void AddComponent(CollisionComponent* component);
	void AddRegistTree(AABBCollisionComponent* component);
	void UpdateRegistTree(AABBCollisionComponent* component,CollisionTreeObject* object);
	void Update();
	void SetDebugMode(bool flag);
	void InterSect();
	void Delete();
	void Delete(AABBCollisionComponent* component);
	void Reset();
	void Draw();
	void Init(int level,const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);
	void SetTreeProperties(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);
protected:
	CollisionManager();
	~CollisionManager();

private:
	std::vector<CollisionComponent*> m_components;
	CollisionTreeManager* m_CollisionTreeManager;
	bool m_isDebugMode;
	double m_CurrentCollisitonTime;
};

#endif