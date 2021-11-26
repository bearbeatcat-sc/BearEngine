#pragma once

#include <SimpleMath.h>
#include <Components/Component.h>

using namespace DirectX;

class Actor;

class RigidBodyComponent
	:public Component
{
public:
	RigidBodyComponent(Actor* pActor);
	~RigidBodyComponent();
	virtual  void Update() override;
	void AddImpulse(const SimpleMath::Vector3& vec);
	void OnCollider();

private:
	void AddGravity();

private:
	SimpleMath::Vector3 _Velocity;
	SimpleMath::Vector3 _Gravity;
	float _Mass;
};

