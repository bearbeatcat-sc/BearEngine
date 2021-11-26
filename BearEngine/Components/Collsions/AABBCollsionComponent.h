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
	
	const SimpleMath::Vector3& GetCenter();
	const SimpleMath::Vector3& GetSize();	
	virtual const SimpleMath::Vector3 GetMin() override;
	virtual const SimpleMath::Vector3 GetMax() override;
	
	void SetCenterPosition(const SimpleMath::Vector3& pos);
	void SetSize(const SimpleMath::Vector3& size);
	void SetAdjustPos(SimpleMath::Vector3 pos);
	virtual bool IsInterSect(CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info) override;
	virtual void Update() override;

private:
	bool _isSetPosition;

private:
	SimpleMath::Vector3 _center;
	SimpleMath::Vector3 _adjustPos;
	SimpleMath::Vector3 _size;
};

#endif