#include "Flor.h"

#include <algorithm>

#include "Cube.h"
#include "Plane.h"
#include "Components/Collsions/CollisionManager.h"
#include "Game_Object/ActorManager.h"
#include "Utility/Random.h"

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
	const SimpleMath::Vector3 basePos = SimpleMath::Vector3(-5.5f, -2.5f, -5.5f);


	for (int z = 0; z < grid_size_z; ++z)
	{
		//const float x = i % grid_size_x;
		//const float z = i / grid_size_z;

		//float rand_x = Random::GetRandom(-1.0f, 1.0f);
		//float rand_z = Random::GetRandom(-1.0f, 1.0f);
		//float rand_y = Random::GetRandom(-1.0f, 1.0f);

		for (int x = 0; x < grid_size_x; ++x)
		{



			//float pos_x = Random::GetRandom(-10.0f, 10.0f);
			//float pos_y = Random::GetRandom(-10.0f, 10.0f);
			//float pos_z = Random::GetRandom(-10.0f, 10.0f);


			bool flag = ((grid_size_x * z) + x) % 2;


			if (flag)
			{
				auto cube = new Cube(basePos + (SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "WhiteCube", false);
				cube->SetActorName("Cube");
				ActorManager::GetInstance().AddActor(cube);
				//SetChild(cube);
			}
			else
			{
				auto cube = new Cube(basePos + (SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "GrayCube", false);
				cube->SetActorName("Cube");
				ActorManager::GetInstance().AddActor(cube);

				//SetChild(cube);
			}
		}


	}

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


