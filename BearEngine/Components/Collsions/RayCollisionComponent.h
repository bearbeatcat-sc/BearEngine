#pragma once

#include "CollisionComponent.h"
#include <string>

class CollisionTreeObject;

using namespace DirectX;

class RayCollisionComponent
	:public CollisionComponent
{
public:
	RayCollisionComponent(Actor* actor, const SimpleMath::Vector3 origin, const SimpleMath::Vector3 dir, std::string collisonTag);
	~RayCollisionComponent() = default;

	const SimpleMath::Vector3& GetOrigin();
	const SimpleMath::Vector3& GetDir();

	void SetOrigin(const SimpleMath::Vector3& origin);
	void SetDir(const SimpleMath::Vector3& dir);

	virtual const SimpleMath::Vector3 GetMin() override;
	virtual const SimpleMath::Vector3 GetMax() override;

	virtual bool IsInterSect(CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info) override;
	virtual void Update() override;

private:
	SimpleMath::Vector3 _origin;
	SimpleMath::Vector3 _dir;

};
