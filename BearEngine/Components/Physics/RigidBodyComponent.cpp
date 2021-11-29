#include "RigidBodyComponent.h"

#include "Components/Collsions/CollisionComponent.h"
#include "Components/Collsions/InterSectInfo.h"
#include "Game_Object/Actor.h"
#include "imgui/imgui.h"
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

void RigidBodyComponent::AddImpulse(const SimpleMath::Vector3 vec)
{
	_Velocity += vec * _Mass;
}

const SimpleMath::Vector3& RigidBodyComponent::GetVelocity()
{
	return _Velocity;
}

// 当たり判定を検知したら、発生するものとして扱う予定
void RigidBodyComponent::OnCollider(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info)
{
	// 今は単純に0にするだけ。	
	_Velocity = SimpleMath::Vector3::Zero;
}

void RigidBodyComponent::OnStatic()
{
	_isStatic = true;
	_Mass = 0.0f;
}

void RigidBodyComponent::OnResolveContact(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info)
{
	// 物理計算は行わない。
	if (!IsCalculatePhysics() || !otherCollisionComponent->GetRigidBody()->IsCalculatePhysics())
	{
		return;
	}

	ResolveContact(other, otherCollisionComponent->GetRigidBody(),inter_sect_info);
}

void RigidBodyComponent::DrawProperties()
{
	float f_gravity[3] =
	{
		_AddGravity.x,
		_AddGravity.y,
		_AddGravity.z
	};

	
	if(ImGui::TreeNode("RigidBodyComponent"))
	{
		if (ImGui::DragFloat3("Gravity", f_gravity, 0.01f))
		{
			_AddGravity = DirectX::SimpleMath::Vector3(f_gravity);
		}

		ImGui::TreePop();
	}

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

// 衝突時の移動量の計算
void RigidBodyComponent::CalculateMoment(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody, InterSectInfo& inter_sect_info)
{
	const SimpleMath::Vector3& normal = inter_sect_info._Normal;
	const SimpleMath::Vector3 vab = _Velocity - otherRigidBody->GetVelocity();
	const float impulse = -2.0f * vab.Dot(normal) / (_Mass + otherRigidBody->_Mass);
	const SimpleMath::Vector3 impulseVec = normal * impulse;

	AddImpulse(impulseVec);
	otherRigidBody->AddImpulse(impulseVec * -1.0f);
}

// 物体同士の押し出し
void RigidBodyComponent::ResolveContact(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody, InterSectInfo& inter_sect_info)
{
	CalculateMoment(other, otherRigidBody, inter_sect_info);
	
	const float tA = _Mass / (_Mass + otherRigidBody->GetMass());
	const float tB = otherRigidBody->GetMass() / (_Mass + otherRigidBody->GetMass());

	auto position = inter_sect_info._InterSectPositionA;
	auto otherPosition = inter_sect_info._InterSectPositionB;

	const SimpleMath::Vector3 ds = otherPosition - position;

	// 同じだけ補正するイメージ
	_user->SetPosition(_user->GetPosition() + ds * tA * 1.0f);
	other->SetPosition(other->GetPosition() - ds * tB * 1.0f);


}

