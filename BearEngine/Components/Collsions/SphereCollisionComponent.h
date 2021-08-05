#ifndef _SPHERE_COLLISION_COMPONENT_H_
#define _SPHERE_COLLISION_COMPONENT_H_

#include "CollisionComponent.h"
#include <string>
#include "SimpleMath.h"

class SphereCollisionComponent
	:public CollisionComponent
{
public:
	SphereCollisionComponent(Actor* actor, float radius, std::string collisonTag);
	~SphereCollisionComponent();
	float GetRadius();
	void SetRadius(float radius);
	void SetAdjustPos(DirectX::SimpleMath::Vector3 pos);
	DirectX::SimpleMath::Vector3 GetAdjustPos();
	virtual bool IsInterSect(CollisionComponent* collisionComponent) override;

private:
	float m_Radius;
	DirectX::SimpleMath::Vector3 m_AdjustPos;

};

#endif