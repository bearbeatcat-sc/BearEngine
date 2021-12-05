#include "Flor.h"


#include "Cube.h"
#include "Plane.h"
#include "Components/Collsions/CollisionManager.h"
#include "Utility/Random.h"
#include "Components/Collsions/OBBCollisionComponent.h"
#include "Components/Collsions/SphereCollisionComponent.h"
#include "Components/Physics/RigidBodyComponent.h"
#include "Utility/LogSystem.h"

Flor::Flor(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale)
	:Actor()
{
	SetPosition(pos);	
}

Flor::~Flor()
{
	
}

void Flor::Init()
{
	SetActorName("Flor");

	const int grid_size_x = 8;
	const int grid_size_z = 7;

	const int grid_Count = grid_size_x * grid_size_z;
	const SimpleMath::Vector3 basePos = GetPosition();


	//for (int z = -grid_size_z * 0.5f; z < grid_size_z; ++z)
	//{
	//	//const float x = i % grid_size_x;
	//	//const float z = i / grid_size_z;

	//	//float rand_x = Random::GetRandom(-1.0f, 1.0f);
	//	//float rand_z = Random::GetRandom(-1.0f, 1.0f);
	//	//float rand_y = Random::GetRandom(-1.0f, 1.0f);

	//	for (int x = -grid_size_x * 0.5f; x < grid_size_x; ++x)
	//	{



	//		//float pos_x = Random::GetRandom(-10.0f, 10.0f);
	//		//float pos_y = Random::GetRandom(-10.0f, 10.0f);
	//		//float pos_z = Random::GetRandom(-10.0f, 10.0f);


	//		bool flag = ((grid_size_x * z) + x) % 2;


	//		if (flag)
	//		{
	//			auto cube = new Cube((SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "WhiteCube", false);
	//			cube->SetActorName("Cube");
	//			//ActorManager::GetInstance().AddActor(cube);
	//			SetChild(cube);
	//		}
	//		else
	//		{
	//			auto cube = new Cube((SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "GrayCube", false);
	//			cube->SetActorName("Cube");
	//			//ActorManager::GetInstance().AddActor(cube);

	//			SetChild(cube);
	//		}
	//	}


	//}

	auto cube = new Cube(SimpleMath::Vector3::Zero, SimpleMath::Vector3(grid_size_x, 1, grid_size_z), 300.0f, "GrayCube", false);
	cube->SetActorName("Cube");
	//ActorManager::GetInstance().AddActor(cube);
	SetChild(cube);
	SetTag("Object");


}

void Flor::UpdateActor()
{
	
}

void Flor::Shutdown()
{
}

void Flor::OnCollsion(Actor* other)
{

}


