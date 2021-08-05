#include "SphereCollisionComponent.h"
#include "CollisionType.h"

SphereCollisionComponent::SphereCollisionComponent(Actor* actor,float radius, std::string collisonTag)
	:m_Radius(radius),CollisionComponent(actor,CollisionType::CollisionType_Sphere,collisonTag), m_AdjustPos(DirectX::SimpleMath::Vector3(0,0,0))
{

}

SphereCollisionComponent::~SphereCollisionComponent()
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

DirectX::SimpleMath::Vector3 SphereCollisionComponent::GetAdjustPos()
{
	return m_AdjustPos;
}

bool SphereCollisionComponent::IsInterSect(CollisionComponent* collisionComponent)
{
	// 中心と点との距離の二乗
	//float distance = (GetUserPosition() - collisionComponent->GetUserPosition()).lengthSquare();
	//float sumRadius = m_Radius + static_cast<SphereCollisionComponent*>(collisionComponent)->GetRadius();

	//return distance <= (sumRadius * sumRadius);

	DirectX::SimpleMath::Vector3 thisPos = GetUserPosition();
	DirectX::SimpleMath::Vector3 otherPos = collisionComponent->GetUserPosition();
	float thisRadius = m_Radius;
	float otherRadius = static_cast<SphereCollisionComponent*>(collisionComponent)->GetRadius();


	DirectX::SimpleMath::Vector3 temp = otherPos - thisPos;


	float sum_Radius = thisRadius + otherRadius;




	return (temp.x * temp.x) + (temp.y * temp.y) + (temp.z * temp.z) <= ((thisRadius + otherRadius) * (thisRadius + otherRadius));
}
