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

// 当たり判定を検知したら、発生するものとして扱う予定
void RigidBodyComponent::OnCollider()
{
	// 今は単純に0にするだけ。	
	_Velocity = SimpleMath::Vector3::Zero;
}

void RigidBodyComponent::AddGravity()
{
	_Velocity += _Gravity * _Mass * Time::DeltaTime;
}

