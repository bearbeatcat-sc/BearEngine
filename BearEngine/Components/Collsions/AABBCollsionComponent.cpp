#include "AABBCollsionComponent.h"
#include "../../Utility/Math/MathUtility.h"
#include "../../Game_Object/Actor.h"
#include "CollisionTree_Object.h"
#include "../../Device/DirectX/Core/Model/DebugDrawer.h"

AABBCollisionComponent::AABBCollisionComponent(Actor* actor, SimpleMath::Vector3 center, SimpleMath::Vector3 size, std::string collisonTag)
	:CollisionComponent(actor, CollisionType::CollisionType_AABB, collisonTag),m_Center(center),m_Size(size), m_IsDrawDebug(true), m_IsSetPosition(false)
{
}

AABBCollisionComponent::~AABBCollisionComponent()
{

}

void AABBCollisionComponent::SetDebug(bool flag)
{
	m_IsDrawDebug = flag;
}

SimpleMath::Vector3 AABBCollisionComponent::GetCenter()
{
	return m_Center + m_AdjustPos;
}

SimpleMath::Vector3 AABBCollisionComponent::GetSize()
{
	return m_Size;
}

SimpleMath::Vector3 AABBCollisionComponent::GetMin()
{
	return m_Center - m_Size;
}

SimpleMath::Vector3 AABBCollisionComponent::GetMax()
{
	return m_Center + m_Size;
}

void AABBCollisionComponent::SetCenterPosition(const SimpleMath::Vector3& pos)
{
	m_Center = pos;
	m_IsSetPosition = true;
}

void AABBCollisionComponent::SetSize(const SimpleMath::Vector3& size)
{
	m_Size = size;
}

void AABBCollisionComponent::SetAdjustPos(SimpleMath::Vector3 pos)
{
	m_AdjustPos = pos;
}

void AABBCollisionComponent::SetTreeObject(CollisionTreeObject* treeobj)
{	
	m_TreeObject = treeobj;
}

bool AABBCollisionComponent::IsInterSect(CollisionComponent* collisionComponent)
{
	// 当たり判定のタイプによって分岐
	if (collisionComponent->GetCollisionType() == CollisionType::CollisionType_AABB)
	{
		auto otherCol = static_cast<AABBCollisionComponent*>(collisionComponent);
		SimpleMath::Vector3 otherMin = otherCol->GetCenter() - otherCol->GetSize();
		SimpleMath::Vector3 otherMax = otherCol->GetCenter() + otherCol->GetSize();

		SimpleMath::Vector3 min = m_Center - m_Size;
		SimpleMath::Vector3 max = m_Center + m_Size;

		if (min.x > otherMax.x) return false;
		if (max.x < otherMin.x) return false;

		if (min.y > otherMax.y) return false;
		if (max.y < otherMin.y) return false;

		if (min.z > otherMax.z) return false;
		if (max.z < otherMin.z) return false;
	}

	return true;
}

void AABBCollisionComponent::Update()
{
	// 位置を手動でセットしていない場合は、セットする。
	if (!m_IsSetPosition) 
	{
		m_Center = m_User->GetPosition() + m_AdjustPos;
		m_IsSetPosition = false;
	}

#ifdef _DEBUG
	if (m_IsDrawDebug)
		DebugDrawer::GetInstance().DrawCube(m_Size, m_Center);
#endif
}

