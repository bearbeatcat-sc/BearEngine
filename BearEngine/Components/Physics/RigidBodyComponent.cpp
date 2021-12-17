#include "RigidBodyComponent.h"

#include "Components/Collsions/CollisionComponent.h"
#include "Components/Collsions/InterSectInfo.h"
#include "Components/Collsions/OBBCollisionComponent.h"
#include "Components/Collsions/SphereCollisionComponent.h"
#include "Game_Object/Actor.h"
#include "imgui/imgui.h"
#include "Utility/Time.h"

RigidBodyComponent::RigidBodyComponent(Actor* pActor, CollisionComponent* collision_component)
	:Component(pActor), _Velocity(SimpleMath::Vector3(0.0f)), _AddGravity(SimpleMath::Vector3(0, -10.0f, 0.0f)),
	_Mass(1.0f), _isStaticPosition(false), _isStaticRotate(false), _Elasticty(1.0f), _CollisionComponent(collision_component),
	_Friction(1.0f)
{
}

void RigidBodyComponent::UpdatePosition()
{
	AddGravity();

	if (_isStaticPosition)return;
	if (_Forces.Length() <= 0.0f && _Velocity.Length() <= 0.0f)return;

	
	SimpleMath::Vector3 accel = _Forces * InvMass();

	_Velocity += accel * Time::DeltaTime;

	auto position = _user->GetPosition();
	position += _Velocity * Time::DeltaTime;
	_user->SetPosition(position);

	_Velocity *= 0.98f;
}

void RigidBodyComponent::UpdateRotation()
{
	if (_isStaticRotate)return;
	if (_AngVel.Length() <= 0.0f)return;

	auto qu = _user->GetVecRotation();
	qu += _AngVel;

	_user->SetRotation(qu);

	// 減衰
	// _V
	_AngVel *= 0.98f;
}

void RigidBodyComponent::Update()
{
	UpdatePosition();
	UpdateRotation();
}

void RigidBodyComponent::AddImpulse(const SimpleMath::Vector3 vec)
{
	if (_isStaticPosition) return;

	_Velocity += vec;
}

void RigidBodyComponent::AddAngularImpulse(const SimpleMath::Vector3 point, const SimpleMath::Vector3 impulse)
{

	if (_isStaticRotate) return;

	// 現状はアクターの位置。あとあと、コライダーの位置を元に設定する予定
	auto centerOfmass = _user->GetPosition();

	//　衝突点との差分からトルクを求める。
	auto torque = (point - centerOfmass).Cross(impulse);

	// 角加速度
	auto angAccel = SimpleMath::Vector3::Transform(torque, InvTensor());
	_AngVel += angAccel;
}

const SimpleMath::Vector3& RigidBodyComponent::GetVelocity()
{
	return _Velocity;
}

// �����蔻�����m������A���������̂Ƃ��Ĉ����\��
void RigidBodyComponent::OnCollider(Actor* other, CollisionComponent* otherCollisionComponent, InterSectInfo& inter_sect_info)
{
	// ���͒P����0�ɂ��邾���B	
	//_Velocity = SimpleMath::Vector3::Zero;
}

void RigidBodyComponent::SetStaticFlag(bool positionFlag, bool rotateFlag)
{
	_isStaticPosition = positionFlag;
	_isStaticRotate = rotateFlag;

}



void RigidBodyComponent::OnResolveContact(Actor* other, CollisionComponent* otherCollisionComponent, InterSectInfo& inter_sect_info)
{
	// �����v�Z�͍s��Ȃ��B
	if (!IsCalculatePhysics() && !otherCollisionComponent->GetRigidBody()->IsCalculatePhysics())
	{
		return;
	}

	ResolveContact(other, otherCollisionComponent->GetRigidBody(), inter_sect_info);
}

void RigidBodyComponent::DrawProperties()
{
	float f_gravity[3] =
	{
		_AddGravity.x,
		_AddGravity.y,
		_AddGravity.z
	};

	float f_addAngVel[3] =
	{
		_AngVel.x,
		_AngVel.y,
		_AngVel.z
	};

	float f_velocity[3] =
	{
		_Velocity.x,
		_Velocity.y,
		_Velocity.z
	};

	if (ImGui::TreeNode("RigidBodyComponent"))
	{
		if (ImGui::DragFloat3("Gravity", f_gravity, 0.01f))
		{
			_AddGravity = DirectX::SimpleMath::Vector3(f_gravity);
		}

		if (ImGui::DragFloat3("AddAngVel", f_addAngVel, 0.01f))
		{
			_AngVel = DirectX::SimpleMath::Vector3(f_addAngVel);
		}

		if (ImGui::DragFloat3("Velocity", f_velocity, 0.01f))
		{
			_Velocity = DirectX::SimpleMath::Vector3(f_velocity);
		}

		if (ImGui::DragFloat("Mass", &_Mass, 0.01f))
		{
		}

		if (ImGui::DragFloat("Elasticty", &_Elasticty, 0.01f))
		{
		}

		if (ImGui::DragFloat("Friction", &_Friction, 0.01f))
		{
		}

		if (ImGui::Checkbox("IsStaticPosition", &_isStaticPosition))
		{
		}

		if (ImGui::Checkbox("IsStaticRotate", &_isStaticRotate))
		{
		}

		ImGui::TreePop();
	}

}



const bool RigidBodyComponent::IsCalculatePhysics() const
{
	return _Mass > 0.0f;
}

void RigidBodyComponent::AddGravity()
{
	_Forces = _AddGravity * _Mass;
}

const float RigidBodyComponent::GetMass()
{
	return _Mass;
}

// �Փˎ��̈ړ��ʂ̌v�Z
void RigidBodyComponent::CalculateMoment(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody, InterSectInfo& inter_sect_info)
{
	// �e���̌v�Z
	//const float elasticityA = _Elasticty;
	//const float elasticityB = otherRigidBody->_Elasticty;

	//const float elasticity = elasticityA * elasticityB;

	//SimpleMath::Vector3 r1 = inter_sect_info._InterSectPositionA - inter_sect_info._PoisitionA;
	//SimpleMath::Vector3 r2 = inter_sect_info._InterSectPositionB - inter_sect_info._PoisitionB;
	//	//const SimpleMath::Vector3 vab = _Velocity - otherRigidBody->GetVelocity();

	////const float impulse = -(1.0f + elasticity) * vab.Dot(normal) / (_Mass + otherRigidBody->_Mass);

	//const SimpleMath::Vector3& normal = inter_sect_info._Normal;

	//// 今回は法線の方向を再計算する
	//auto relativeVel = (_Velocity + _AngVel.Cross(r1)) - (otherRigidBody->GetVelocity() + otherRigidBody->_AngVel.Cross(r2));
	//relativeVel.Normalize();

	//const float impulse = -(1.0f + elasticity) * relativeVel.Dot(normal) / (_Mass + otherRigidBody->_Mass);


	//const SimpleMath::Vector3 impulseVec = relativeVel * impulse;

	//_Velocity = impulseVec;
	//otherRigidBody->_Velocity = impulseVec * -1.0f;

	//AddImpulse(impulseVec);
	//otherRigidBody->AddImpulse(impulseVec * -1.0f);


	float invMass1 = InvMass();
	float invMass2 = otherRigidBody->InvMass();

	float invMassSum = invMass1 + invMass2;

	// お互い、まったく質量を持たない場合は計算できない
	if (invMassSum == 0.0f) { return; }

	SimpleMath::Vector3 r1 = inter_sect_info._InterSectPositionB - inter_sect_info._PoisitionA;
	SimpleMath::Vector3 r2 = inter_sect_info._InterSectPositionB - inter_sect_info._PoisitionB;

	auto i1 = InvTensor();
	auto i2 = otherRigidBody->InvTensor();

	//auto relativeVel = otherRigidBody->_Velocity - _Velocity;

	// 角運動量と移動量を加えたもの
	auto relativeVel = (otherRigidBody->_Velocity + otherRigidBody->_AngVel.Cross(r2))
		- (_Velocity + _AngVel.Cross(r1));

	auto relativeNorm = inter_sect_info._Normal;
	relativeNorm.Normalize();

	if (relativeVel.Dot(relativeNorm) > 0.0f)
	{
		return;
	}


	// 反発係数
	float e = fminf(_Elasticty, otherRigidBody->_Elasticty);
	float numerator = (-(1.0f + e) * relativeVel.Dot(relativeNorm));

	float d1 = invMassSum;
	auto d2 = SimpleMath::Vector3::Transform(r1.Cross(relativeNorm), i1).Cross(r1);
	auto d3 = SimpleMath::Vector3::Transform(r2.Cross(relativeNorm), i2).Cross(r2);

	float denominator = d1 + relativeNorm.Dot(d2 + d3);

	//float j = numerator / invMassSum;

	// ゼロ除算防止
	float j = (denominator == 0.0f) ? 0.0f : numerator / denominator;

	auto impulse = relativeNorm * j;
	SetVelocity(_Velocity - impulse * invMass1);
	otherRigidBody->SetVelocity(otherRigidBody->_Velocity + impulse * invMass2);

	SetAngVel(_AngVel - SimpleMath::Vector3::Transform(r1.Cross(impulse), i1));
	otherRigidBody->SetAngVel(otherRigidBody->_AngVel + SimpleMath::Vector3::Transform(r2.Cross(impulse), i2));



	// 衝突法線
	auto t = relativeVel - (relativeNorm * relativeVel.Dot(relativeNorm));
	t.Normalize();

	numerator = -relativeVel.Dot(t);
	d1 = invMassSum;
	d2 = SimpleMath::Vector3::Transform(r1.Cross(t), i1).Cross(r1);
	d3 = SimpleMath::Vector3::Transform(r2.Cross(t), i2).Cross(r2);
	denominator = d1 + t.Dot(d2 + d3);

	//float jt = numerator / invMassSum;
	float jt = numerator / denominator;

	// 摩擦による計算
	float friction = sqrtf(_Friction * otherRigidBody->_Friction);

	if (jt > j * friction)
	{
		jt = j * friction;
	}
	else if (jt < -j * friction)
	{
		jt = -j * friction;
	}


	auto tangentImpuse = t * jt;

	SetVelocity(_Velocity - tangentImpuse * invMass1);
	otherRigidBody->SetVelocity(otherRigidBody->_Velocity + tangentImpuse * invMass2);
	SetAngVel(_AngVel - SimpleMath::Vector3::Transform(r1.Cross(tangentImpuse), i1));
	otherRigidBody->SetAngVel(otherRigidBody->_AngVel + SimpleMath::Vector3::Transform(r2.Cross(tangentImpuse), i2));
}

void RigidBodyComponent::CalculateMoment(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody,
	InterSectInfo& inter_sect_info, int count)
{
	float invMass1 = InvMass();
	float invMass2 = otherRigidBody->InvMass();

	float invMassSum = invMass1 + invMass2;

	// お互い、まったく質量を持たない場合は計算できない
	if (invMassSum == 0.0f) { return; }

	SimpleMath::Vector3 r1 = inter_sect_info._InterSectPositions[count] - inter_sect_info._PoisitionA;
	SimpleMath::Vector3 r2 = inter_sect_info._InterSectPositions[count] - inter_sect_info._PoisitionB;

	auto i1 = InvTensor();
	auto i2 = otherRigidBody->InvTensor();

	//auto relativeVel = otherRigidBody->_Velocity - _Velocity;
	//auto relativeVel = (_Velocity + _AngVel.Cross(r1)) - (otherRigidBody->_Velocity + otherRigidBody->_AngVel.Cross(r2));
	auto relativeVel = (otherRigidBody->_Velocity + otherRigidBody->_AngVel.Cross(r2)) - (_Velocity + _AngVel.Cross(r1));

	auto relativeNorm = inter_sect_info._Normal;
	relativeNorm.Normalize();



	if (relativeVel.Dot(relativeNorm) >= 0.0f)
	{
		return;
	}


	// 反発係数
	float e = fminf(_Elasticty, otherRigidBody->_Elasticty);
	float numerator = (-(1.0f + e) * relativeVel.Dot(relativeNorm));

	float d1 = invMassSum;
	auto d2 = SimpleMath::Vector3::Transform(r1.Cross(relativeNorm), i1).Cross(r1);
	auto d3 = SimpleMath::Vector3::Transform(r2.Cross(relativeNorm), i2).Cross(r2);

	float denominator = d1 + relativeNorm.Dot(d2 + d3);

	//float j = numerator / invMassSum;

	// ゼロ除算防止
	float j = (denominator == 0.0f) ? 0.0f : numerator / denominator;

	if (inter_sect_info._InterSectPositions.size() > 0 && j != 0.0f)
	{
		j /= (float)inter_sect_info._InterSectPositions.size();
	}

	auto impulse = relativeNorm * j;
	SetVelocity(_Velocity - impulse * invMass1);
	otherRigidBody->SetVelocity(otherRigidBody->_Velocity + impulse * invMass2);

	SetAngVel(_AngVel - SimpleMath::Vector3::Transform(r1.Cross(impulse), i1));
	otherRigidBody->SetAngVel(otherRigidBody->_AngVel + SimpleMath::Vector3::Transform(r2.Cross(impulse), i2));

	// 衝突法線
	auto t = relativeVel - (relativeNorm * relativeVel.Dot(relativeNorm));
	t.Normalize();

	numerator = -relativeVel.Dot(t);
	d1 = invMassSum;
	d2 = SimpleMath::Vector3::Transform(r1.Cross(t), i1).Cross(r1);
	d3 = SimpleMath::Vector3::Transform(r2.Cross(t), i2).Cross(r2);
	denominator = d1 + t.Dot(d2 + d3);

	//float jt = numerator / invMassSum;
	float jt = numerator / denominator;

	if (inter_sect_info._InterSectPositions.size() > 0 && jt != 0.0f)
	{
		jt /= (float)inter_sect_info._InterSectPositions.size();
	}

	// 摩擦による計算
	float friction = sqrtf(_Friction * otherRigidBody->_Friction);

	if (jt > j * friction)
	{
		jt = j * friction;
	}
	else if (jt < -j * friction)
	{
		jt = -j * friction;
	}


	auto tangentImpuse = t * jt;

	SetVelocity(_Velocity - tangentImpuse * invMass1);
	otherRigidBody->SetVelocity(otherRigidBody->_Velocity + tangentImpuse * invMass2);
	SetAngVel(_AngVel - SimpleMath::Vector3::Transform(r1.Cross(tangentImpuse), i1));
	otherRigidBody->SetAngVel(otherRigidBody->_AngVel + SimpleMath::Vector3::Transform(r2.Cross(tangentImpuse), i2));
}

// ���̓��m�̉����o��
void RigidBodyComponent::ResolveContact(Actor* other, std::shared_ptr<RigidBodyComponent> otherRigidBody, InterSectInfo& inter_sect_info)
{
	const int interSectPositionCount = inter_sect_info._InterSectPositions.size();

	// 複数の交点がある場合は、OBBtoOBB
	if (interSectPositionCount > 0)
	{
		for (int i = 0; i < interSectPositionCount; ++i)
		{
			CalculateMoment(other, otherRigidBody, inter_sect_info, i);

			Update();
			otherRigidBody->Update();
		}

		float totalMass = InvMass() + otherRigidBody->InvMass();

		float depth = fmaxf(inter_sect_info.depth - 0.01f, 0.0f);
		float scalar = depth / totalMass;
		auto correction = (inter_sect_info._Normal * scalar);

		UpdateActorPosition(_user->GetPosition() - correction * InvMass());
		otherRigidBody->UpdateActorPosition(otherRigidBody->_user->GetPosition() + correction * otherRigidBody->InvMass());

		return;
	}

	CalculateMoment(other, otherRigidBody, inter_sect_info);

	Update();
	otherRigidBody->Update();

	const float tA = _Mass / (_Mass + otherRigidBody->GetMass());
	const float tB = otherRigidBody->GetMass() / (_Mass + otherRigidBody->GetMass());

	auto position = inter_sect_info._InterSectPositionA;
	auto otherPosition = inter_sect_info._InterSectPositionB;

	const SimpleMath::Vector3 ds = otherPosition - position;

	UpdateActorPosition(_user->GetPosition() + ds * tA);
	otherRigidBody->UpdateActorPosition(other->GetPosition() - ds * tB);
}

SimpleMath::Matrix RigidBodyComponent::InvTensor(SphereCollisionComponent* sphere_collision_component)
{
	float ix = 0.0f;
	float iy = 0.0f;
	float iz = 0.0f;
	float iw = 1.0f;

	if (_Mass != 0.0f)
	{
		float r = sphere_collision_component->GetRadius();
		float r2 = r * r;
		float fraction = (2.0f / 5.0f);

		ix = r2 * _Mass * fraction;
		iy = r2 * _Mass * fraction;
		iz = r2 * _Mass * fraction;

	}
	else
	{
		return SimpleMath::Matrix::Identity;
	}


	auto result = SimpleMath::Matrix(
		ix, 0, 0, 0,
		0, iy, 0, 0,
		0, 0, iz, 0,
		0, 0, 0, iw);

	return result.Invert();
}


SimpleMath::Matrix RigidBodyComponent::InvTensor(OBBCollisionComponent* obb_collision_component)
{
	float ix = 0.0f;
	float iy = 0.0f;
	float iz = 0.0f;
	float iw = 0.0f;

	if (_Mass != 0.0f)
	{
		SimpleMath::Vector3 size = obb_collision_component->GetSize() * 2.0f;

		float fraction = (1.0f / 12.0f);

		float x2 = size.x * size.x;
		float y2 = size.y * size.y;
		float z2 = size.z * size.z;

		ix = (y2 + z2) * _Mass * fraction;
		iy = (x2 + z2) * _Mass * fraction;
		iz = (x2 + y2) * _Mass * fraction;

		iw = 1.0f;
	}
	else
	{
		return SimpleMath::Matrix::Identity;
	}

	auto result = SimpleMath::Matrix(
		ix, 0, 0, 0,
		0, iy, 0, 0,
		0, 0, iz, 0,
		0, 0, 0, iw);

	return result.Invert();
}

SimpleMath::Matrix RigidBodyComponent::InvTensor()
{
	auto type = _CollisionComponent->GetCollisionType();

	if (type == CollisionType_Sphere)
	{
		return InvTensor(static_cast<SphereCollisionComponent*>(_CollisionComponent));
	}

	if (type == CollisionType_OBB)
	{
		return InvTensor(static_cast<OBBCollisionComponent*>(_CollisionComponent));
	}

	return SimpleMath::Matrix::Identity;
}


void RigidBodyComponent::UpdateActorPosition(const SimpleMath::Vector3 pos)
{
	if (_isStaticPosition)
	{
		return;
	}

	_user->SetPosition(pos);
}

float RigidBodyComponent::InvMass()
{
	if (_Mass == 0.0f) { return 0.0f; }

	return 1.0f / _Mass;
}

void RigidBodyComponent::SetVelocity(const SimpleMath::Vector3& velocity)
{
	if (_isStaticPosition)
	{
		return;
	}

	_Velocity = velocity;
}

void RigidBodyComponent::SetAngVel(const SimpleMath::Vector3& angVel)
{
	if (_isStaticRotate)
	{
		return;
	}
	_AngVel = angVel;
}

