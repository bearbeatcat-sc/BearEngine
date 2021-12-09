#include "Cube.h"

#include "Utility/Random.h"
#include "Utility/Timer.h"

#include "Components/Collsions/CollisionManager.h"
#include "Components/Collsions/OBBCollisionComponent.h"
#include "Components/Physics/RigidBodyComponent.h"
#include "Device/DirectX/DirectXInput.h"
#include "Device/Raytracing/DXRPipeLine.h"
#include "Utility/LogSystem.h"


Cube::Cube(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale, float destroyTime, const std::string& meshName, bool moveFlag)
	:_initScale(scale), _IsGenerate(false), _IsMove(moveFlag), _DXRMeshName(meshName), _IsStaticPosition(false),Actor(), _IsStaticRotate(false)
{
	SetPosition(pos);
	SetScale(scale);

	_DestroyTimer = std::make_shared<Timer>(destroyTime);
	_GenerateTimer = std::make_shared<Timer>(0.3f);

	_Acc = SimpleMath::Vector3(0, 4.0f, 0.0f);
}

void Cube::OnStatic(bool posflag, bool rotateFlag)
{
	_IsStaticPosition = posflag;
	_IsStaticRotate = rotateFlag;
}

void Cube::UpdateActor()
{
	//if (!_IsMove)return;

	//_Acc += SimpleMath::Vector3(0, -2.0f, 0) * Time::DeltaTime;
	//m_Position += Time::DeltaTime * _Acc;

	//if (!_IsGenerate)
	//{
	//	Generate();
	//	return;
	//}

	//_DestroyTimer->Update();
	//if (_DestroyTimer->IsTime())
	//{
	//	_instance->Destroy();
	//	Destroy();
	//}

	//m_Scale = SimpleMath::Vector3::Lerp(_initScale, SimpleMath::Vector3(0.0f), _DestroyTimer->GetRatio());
	//auto mtx = SimpleMath::Matrix::CreateFromQuaternion(m_Rotation) * SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Position);
	auto mtx = GetWorldMatrix();
	_instance->SetMatrix(mtx);

	if(DirectXInput::GetInstance().IsKeyDown(DIK_SPACE))
	{
		_rigidBodyComponent->AddAngularImpulse(SimpleMath::Vector3(0, 0.1f, 0), SimpleMath::Vector3(0,0,1));
	}
}

void Cube::Generate()
{
	_GenerateTimer->Update();

	if (_GenerateTimer->IsTime())
	{
		_GenerateTimer->Reset();
		_IsGenerate = true;
		return;
	}

	m_Scale = SimpleMath::Vector3::Lerp(SimpleMath::Vector3(0.0f), _initScale, _GenerateTimer->GetRatio() * 1.1f);
	auto mtx = GetWorldMatrix();
	_instance->SetMatrix(mtx);
}

void Cube::Init()
{
	_instance = DXRPipeLine::GetInstance().AddInstance(_DXRMeshName, 0);


	auto mtx = GetWorldMatrix();;
	_instance->SetMatrix(mtx);
	_instance->CreateRaytracingInstanceDesc();

	m_pCollisionComponent = new OBBCollisionComponent(this, GetPosition(), m_Scale, "Object");
	//m_pCollisionComponent = new SphereCollisionComponent(this, 10.0f, "Object");


	CollisionManager::GetInstance().AddComponent(m_pCollisionComponent);
	CollisionManager::GetInstance().AddRegistTree(m_pCollisionComponent);
	_rigidBodyComponent = std::make_shared<RigidBodyComponent>(this,m_pCollisionComponent);
	AddComponent(_rigidBodyComponent);
	m_pCollisionComponent->RegistRigidBody(_rigidBodyComponent);
	_rigidBodyComponent->_Mass = 1.0f;
	_rigidBodyComponent->_Elasticty = 1.0f;
	_rigidBodyComponent->_AddGravity = SimpleMath::Vector3(-0.6f, 0.0f, 0.0f);

	_rigidBodyComponent->SetStaticFlag(_IsStaticPosition, _IsStaticRotate);

	SetTag("Block");
}

void Cube::Shutdown()
{
	m_pCollisionComponent->Delete();
}

void Cube::OnCollsion(Actor* other)
{
	if (other->IsContainsTag("Block"))
	{
		LogSystem::AddLog("Hit");
	}
}
