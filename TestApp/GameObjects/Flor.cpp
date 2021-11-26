﻿#include "Flor.h"


#include "Cube.h"
#include "Plane.h"
#include "Components/Collsions/CollisionManager.h"
#include "Utility/Random.h"
#include "Components/Collsions/OBBCollisionComponent.h"
#include "Components/Physics/RigidBodyComponent.h"

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
				auto cube = new Cube( (SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "WhiteCube", false);
				cube->SetActorName("Cube");
				//ActorManager::GetInstance().AddActor(cube);
				SetChild(cube);
			}
			else
			{
				auto cube = new Cube( (SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "GrayCube", false);
				cube->SetActorName("Cube");
				//ActorManager::GetInstance().AddActor(cube);

				SetChild(cube);
			}
		}


	}

	SetTag("Object");

	m_pCollisionComponent = new OBBCollisionComponent(this,basePos,SimpleMath::Vector3(2.0f * grid_size_x, 1, 2.0f * grid_size_z), "Object");
	CollisionManager::GetInstance().AddComponent(m_pCollisionComponent);
	CollisionManager::GetInstance().AddRegistTree(m_pCollisionComponent);
	_rigidBodyComponent = std::make_shared<RigidBodyComponent>(this);
	AddComponent(_rigidBodyComponent);
	m_pCollisionComponent->RegistRigidBody(_rigidBodyComponent);
	_rigidBodyComponent->_AddGravity = SimpleMath::Vector3::Zero;
	
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


