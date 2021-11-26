#pragma once

#include <SimpleMath.h>
#include <Components/Component.h>

struct InterSectInfo;
using namespace DirectX;

class Actor;
class CollisionComponent;

class RigidBodyComponent
	:public Component
{
public:
	RigidBodyComponent(Actor* pActor);
	~RigidBodyComponent();
	virtual  void Update() override;
	void AddImpulse(const SimpleMath::Vector3& vec);
	void OnCollider(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info);
	void OnStatic();
	void OnResolveContact(Actor* other, CollisionComponent* otherCollisionComponent, InterSectInfo& inter_sect_info);
	
	const bool IsStatic() const;
	const bool IsCalculatePhysics() const;

	SimpleMath::Vector3 _AddGravity;
	float _Mass;

private:
	void AddGravity();
	const float GetMass();
	void ResolveContact(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody,InterSectInfo& inter_sect_info);

private:
	SimpleMath::Vector3 _Velocity;
	SimpleMath::Vector3 _Gravity;
	bool _isStatic;
};

