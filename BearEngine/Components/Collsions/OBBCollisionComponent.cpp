#include "OBBCollisionComponent.h"

#include "CollisionInterSect.h"
#include "../../Game_Object/Actor.h"
#include "SphereCollisionComponent.h"
#include "CollisionTree_Object.h"
#include "Device/DirectX/Core/Model/DebugDrawer.h"

OBBCollisionComponent::OBBCollisionComponent(Actor* actor, SimpleMath::Vector3 center, SimpleMath::Vector3 size,std::string collisonTag)
	:CollisionComponent(actor, CollisionType::CollisionType_OBB, collisonTag), _center(center), _size(size), _isSetPosition(false)
{
	_directionVec.resize(3);
}

const SimpleMath::Vector3 OBBCollisionComponent::GetCenter()
{
	return _center + _adjustPos;
}

const SimpleMath::Vector3& OBBCollisionComponent::GetSize()
{
	return _size;
}

const SimpleMath::Vector3 OBBCollisionComponent::GetMin()
{
	return _center - _size;

}

const SimpleMath::Vector3 OBBCollisionComponent::GetMax()
{
	return _center + _size;
}

const std::vector<SimpleMath::Vector3>& OBBCollisionComponent::GetDirectionVec()
{
	return _directionVec;
}

void OBBCollisionComponent::SetCenterPosition(const SimpleMath::Vector3& pos)
{
	_center = pos;
	_isSetPosition = true;
}

void OBBCollisionComponent::SetSize(const SimpleMath::Vector3& size)
{
	_size = size;

}

void OBBCollisionComponent::SetAdjustPos(SimpleMath::Vector3 pos)
{
	_adjustPos = pos;
}

bool OBBCollisionComponent::IsInterSect(CollisionComponent* collisionComponent)
{
	auto otherCollisionType = collisionComponent->GetCollisionType();

	if (otherCollisionType == CollisionType_Sphere)
	{
		// 今は使わないかも
		SimpleMath::Vector3 point;
		return CollisionInterSect::SphereToOBBInterSect(static_cast<SphereCollisionComponent*>(collisionComponent),this,point);
	}

	if (otherCollisionType == CollisionType_OBB)
	{
		return CollisionInterSect::OBBToOBBInterSect(this, static_cast<OBBCollisionComponent*>(collisionComponent));
	}

	return false;
}

void OBBCollisionComponent::Update()
{
	// 位置を手動でセットしていない場合は、セットする。
	if (!_isSetPosition)
	{
		_center = _user->GetPosition() + _adjustPos;
		_isSetPosition = false;

		// 回転用クォータニオンから回転行列に変換。
		auto matrix = SimpleMath::Matrix::CreateFromQuaternion(_user->GetRotation());

		
		// 向きのベクトルに収めていく。
		_directionVec[0] = SimpleMath::Vector3(matrix._11,matrix._12,matrix._13);
		_directionVec[1] = SimpleMath::Vector3(matrix._21, matrix._22, matrix._23);
		_directionVec[2] = SimpleMath::Vector3(matrix._31, matrix._32, matrix._33);
	}

#ifdef _DEBUG
	if (_isDrawDebug)
		DebugDrawer::GetInstance().DrawCube(_size * 2.0f, _center, SimpleMath::Matrix::CreateFromQuaternion(_user->GetRotation()));
#endif
}
