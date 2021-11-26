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
	~SphereCollisionComponent() = default;
	float GetRadius();
	void SetRadius(float radius);
	void SetAdjustPos(DirectX::SimpleMath::Vector3 pos);
	
	const DirectX::SimpleMath::Vector3& GetAdjustPos();
	const DirectX::SimpleMath::Vector3& GetPosition();
	
	virtual bool IsInterSect(CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info) override;
	virtual void Update() override;
	
	const DirectX::SimpleMath::Vector3 GetMin() override;
	const DirectX::SimpleMath::Vector3 GetMax() override;
private:
	float m_Radius;
	DirectX::SimpleMath::Vector3 m_AdjustPos;

};

#endif