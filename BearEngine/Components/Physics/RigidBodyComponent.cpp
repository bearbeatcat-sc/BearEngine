#include "RigidBodyComponent.h"

#include "Game_Object/Actor.h"
#include "Utility/Time.h"

RigidBodyComponent::RigidBodyComponent(Actor* pActor)
	:Component(pActor),_Velocity(SimpleMath::Vector3(0.0f)),_Gravity(SimpleMath::Vector3(0,-10.0f,0.0f))
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
void RigidBodyComponent::OnCollider()
{
	// ���͒P����0�ɂ��邾���B	
	_Velocity = SimpleMath::Vector3::Zero;
}

void RigidBodyComponent::AddGravity()
{
	_Velocity += _Gravity * _Mass * Time::DeltaTime;
}

