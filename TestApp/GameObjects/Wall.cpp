#include "Wall.h"

#include <algorithm>

#include "Plane.h"
#include "Components/Collsions/CollisionManager.h"
#include "Game_Object/ActorManager.h"

Wall::Wall(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale,InkManager* pInkManager)
	:m_pInkManager(pInkManager)
{
	SetPosition(pos);
}

Wall::~Wall()
{

}

void Wall::Init()
{
	const int grid_size_x = 11;
	const int grid_size_y = 11;

	const int grid_Count = grid_size_x * grid_size_y;
	const SimpleMath::Vector3 basePos = SimpleMath::Vector3(-5.5f, -3.5f, 5.5f);

	for (int i = 0; i < grid_Count; ++i)
	{
		float x = i % grid_size_x;
		float y = i / grid_size_y;

		float color = std::clamp((float)(i % 2), 0.5f, 0.8f);

		auto plane = new ::Plane(basePos + (SimpleMath::Vector3(x, y, 0) * 2.0f), SimpleMath::Vector3(2, 2, 1), m_pInkManager);
		plane->SetColor(SimpleMath::Color(color, color, color));
		plane->SetDrawType(MeshDrawer::Wall);
		ActorManager::GetInstance().AddActor(plane);

	}

	m_Scale = SimpleMath::Vector3(grid_size_x * 2, grid_size_y * 2, 1);

	SetPosition(basePos + SimpleMath::Vector3(2 * 5.5f, 2 * 5.5f, -1.5f));
	m_pCollisionComponent = new AABBCollisionComponent(this, basePos + SimpleMath::Vector3(2 * 5.5f, 2 * 5.5f, 0), m_Scale, "Object");
	CollisionManager::GetInstance().AddComponent(m_pCollisionComponent);
	CollisionManager::GetInstance().AddRegistTree(m_pCollisionComponent);

	SetTag("Object");
}

void Wall::UpdateActor()
{

}

void Wall::Shutdown()
{
	m_pCollisionComponent->Delete();
}

void Wall::OnCollsion(Actor* other)
{

}


