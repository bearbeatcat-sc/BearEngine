#include "RigidBodyComponent.h"

#include "Components/Collsions/CollisionComponent.h"
#include "Components/Collsions/InterSectInfo.h"
#include "Game_Object/Actor.h"
#include "Utility/Time.h"

RigidBodyComponent::RigidBodyComponent(Actor* pActor)
	:Component(pActor), _Velocity(SimpleMath::Vector3(0.0f)), _AddGravity(SimpleMath::Vector3(0, -10.0f, 0.0f)),
_Gravity(SimpleMath::Vector3::Zero), _Mass(1.0f), _isStatic(false)
{
}

RigidBodyComponent::~RigidBodyComponent()
{
}

void RigidBodyComponent::Update()
{
	AddGravity();

	auto position = _user->GetPosition();
	position += _Velocity;

	_user->SetPosition(position);
}

void RigidBodyComponent::AddImpulse(const SimpleMath::Vector3& vec)
{
	_Velocity += vec * _Mass;
}

// �����蔻������m������A����������̂Ƃ��Ĉ����\��
void RigidBodyComponent::OnCollider(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info)
{
	// ���͒P����0�ɂ��邾���B	
	_Velocity = SimpleMath::Vector3::Zero;
}

void RigidBodyComponent::OnStatic()
{
	_isStatic = true;
	_Mass = 0.0f;
}

void RigidBodyComponent::OnResolveContact(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info)
{
	// �����v�Z�͍s��Ȃ��B
	if (!IsCalculatePhysics() || !otherCollisionComponent->GetRigidBody()->IsCalculatePhysics())
	{
		return;
	}

	ResolveContact(other, otherCollisionComponent->GetRigidBody(),inter_sect_info);
}

const bool RigidBodyComponent::IsStatic() const
{
	return _isStatic;
}

const bool RigidBodyComponent::IsCalculatePhysics() const
{
	return _Mass > 0.0f && !_isStatic;
}

void RigidBodyComponent::AddGravity()
{
	_Velocity += _AddGravity * _Mass * Time::DeltaTime;
}

const float RigidBodyComponent::GetMass()
{
	return _Mass;
}

// ���̓��m�̉����o��
void RigidBodyComponent::ResolveContact(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody, InterSectInfo& inter_sect_info)
{

	
	const float tA = _Mass / (_Mass + otherRigidBody->GetMass());
	const float tB = otherRigidBody->GetMass() / (_Mass + otherRigidBody->GetMass());

	auto position = inter_sect_info._InterSectPositionA;
	auto otherPosition = inter_sect_info._InterSectPositionB;

	const SimpleMath::Vector3 ds = position - otherPosition;

	// ���������␳����C���[�W
	_user->SetPosition(position + ds * (tA * 1.02f));
	other->SetPosition(position - ds * (tB * 1.02f));


}

