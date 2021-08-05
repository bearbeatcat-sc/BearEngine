#ifndef _AABB_COLLISION_COMPONENT_H_
#define _AABB_COLLISION_COMPONENT_H_

#include "CollisionComponent.h"
#include <string>

class CollisionTreeObject;

using namespace DirectX;

class AABBCollisionComponent
	:public CollisionComponent
{
public:
	AABBCollisionComponent(Actor* actor, SimpleMath::Vector3 center, SimpleMath::Vector3 size, std::string collisonTag);
	~AABBCollisionComponent();
	void SetDebug(bool flag);
	SimpleMath::Vector3 GetCenter();
	SimpleMath::Vector3 GetSize();	
	SimpleMath::Vector3 GetMin();
	SimpleMath::Vector3 GetMax();
	void SetCenterPosition(const SimpleMath::Vector3& pos);
	void SetSize(const SimpleMath::Vector3& size);
	void SetAdjustPos(SimpleMath::Vector3 pos);
	void SetTreeObject(CollisionTreeObject* treeobj);
	virtual bool IsInterSect(CollisionComponent* collisionComponent) override;
	virtual void Update() override;

private:
	bool m_IsDrawDebug;
	bool m_IsSetPosition;

private:
	SimpleMath::Vector3 m_Center;
	SimpleMath::Vector3 m_AdjustPos;
	SimpleMath::Vector3 m_Size;
};

#endif