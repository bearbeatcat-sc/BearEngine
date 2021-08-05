#include "CollisionComponent.h"
#include "../../Game_Object/Actor.h"
#include "CollisionTagManager.h"
#include "CollisionTree_Object.h"
#include "../../Utility/Time.h"

CollisionComponent::CollisionComponent(Actor* user, CollisionType collType, const std::string& collisonTag, int upadeteOredr)
	:Component(user, upadeteOredr), m_CollisionType(collType), m_CollisonTag(collisonTag), m_TreeObject(nullptr),
	m_IsRigid(false), m_m(0.1f)
{
	m_CollisionIndex = CollisionTagManagaer::GetInstance().GetTagIndex(collisonTag);
}

CollisionComponent::~CollisionComponent()
{
	if (m_TreeObject != nullptr)
	{
		delete m_TreeObject;
		m_TreeObject = nullptr;
	}

	m_User = nullptr;
}

void CollisionComponent::Update()
{
	return;
}

DirectX::SimpleMath::Vector3 CollisionComponent::GetUserPosition()
{
	return m_User->GetPosition();
}

CollisionType CollisionComponent::GetCollisionType()
{
	return m_CollisionType;
}

const std::string& CollisionComponent::GetCollsionTag()
{
	return m_CollisonTag;
}

void CollisionComponent::SetRigidFlag(bool flag)
{
	m_IsRigid = flag;
}

void CollisionComponent::SetCollisionTag(const std::string&  tag)
{
	m_CollisonTag = tag;
}

int CollisionComponent::GetCollisionIndex()
{
	return m_CollisionIndex;
}

const SimpleMath::Vector3& CollisionComponent::GetVelocity()
{
	return m_Vel;
}

const SimpleMath::Vector3& CollisionComponent::GetAcc()
{
	return m_Acc;
}

const float& CollisionComponent::GetMass()
{
	return m_m;
}

void CollisionComponent::SetVelocity(DirectX::SimpleMath::Vector3 vec)
{
	m_Vel = vec;
}

void CollisionComponent::SetAcc(DirectX::SimpleMath::Vector3 acc)
{
	m_Acc = acc;
}

void CollisionComponent::SetMass(float mass)
{
	m_m = mass;
}

Actor* CollisionComponent::GetUser()
{
	return m_User;
}

void CollisionComponent::UserOnCollision(Actor* other, CollisionComponent* collisionComponent)
{
	// TODO:　実装中のため使用しない
	//if (m_IsRigid)
	//{
	//	RigidUpdate(collisionComponent);
	//}

	m_User->OnCollsion(other);
}

void CollisionComponent::Delete()
{
	m_DeleteFlag = true;
}

bool CollisionComponent::IsDelete()
{
	return m_DeleteFlag;
}

//void CollisionComponent::OnAction(Actor* other, CollisionComponent* collisionComponent)
//{
//	if (m_IsRigid)
//	{
//		RigidUpdate(collisionComponent);
//	}
//
//	m_User->OnCollsion(other);
//}

CollisionTreeObject* CollisionComponent::GetCollisionTreeObject()
{
	return m_TreeObject;
}

void CollisionComponent::Rebound(CollisionComponent* collisionComponent)
{
	SimpleMath::Vector3 n = (collisionComponent->GetUserPosition() - m_User->GetPosition());
	n.Normalize();

	SimpleMath::Vector3 v = m_Vel;
	SimpleMath::Vector3 V = collisionComponent->GetVelocity();

	m_Vel = -(((V - v).Dot(n)) * n) + V;
	auto vel = -(((v - V).Dot(n)) * n) + V;

	collisionComponent->SetVelocity(vel);
}

void CollisionComponent::RigidUpdate(CollisionComponent* collisionComponent)
{
	Rebound(collisionComponent);

	m_Vel += m_Acc * Time::DeltaTime;
	auto pos = m_User->GetPosition() + m_Vel * Time::DeltaTime;
	m_User->SetPosition(pos);
}


