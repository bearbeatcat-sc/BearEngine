﻿#include "MainGame.h"

#include "Utility/Camera.h"
#include "Device/DirectX/DirectXInput.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Game_Object/ActorManager.h"
#include "CameraAsistant.h"
#include "GameObjects/Sphere.h"
#include "Device/Raytracing/DXRPipeLine.h"
#include "GameObjects/Cube.h"
#include "Utility/Timer.h"
#include "Utility/Random.h"

#include <imgui/imgui.h>
#include <time.h>
#include <Device/Lights/LightManager.h>
#include <Device/DirectX/Core/Sounds/SoundManager.h>
#include <Device/RenderingPipeline.h>


MainGame::MainGame()
	:Game(L"TestApp", 1920, 1080)
{
}

MainGame::~MainGame()
{
	delete m_CameraAsistant;
}

void MainGame::Init()
{
	DirectXGraphics::GetInstance().InitFontSystem(L"font/fonttest.spritefont");

	auto cubeMeshData = MeshManager::GetInstance().GetMeshData("CubeModelData");
	cubeMeshData->SetPhysicsBaseMaterial(PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 0.0f, 0.0f, 1.0f), SimpleMath::Vector4(1.0f, 10.0f, 1.0f, 0.5f), 0.5f));

	auto sphereMeshData = MeshManager::GetInstance().FindSpehere(12);
	//sphereMeshData->SetRaytraceMaterial(MeshData::RaytraceMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f));

	auto blenderMonkey = MeshManager::GetInstance().GetMeshData("BlenderMonkey");
	blenderMonkey->SetPhysicsBaseMaterial(PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(1.0f, 10.0f, 1.0f, 1.0f), 1.0f));


	//sphereMeshData->SetTestMaterial(MeshData::TestMat{ true });


	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "WhiteCube", PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.5f), 0.1f));
	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "GrayCube", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.5f), 0.1));

	DXRPipeLine::GetInstance().AddMeshData(sphereMeshData, L"HitGroup", "Sphere", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.6f), 0.8f, 0.1f, 2.22f));
	DXRPipeLine::GetInstance().AddMeshData(blenderMonkey, L"HitGroup", "Sphere2", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.6f), 0.8f, 0.2f,1.22f));

	// 必ず、メッシュデータを追加してからパイプラインの初期化を行う。
	DXRPipeLine::GetInstance().InitPipeLine();



	auto dirlight = LightManager::GetInstance().GetDirectionalLight();
	auto dir = SimpleMath::Vector3(-0.103f, -0.513f, -0.852f);
	dirlight->UpdateDirectionalLight(dir, SimpleMath::Color(1, 1, 1, 1));

	auto pointLight = std::make_shared<PointLight>(SimpleMath::Vector3(0, 0, 0), SimpleMath::Color(1, 1, 1, 1), 1.0f, 1.0f);
	LightManager::GetInstance().AddPointLight(pointLight);
	
	m_CameraAsistant = new CameraAsistant();


	const SimpleMath::Vector3 base_position = Vector3(2.5f, 0.0f, 2.5f);

	RenderingPipeLine::GetInstance().SetSkyBox("Resources/OutputCube.dds", SimpleMath::Vector3(10.0f));
	RenderingPipeLine::GetInstance().SetDrawFluidFlag(false);


	//auto meshData = MeshManager::GetInstance().FindSpehere(6);

	_AddTimer = std::make_shared<Timer>(3.0f);
	_GenerateTimer = std::make_shared<Timer>(0.0001f);
	_IsGenerate = false;


	//float floorsize = 30 / 6;

	//for(int z = 0; z < 6; ++z)
	//{
	//	for(int x = 0; x < 6; ++x)
	//	{
	//		auto floor = new Sphere(SimpleMath::Vector3(x, 0, z), Sphere::SphereType_Normal);
	//		floor->SetScale(SimpleMath::Vector3(floorsize, 1, floorsize));
	//		ActorManager::GetInstance().AddActor(floor);
	//	}
	//}


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



			float pos_x = Random::GetRandom(-10.0f, 10.0f);
			float pos_y = Random::GetRandom(-10.0f, 10.0f);
			float pos_z = Random::GetRandom(-10.0f, 10.0f);


			bool flag = ((grid_size_x * z) + x) % 2;
			auto cube = new Cube(basePos + (SimpleMath::Vector3(x, 0, z) * 2.0f), SimpleMath::Vector3(1.0f, 1, 1.0f),300.0f, flag,false);
			ActorManager::GetInstance().AddActor(cube);
		}


	}
	
	auto wall = new Cube(SimpleMath::Vector3(1.5f, 3.0f, 10.0f),SimpleMath::Vector3(grid_size_x,6,1),300.0f,true,false);
	ActorManager::GetInstance().AddActor(wall);

	auto sphere = new Sphere(SimpleMath::Vector3(0, 0.0f, 6.0f), Sphere::SphereType_Normal);
	sphere->SetScale(SimpleMath::Vector3(1.0f));
	sphere->SetRotation(Quaternion::CreateFromYawPitchRoll(3.0f,0.0f,0.0f));
	ActorManager::GetInstance().AddActor(sphere);

	auto sphere2 = new Sphere(SimpleMath::Vector3(6.0f, 0.0f, 3.0f), Sphere::SphereType_NormalLowPoly);
	sphere2->SetScale(SimpleMath::Vector3(1.0f));
	sphere2->SetRotation(Quaternion::CreateFromYawPitchRoll(-2.4f, 0.0f, 0.0f));
	ActorManager::GetInstance().AddActor(sphere2);

	for (int i = 0; i < 241; ++i)
	{
		auto pos_x = Random::GetRandom(-20.0f, 20.0f);
		auto pos_z = Random::GetRandom(-5.0f, 10.0f);
		float pos_y = 2.0f;

		float rotateX = Random::GetRandom(-1.0f, 1.0f);
		float rotateY = Random::GetRandom(-1.0f, 1.0f);
		float rotateZ = Random::GetRandom(-1.0f, 1.0f);

		float scale = 0.4f;

		//auto sphere = new Sphere(SimpleMath::Vector3((i / 8) * 1.0f, pos_y, (i % 8) * 1.0f), Sphere::SphereType_Normal);
		auto sphere = new Sphere(SimpleMath::Vector3(pos_x,pos_y,pos_z), Sphere::SphereType_Normal);
		sphere->SetScale(SimpleMath::Vector3(scale));
		sphere->SetRotation(Quaternion::CreateFromYawPitchRoll(rotateX, rotateY, rotateZ));
		ActorManager::GetInstance().AddActor(sphere);

		//_GenerateCount++;

		//if (_GenerateCount >= _MaxGenerateCount)
		//{
		//	_IsGenerate = false;
		//}
	}

}


void MainGame::Update()
{
	m_CameraAsistant->Update();

	if(!_IsGenerate)
	{
		_AddTimer->Update();
		if (_AddTimer->IsTime())
		{
			_IsGenerate = true;
			_GenerateCount = 0;
			_AddTimer->Reset();
		}
		return;
	}


	
	//_GenerateTimer->Update();
	//if(_GenerateTimer->IsTime())
	//{
	//	_GenerateTimer->Reset();
	//	auto pos_x = Random::GetRandom(-20.0f, 20.0f);
	//	auto pos_z = Random::GetRandom(10.0f, 30.0f);
	//	float pos_y = 2.0f;

	//	float rotateX = Random::GetRandom(-1.0f, 1.0f);
	//	float rotateY = Random::GetRandom(-1.0f, 1.0f);
	//	float rotateZ = Random::GetRandom(-1.0f, 1.0f);

	//	float scale = Random::GetRandom(1.0f, 3.0f);

	//	auto sphere = new Sphere(SimpleMath::Vector3(pos_x, pos_y, pos_z), Sphere::SphereType_Normal);
	//	sphere->SetScale(SimpleMath::Vector3(scale));
	//	sphere->SetRotation(Quaternion::CreateFromYawPitchRoll(rotateX, rotateY, rotateZ));
	//	ActorManager::GetInstance().AddActor(sphere);

	//	_GenerateCount++;

	//	if(_GenerateCount >= _MaxGenerateCount)
	//	{
	//		_IsGenerate = false;
	//	}
	//}



}

void MainGame::Destroy()
{
}
