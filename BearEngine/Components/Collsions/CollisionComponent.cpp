#include "CollisionComponent.h"
#include "../../Game_Object/Actor.h"
#include "CollisionTagManager.h"
#include "CollisionTree_Object.h"
#include "../../Utility/Time.h"

CollisionComponent::CollisionComponent(Actor* user, CollisionType collType, const std::string& collisonTag, int upadeteOredr)
	:Component(user, upadeteOredr), _collisionType(collType), _collisionTag(collisonTag), _treeObject(nullptr),_isDrawDebug(true)
{
	_collisionIndex = CollisionTagManagaer::GetInstance().GetTagIndex(collisonTag);
}

CollisionComponent::~CollisionComponent()
{
	if (_treeObject != nullptr)
	{
		delete _treeObject;
		_treeObject = nullptr;
	}

	_user = nullptr;
}

void CollisionComponent::Update()
{
	return;
}

const DirectX::SimpleMath::Vector3& CollisionComponent::GetUserPosition()
{
	return _user->GetPosition();
}

CollisionType CollisionComponent::GetCollisionType()
{
	return _collisionType;
}

void CollisionComponent::OnDrawDebug()
{
	_isDrawDebug = true;
}

void CollisionComponent::OffDrawDebug()
{
	_isDrawDebug = false;
}

const std::string& CollisionComponent::GetCollsionTag()
{
	return _collisionTag;
}



void CollisionComponent::SetCollisionTag(const std::string&  tag)
{
	_collisionTag = tag;
}

int CollisionComponent::GetCollisionIndex()
{
	return _collisionIndex;
}

void CollisionComponent::SetTreeObject(CollisionTreeObject* treeobj)
{
	_treeObject = treeobj;
}

Actor* CollisionComponent::GetUser()
{
	return _user;
}

void CollisionComponent::UserOnCollision(Actor* other, CollisionComponent* collisionComponent)
{
	// TODO:　実装中のため使用しない
	//if (m_IsRigid)
	//{
	//	RigidUpdate(collisionComponent);
	//}

	_user->OnCollsion(other);
}

void CollisionComponent::Delete()
{
	_deleteFlag = true;
}

bool CollisionComponent::IsDelete()
{
	return _deleteFlag;
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
	return _treeObject;
}

