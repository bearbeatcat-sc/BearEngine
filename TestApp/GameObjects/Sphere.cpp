#include "Sphere.h"

#include <Components/MeshComponent.h>
#include <Utility/CameraManager.h>
#include <Utility/Time.h>
#include <Utility/Random.h>
#include <Components/Collsions/SphereCollisionComponent.h>
#include <Components/Collsions/OBBCollisionComponent.h>

#include "Components/Collsions/CollisionManager.h"
#include "Device/Raytracing/DXRPipeLine.h"
#include "Utility/LogSystem.h"


Sphere::Sphere(Vector3 pos, SphereType type)
	:m_Type(type), m_MoveTime(0.0f)
{
	SetPosition(pos);
}

Sphere::~Sphere()
{
}

void Sphere::SetColor(const DirectX::SimpleMath::Color& color)
{
	mColor = color;
}

void Sphere::UpdateActor()
{
	//Move();
	//m_pMeshComponent->SetMatrix(GetWorldMatrix());
	auto mtx = GetWorldMatrix();
	_instance->SetMatrix(mtx);

}

void Sphere::Init()
{
	//switch (m_Type)
	//{
	//case Sphere::SphereType_Normal:
	//	//SetScale(DirectX::SimpleMath::Vector3(1.0f));
	//	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this, CameraManager::GetInstance().GetMainCamera(), 32, "NormalMeshEffect"));
	//	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	//	break;
	//case Sphere::SphereType_NormalLowPoly:
	//	SetScale(DirectX::SimpleMath::Vector3(1.00f, 1.00f, 1.00f));
	//	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this, "Sphere", CameraManager::GetInstance().GetMainCamera(), "NormalMeshEffect"));
	//	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	//	break;
	//case Sphere::SphereType_GouraudNormal:
	//	SetScale(DirectX::SimpleMath::Vector3(0.063f, 0.063f, 0.063f));
	//	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this, CameraManager::GetInstance().GetMainCamera(), 16, "GouraudMeshEffect"));
	//	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	//	break;
	//case Sphere::SphereType_GouraudLowPoly:
	//	SetScale(DirectX::SimpleMath::Vector3(1.00f, 1.00f, 1.00f));
	//	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this, "Sphere", CameraManager::GetInstance().GetMainCamera(), "GouraudMeshEffect"));
	//	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	//	break;
	//case Sphere::SphereType_NormalMesh:
	//	//SetScale(DirectX::SimpleMath::Vector3(0.063f, 0.063f, 0.063f));
	//	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this, "blenderMonkey", CameraManager::GetInstance().GetMainCamera(), "NormalMeshEffect"));
	//	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	//	break;
	//case Sphere::SphereType_GouraudMesh:
	//	SetScale(DirectX::SimpleMath::Vector3(0.63f, 0.63f, 0.63f));
	//	m_pMeshComponent = std::shared_ptr<MeshComponent>(new MeshComponent(this, "SpaceShip", CameraManager::GetInstance().GetMainCamera(), "GouraudMeshEffect"));
	//	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	//	break;
	//}



	//m_pMeshComponent->SetColor(mColor);
	//AddComponent(m_pMeshComponent);
	//SetScale(DirectX::SimpleMath::Vector3(0.5f));

	m_MoveTime = Random::GetRandom(0.0f, 1.0f);

	m_ActionType = static_cast<ActionType>(Random::GetRandom(1, ActionType_Thrust));

	mInitPos = m_Position;
	mStartPos = mInitPos;

	if(m_Type == SphereType_NormalLowPoly)
	{
		_instance = DXRPipeLine::GetInstance().AddInstance("Sphere2", 0);
		_sphereCollisionComponent = new SphereCollisionComponent(this, m_Scale.x, "Object");
		CollisionManager::GetInstance().AddComponent(_sphereCollisionComponent);
		CollisionManager::GetInstance().AddRegistTree(_sphereCollisionComponent);

	}
	else
	{
		_instance = DXRPipeLine::GetInstance().AddInstance("Sphere", 0);
		_obbCollisionComponent = new OBBCollisionComponent(this, GetPosition(), m_Scale, "Object");
		CollisionManager::GetInstance().AddComponent(_obbCollisionComponent);
		CollisionManager::GetInstance().AddRegistTree(_obbCollisionComponent);
	}
	

	auto mtx = GetWorldMatrix();
	_instance->SetMatrix(mtx);
	_instance->CreateRaytracingInstanceDesc();

	

}

void Sphere::Shutdown()
{
	if(_obbCollisionComponent != nullptr)
		_obbCollisionComponent->Delete();

	if (_sphereCollisionComponent != nullptr)
		_sphereCollisionComponent->Delete();
}

void Sphere::OnCollsion(Actor* other)
{
	LogSystem::AddLog("Hit");
}

void Sphere::UpDown()
{
	auto pos = m_Position;

	//pos.x += cos(m_MoveTime) * Time::DeltaTime * 2.0f;
	pos.y += cos(m_MoveTime) * Time::DeltaTime * 1.2f;
	m_MoveTime += Time::DeltaTime;

	SetPosition(pos);
	//m_pMeshComponent->SetMatrix(GetWorldMatrix());
}

void Sphere::Thrust()
{
	auto pos = m_Position;

	//pos.x += cos(m_MoveTime) * Time::DeltaTime * 2.0f;
	pos.z += Time::DeltaTime * 1.2f;

	SetPosition(pos);
	//m_pMeshComponent->SetMatrix(GetWorldMatrix());

	float distance = Vector3::Distance(pos, mStartPos);

	if (distance >= 16.0f)
	{
		mStartPos = mInitPos * Random::GetRandom(-1.2f, 1.2f);
		m_Position = mStartPos;
	}

}

void Sphere::Move()
{
	switch (m_ActionType)
	{
	case ActionType_UpDown:
		UpDown();
		break;

	case ActionType_Thrust:
		Thrust();
		break;
	default:;
	}


}
