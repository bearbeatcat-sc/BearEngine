#include "RigidBody.h"

#include "Game_Object/Actor.h"
#include "Utility/Time.h"

RigidBody::RigidBody(Actor* pActor)
	:_Velocity(SimpleMath::Vector3(0.0f)),_Gravity(SimpleMath::Vector3(0,-10.0f,0.0f)),_User(pActor)
{
}

RigidBody::~RigidBody()
{
}

void RigidBody::Update()
{
	AddGravity();

	auto position = _User->GetPosition();
	position += _Velocity;

	_User->SetPosition(position);
}

void RigidBody::AddImpulse(const SimpleMath::Vector3& vec)
{
	_Velocity += vec * _Mass;
}

void RigidBody::AddGravity()
{
	_Velocity += _Gravity * _Mass * Time::DeltaTime;
}

