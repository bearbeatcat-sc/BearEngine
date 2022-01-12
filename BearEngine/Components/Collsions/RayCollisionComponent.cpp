#include "RayCollisionComponent.h"

#include "CollisionInterSect.h"
#include "OBBCollisionComponent.h"

RayCollisionComponent::RayCollisionComponent(Actor* actor,const  SimpleMath::Vector3 origin,const  SimpleMath::Vector3 dir,std::string collisonTag)
	:CollisionComponent(actor,CollisionType_Ray,collisonTag),_origin(origin),_dir(dir)
{
}

const SimpleMath::Vector3& RayCollisionComponent::GetOrigin()
{
	return _origin;
}

const SimpleMath::Vector3& RayCollisionComponent::GetDir()
{
	return _dir;
}

void RayCollisionComponent::SetOrigin(const SimpleMath::Vector3& origin)
{
	_origin = origin;
}

void RayCollisionComponent::SetDir(const SimpleMath::Vector3& dir)
{
	_dir = dir;
}

// レイなので、扱わない
const SimpleMath::Vector3 RayCollisionComponent::GetMin()
{
	return SimpleMath::Vector3::Zero;
}

const SimpleMath::Vector3 RayCollisionComponent::GetMax()
{
	return SimpleMath::Vector3::Zero;
}

bool RayCollisionComponent::IsInterSect(CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info)
{
	const auto otherCollisionType = collisionComponent->GetCollisionType();


	if (otherCollisionType == CollisionType_OBB)
	{
		return CollisionInterSect::OBBToRayInterSect( static_cast<OBBCollisionComponent*>(collisionComponent), this, inter_sect_info);
	}
}

void RayCollisionComponent::Update()
{
	
}
