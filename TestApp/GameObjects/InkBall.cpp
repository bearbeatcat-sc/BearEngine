#include "InkBall.h"

#include "Utility/CameraManager.h"
#include "Components/MeshComponent.h"
#include "Components/Collsions/AABBCollsionComponent.h"
#include "Components/Collsions/CollisionManager.h"
#include "Device/ParticleSystems/ParticleEmitter.h"
#include "Utility/LogSystem.h"

#include "InkManager.h"
#include "Utility/Time.h"

InkBall::InkBall(const Vector3& pos, const Vector3& vel,const Vector3& acc,InkManager* pInkManager)
	:Actor(),m_pInkManager(pInkManager), m_Velocity(vel),m_Acc(acc)
{
	SetPosition(pos);	
}

InkBall::~InkBall()
{
}

void InkBall::Move()
{
	auto pos = GetPosition();

	m_Velocity += m_Acc * Time::DeltaTime;
	pos += m_Velocity * Time::DeltaTime;

	SetPosition(pos);
}

void InkBall::UpdateActor()
{
	Move();
	m_pMeshComponent->SetMatrix(GetWorldMatrix());
}

void InkBall::Init()
{
	m_pMeshComponent = std::make_shared<MeshComponent>(this, CameraManager::GetInstance().GetMainCamera(), 32, "NormalMeshEffect");
	m_pMeshComponent->SetMatrix(GetWorldMatrix());
	AddComponent(m_pMeshComponent);

	m_pCollisionComponent = new AABBCollisionComponent(this, GetPosition(), m_Scale, "Ball");
	CollisionManager::GetInstance().AddComponent(m_pCollisionComponent);
	CollisionManager::GetInstance().AddRegistTree(m_pCollisionComponent);	
}

void InkBall::Shutdown()
{
	m_pCollisionComponent->Delete();
}

void InkBall::OnCollsion(Actor* other)
{
	if(other->IsContainsTag("Object"))
	{
		m_pInkManager->SetPosition(m_Position);
		m_pInkManager->AddParticle(1000);

		LogSystem::AddLog("Hit!");
		Destroy();
	}
}
