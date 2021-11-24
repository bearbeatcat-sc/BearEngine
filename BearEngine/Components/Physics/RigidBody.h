#pragma once
#include <SimpleMath.h>

using namespace DirectX;

class Actor;

class RigidBody
{
public:
	RigidBody(Actor* pActor);
	~RigidBody();
	void Update();
	void AddImpulse(const SimpleMath::Vector3& vec);

private:
	void AddGravity();

private:
	SimpleMath::Vector3 _Velocity;
	SimpleMath::Vector3 _Gravity;
	float _Mass;

	Actor* _User;
};

