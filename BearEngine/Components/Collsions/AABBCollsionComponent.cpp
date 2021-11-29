#include "AABBCollsionComponent.h"
#include "../../Utility/Math/MathUtility.h"
#include "../../Game_Object/Actor.h"
#include "CollisionTree_Object.h"
#include "../../Device/DirectX/Core/Model/DebugDrawer.h"

AABBCollisionComponent::AABBCollisionComponent(Actor* actor, SimpleMath::Vector3 center, SimpleMath::Vector3 size, std::string collisonTag)
	:CollisionComponent(actor, CollisionType::CollisionType_AABB, collisonTag),_center(center),_size(size), _isSetPosition(false)
{
}

AABBCollisionComponent::~AABBCollisionComponent()
{

}

const SimpleMath::Vector3& AABBCollisionComponent::GetCenter()
{
	return _center + _adjustPos;
}

const SimpleMath::Vector3& AABBCollisionComponent::GetSize()
{
	return _size;
}

const SimpleMath::Vector3 AABBCollisionComponent::GetMin()
{
	return _center - _size;
}

const SimpleMath::Vector3 AABBCollisionComponent::GetMax()
{
	return _center + _size;
}

void AABBCollisionComponent::SetCenterPosition(const SimpleMath::Vector3& pos)
{
	_center = pos;
	_isSetPosition = true;
}

void AABBCollisionComponent::SetSize(const SimpleMath::Vector3& size)
{
	_size = size;
}

void AABBCollisionComponent::SetAdjustPos(SimpleMath::Vector3 pos)
{
	_adjustPos = pos;
}

bool AABBCollisionComponent::IsInterSect(CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info)
{
	auto otherCollisionType = collisionComponent->GetCollisionType();
	
	// �����蔻��̃^�C�v�ɂ���ĕ���
	if (otherCollisionType == CollisionType_AABB)
	{
		auto otherCol = static_cast<AABBCollisionComponent*>(collisionComponent);
		SimpleMath::Vector3 otherMin = otherCol->GetCenter() - otherCol->GetSize();
		SimpleMath::Vector3 otherMax = otherCol->GetCenter() + otherCol->GetSize();

		SimpleMath::Vector3 min = _center - _size;
		SimpleMath::Vector3 max = _center + _size;

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
	// �ʒu��蓮�ŃZ�b�g���Ă��Ȃ��ꍇ�́A�Z�b�g����B
	if (!_isSetPosition) 
	{
		_center = _user->GetPosition() + _adjustPos;
		_isSetPosition = false;
	}

#ifdef _DEBUG
	if (_isDrawDebug)
		DebugDrawer::GetInstance().DrawCube(_size, _center,SimpleMath::Matrix::Identity);
#endif
}

