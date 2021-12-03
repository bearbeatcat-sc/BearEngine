#pragma once

#include <SimpleMath.h>
#include <Components/Component.h>

#include "Components/Collsions/CollisionType.h"

struct InterSectInfo;
using namespace DirectX;

class Actor;
class CollisionComponent;
class SphereCollisionComponent;
class OBBCollisionComponent;

class RigidBodyComponent
	:public Component
{
public:
	RigidBodyComponent(Actor* pActor, CollisionComponent* collision_component);
	void UpdatePosition();
	void UpdateRotation();
	~RigidBodyComponent() = default;
	
	virtual  void Update() override;
	
	void AddImpulse(const SimpleMath::Vector3 vec);
	void AddAngularImpulse(const SimpleMath::Vector3 point, const SimpleMath::Vector3 impulse);
	
	const SimpleMath::Vector3& GetVelocity();
	SimpleMath::Matrix InvTensor();

	void OnCollider(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info);
	void OnStaticRotate();
	void OnStaticPosition();
	void OnResolveContact(Actor* other, CollisionComponent* otherCollisionComponent, InterSectInfo& inter_sect_info);
	void DrawProperties() override;
	void UpdateActorPosition(const SimpleMath::Vector3 pos);
	float InvMass();
	
	void SetVelocity(const SimpleMath::Vector3& velocity);
	void SetAngVel(const SimpleMath::Vector3& angVel);

	const bool IsCalculatePhysics() const;

	SimpleMath::Vector3 _AddGravity;
	float _Mass;
	float _Elasticty; // íeê´
	float _Friction; // ñÄéC

	SimpleMath::Vector3 _Orientation; // âÒì]
	SimpleMath::Vector3 _AngVel;
	SimpleMath::Vector3 _Toruques; // ÉgÉãÉN

private:
	void AddGravity();
	const float GetMass();
	void CalculateMoment(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody,
	                     InterSectInfo& inter_sect_info);
	void ResolveContact(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody,InterSectInfo& inter_sect_info);
	
	SimpleMath::Matrix InvTensor(OBBCollisionComponent* obb_collision_component);
	SimpleMath::Matrix InvTensor(SphereCollisionComponent* sphere_collision_component);
private:
	SimpleMath::Vector3 _Velocity;
	SimpleMath::Vector3 _Forces;
	
	bool _isStaticPosition;
	bool _isStaticRotate;

	CollisionComponent* _CollisionComponent;
};

