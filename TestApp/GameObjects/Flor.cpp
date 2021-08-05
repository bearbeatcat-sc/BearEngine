#include "Flor.h"

#include <algorithm>

#include "Plane.h"
#include "Components/Collsions/CollisionManager.h"
#include "Game_Object/ActorManager.h"

Flor::Flor(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale, InkManager* pInkManager)
	:m_pInkManager(pInkManager)
{
	SetPosition(pos);	
}

Flor::~Flor()
{
	
}

void Flor::Init()
{
	const int grid_size_x = 16;
	const int grid_size_z = 16;

	const int grid_Count = grid_size_x * grid_size_z;
	const SimpleMath::Vector3 basePos = SimpleMath::Vector3(-5.5f, -2.5f, -5.5f);

	for (int i = 0; i < grid_Count; ++i)
	{
		float x = i % grid_size_x;
		float z = i / grid_size_z;

		float color = std::clamp((float)(i % 2), 0.5f, 0.8f);

		auto plane = new ::Plane(basePos + (SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(2, 1, 2), m_pInkManager);
		plane->SetColor(SimpleMath::Color(color, color, color));
		plane->SetDrawType(MeshDrawer::Object);
		ActorManager::GetInstance().AddActor(plane);
	}

	m_Scale = SimpleMath::Vector3(grid_size_x * 2, 1, grid_size_z * 2);

	SetPosition(basePos + SimpleMath::Vector3(2 * 8, 0, 2 * 8));
	m_pCollisionComponent = new AABBCollisionComponent(this, basePos + SimpleMath::Vector3(2 * 8,1,2 * 8) , m_Scale, "Object");
	CollisionManager::GetInstance().AddComponent(m_pCollisionComponent);
	CollisionManager::GetInstance().AddRegistTree(m_pCollisionComponent);

	SetTag("Object");
}

void Flor::UpdateActor()
{
	
}

void Flor::Shutdown()
{
	m_pCollisionComponent->Delete();
}

void Flor::OnCollsion(Actor* other)
{
	
}


