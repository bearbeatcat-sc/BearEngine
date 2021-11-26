#include "SphereCollisionComponent.h"

#include "CollisionInterSect.h"
#include "CollisionType.h"
#include "Game_Object/Actor.h"
#include "OBBCollisionComponent.h"
#include "Device/DirectX/Core/Model/DebugDrawer.h"

SphereCollisionComponent::SphereCollisionComponent(Actor* actor,float radius, std::string collisonTag)
	:m_Radius(radius),CollisionComponent(actor,CollisionType::CollisionType_Sphere,collisonTag), m_AdjustPos(DirectX::SimpleMath::Vector3(0,0,0))
{

}

float SphereCollisionComponent::GetRadius()
{
	return m_Radius;
}

void SphereCollisionComponent::SetRadius(float radius)
{
	m_Radius = radius;
}

void SphereCollisionComponent::SetAdjustPos(DirectX::SimpleMath::Vector3 pos)
{
	m_AdjustPos = pos;
}

const DirectX::SimpleMath::Vector3& SphereCollisionComponent::GetAdjustPos()
{
	return m_AdjustPos;
}

const DirectX::SimpleMath::Vector3& SphereCollisionComponent::GetPosition()
{
	return _user->GetPosition() + m_AdjustPos;
}

bool SphereCollisionComponent::IsInterSect(CollisionComponent* collisionComponent)
{
	// 中心と点との距離の二乗
	//float distance = (GetUserPosition() - collisionComponent->GetUserPosition()).lengthSquare();
	//float sumRadius = m_Radius + static_cast<SphereCollisionComponent*>(collisionComponent)->GetRadius();

	//return distance <= (sumRadius * sumRadius);

	auto otherCollisionType = collisionComponent->GetCollisionType();

	if(otherCollisionType == CollisionType_Sphere)
	{
		return CollisionInterSect::SphereToSphereInterSect(this, static_cast<SphereCollisionComponent*>(collisionComponent));
	}

	if (otherCollisionType == CollisionType_OBB)
	{
		// 今は使わないかも
		SimpleMath::Vector3 point;
		return CollisionInterSect::SphereToOBBInterSect(this, static_cast<OBBCollisionComponent*>(collisionComponent), point);
	}

	
	return false;

}

void SphereCollisionComponent::Update()
{

#ifdef _DEBUG
	if (_isDrawDebug)
		DebugDrawer::GetInstance().DrawSphere(m_Radius * 2.0f, GetPosition());
#endif
}

const DirectX::SimpleMath::Vector3 SphereCollisionComponent::GetMin()
{
	return _user->GetPosition() - DirectX::SimpleMath::Vector3(m_Radius);
}

const DirectX::SimpleMath::Vector3 SphereCollisionComponent::GetMax()
{
	return _user->GetPosition() + DirectX::SimpleMath::Vector3(m_Radius);
}
