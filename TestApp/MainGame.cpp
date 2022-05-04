#include "MainGame.h"

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
#include <Device/Rendering/RenderingPipeline.h>
#include <Components/Collsions/CollisionTagManager.h>

#include "Device/Raytracing/DXRMeshData.h"
#include "GameObjects/Flor.h"
#include "Utility/Time.h"


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

	MeshManager::GetInstance().GetSpehereMesh(12, "NormalMeshEffect");
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "blenderMonkey.obj", "BlenderMonkey");
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "cube0.obj", "Cube0");
	TextureManager::GetInstance().AddTexture("Resources/bear.png", "Bear");
	TextureManager::GetInstance().AddTexture("Resources/flog.png", "Flog");
	TextureManager::GetInstance().AddTexture("Resources/TestUI.png", "TestUI");

	auto cubeMeshData = MeshManager::GetInstance().GetMeshData("CubeModelData");
	cubeMeshData->SetPhysicsBaseMaterial(PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 0.0f, 0.0f, 1.0f), SimpleMath::Vector4(1.0f, 10.0f, 1.0f, 0.5f), 0.5f));

	auto sphereMeshData = MeshManager::GetInstance().FindSpehere(12);
	auto planeMeshData = MeshManager::GetInstance().GetPlaneMeshData();
	//sphereMeshData->SetRaytraceMaterial(MeshData::RaytraceMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f));

	auto blenderMonkyMeshData = MeshManager::GetInstance().GetMeshData("BlenderMonkey");
	blenderMonkyMeshData->SetPhysicsBaseMaterial(PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(1.0f, 10.0f, 1.0f, 1.0f), 1.0f));


	//sphereMeshData->SetTestMaterial(MeshData::TestMat{ true });
	_blenderMonkyMaterial = PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.6f), 0.8f, 1.0f, 1.22f);


	//_test = DXRPipeLine::GetInstance().AddMeshData(planeMeshData, L"HitGroup", "WhiteCube", _blenderMonkyMaterial,"TestUI");
	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "BlackCube", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f));
	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "WhiteCube", PhysicsBaseMaterial(SimpleMath::Vector4(0.8f, 0.8f, 0.8f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f));

	DXRPipeLine::GetInstance().AddMeshData(blenderMonkyMeshData, L"HitGroup", "RoughCube", PhysicsBaseMaterial(SimpleMath::Vector4(0.8f, 0.8f, 0.8f, 1.0f), SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f));

	DXRPipeLine::GetInstance().AddMeshData(sphereMeshData, L"HitGroup", "Sphere", PhysicsBaseMaterial(SimpleMath::Vector4(0.1f,0.1f,0.1f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f));
	_blenderMonkey = DXRPipeLine::GetInstance().AddMeshData(sphereMeshData, L"HitGroup", "Sphere2", _blenderMonkyMaterial,"Bear");

	// 必ず、メッシュデータを追加してからパイプラインの初期化を行う。
	DXRPipeLine::GetInstance().InitPipeLine();



	auto dirlight = LightManager::GetInstance().GetDirectionalLight();
	auto dir = SimpleMath::Vector3(-0.964f, -0.136f, -0.185f);
	dirlight->UpdateDirectionalLight(dir, SimpleMath::Color(1, 1, 1, 1));

	m_CameraAsistant = new CameraAsistant();


	const SimpleMath::Vector3 base_position = Vector3(2.5f, 0.0f, 2.5f);

	RenderingPipeLine::GetInstance().SetSkyBox("Resources/OutputCube.dds", SimpleMath::Vector3(10.0f));
	RenderingPipeLine::GetInstance().SetDrawFluidFlag(false);


	std::vector<bool> ObjectCollTable =
	{
		true,
	};
	CollisionTagManagaer::GetInstance().AddTag("Object", ObjectCollTable);


	auto meshData = MeshManager::GetInstance().FindSpehere(6);

	_AddTimer = std::make_shared<Timer>(3.0f);
	_GenerateTimer = std::make_shared<Timer>(0.0001f);
	_IsGenerate = false;


	const int grid_size_x = 5;
	const int grid_size_z = 5;

	const int grid_Count = grid_size_x * grid_size_z;


	//auto sphere = new Sphere(SimpleMath::Vector3(-2.0f, 0.0f, 0.6f), Sphere::SphereType_Normal);
	//sphere->SetScale(SimpleMath::Vector3(1.0f));
	//sphere->SetRotation(SimpleMath::Vector3(3.0f, 0.0f, 0.0f));
	//ActorManager::GetInstance().AddActor(sphere);

	for(int i = 0; i < grid_Count; ++i)
	{
		const float x = (i % 5) * 2.0f;
		const float z = (i / 5) * 2.0f;
		const float y = 0.0f;

		const bool isBlack = i % 2;

		if(isBlack)
		{
			auto floor = new Cube(SimpleMath::Vector3(x, y, z), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "WhiteCube", false);
			floor->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
			floor->SetActorName("Floor");
			ActorManager::GetInstance().AddActor(floor);

			continue;
		}

		auto floor = new Cube(SimpleMath::Vector3(x, y, z), SimpleMath::Vector3(1.0f, 1, 1.0f), 300.0f, "BlackCube", false);
		floor->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
		floor->SetActorName("Floor");
		ActorManager::GetInstance().AddActor(floor);
	}




	//auto wall = new Cube(SimpleMath::Vector3(2.830f, 2.240f, 8.840f), SimpleMath::Vector3(grid_size_x, 6, 1), 300.0f, "WhiteCube", false);
	//ActorManager::GetInstance().AddActor(wall);
	//wall->SetActorName("Wall");
	//wall->SetRotation(SimpleMath::Vector3(0, 0, -0.250f));
	//wall->OnStatic(true,true);

	//auto wall2 = new Cube(SimpleMath::Vector3(11, 4.5f, 7.260f), SimpleMath::Vector3(grid_size_x, 6, 1), 300.0f, "WhiteCube", false);
	//ActorManager::GetInstance().AddActor(wall2);
	//wall2->SetActorName("Wall");
	//wall2->SetRotation(SimpleMath::Vector3(0.0f, 0.0f, 0));
	//wall2->OnStatic(true, true);

	//auto leftWall = new Cube(SimpleMath::Vector3(-7.0f, 3.0f, -0.0f), SimpleMath::Vector3(1, 6, grid_size_x), 300.0f, "RoughCube", false);
	//ActorManager::GetInstance().AddActor(leftWall);

	//auto rightWall = new Cube(SimpleMath::Vector3(7.0f, 3.0f, -0.0f), SimpleMath::Vector3(1, 6, grid_size_x), 300.0f, "RoughCube", false);
	//ActorManager::GetInstance().AddActor(rightWall);

	//auto topWall = new Cube(SimpleMath::Vector3(0.0f, 8, 0.0f), SimpleMath::Vector3(grid_size_x, 1, grid_size_x), 300.0f, "RoughCube", false);
	//ActorManager::GetInstance().AddActor(topWall);

	//auto backWall = new Cube(SimpleMath::Vector3(1.5f, 3.0f, -9.0f), SimpleMath::Vector3(grid_size_x, 6, 1), 300.0f, "RoughCube", false);
	//ActorManager::GetInstance().AddActor(backWall);


	//auto sphere2 = new Sphere(SimpleMath::Vector3(3.0f, -150.0f, 3.0f), Sphere::SphereType_NormalLowPoly);
	//sphere2->SetScale(SimpleMath::Vector3(300.0f));
	//sphere2->SetRotation(SimpleMath::Vector3(-2.4f, 0.0f, 0.0f));
	//ActorManager::GetInstance().AddActor(sphere2);

	//auto cube = new Cube(SimpleMath::Vector3(4.5f, 1.0f, -0.810f), SimpleMath::Vector3(1.0f, 1.0f, 1.0f), 300.0f, "RoughCube", false);
	//cube->SetRotation(SimpleMath::Vector3(0, -0.1f, -0.3f));
	//cube->SetActorName("Cube");
	//cube->OnStatic(true, false);
	//ActorManager::GetInstance().AddActor(cube);


	//for (int i = 0; i < 200; ++i)
	//{
	//	auto pos_x = Random::GetRandom(-6.0f, 6.0f);
	//	auto pos_z = Random::GetRandom(-8.0f, 8.0f);
	//	float pos_y = 2.0f;

	//	float rotateX = Random::GetRandom(-1.0f, 1.0f);
	//	float rotateY = Random::GetRandom(-1.0f, 1.0f);
	//	float rotateZ = Random::GetRandom(-1.0f, 1.0f);

	//	float scale = 0.1f;

	//	//auto sphere = new Sphere(SimpleMath::Vector3((i / 8) * 1.0f, pos_y, (i % 8) * 1.0f), Sphere::SphereType_Normal);
	//	auto sphere = new Sphere(SimpleMath::Vector3(pos_x,pos_y,pos_z), Sphere::SphereType_Normal);
	//	sphere->SetScale(SimpleMath::Vector3(scale));
	//	sphere->SetRotation(SimpleMath::Vector3(rotateX, rotateY, rotateZ));
	//	ActorManager::GetInstance().AddActor(sphere);

	//	//_GenerateCount++;

	//	//if (_GenerateCount >= _MaxGenerateCount)
	//	//{
	//	//	_IsGenerate = false;
	//	//}
	//}

	auto cube = new Cube(SimpleMath::Vector3(3.770f,3.910f,0), SimpleMath::Vector3(0.3f), 300.0f, "Sphere", false);
	cube->SetActorName("Cube");
	cube->SetRotation(SimpleMath::Vector3(3.0f,0,0));
	cube->SetScale(SimpleMath::Vector3(3.0f));
	ActorManager::GetInstance().AddActor(cube);

	auto cube2 = new Cube(SimpleMath::Vector3(1.190f,2.190f,5.470f), SimpleMath::Vector3(0.3f), 300.0f, "RoughCube", false);
	cube2->SetActorName("Cube");
	cube2->SetRotation(SimpleMath::Vector3(2.440f, 0, 0));
	cube2->SetScale(SimpleMath::Vector3(1.0f));
	ActorManager::GetInstance().AddActor(cube2);

	auto cube3 = new Cube(SimpleMath::Vector3(5.290f,1.830f,1.990f), SimpleMath::Vector3(0.3f), 300.0f, "RoughCube", false);
	cube3->SetActorName("Cube");
	cube3->SetRotation(SimpleMath::Vector3(3.890f, -0.250f, 0));
	cube3->SetScale(SimpleMath::Vector3(1.3f));
	ActorManager::GetInstance().AddActor(cube3);

	//for (int i = 0; i < 10; i++)
	//{
	//	auto pos_x = 1.0f * i / 2;
	//	auto pos_z = 1.0f * (i % 2);
	//	float pos_y = 3.0f;

	//	auto cube = new Cube(SimpleMath::Vector3(pos_x, 3.0f, pos_z), SimpleMath::Vector3(0.3f), 300.0f, "RoughCube", false);
	//	cube->SetActorName("Cube");
	//	cube->SetRotation(SimpleMath::Vector3(pos_x, pos_z, pos_y));
	//	cube->OnStatic(true, false);
	//	ActorManager::GetInstance().AddActor(cube);
	//}
}


void MainGame::Update()
{
	m_CameraAsistant->Update();

	//_blenderMonkyMaterial._albedo.x += 0.4f * Time::DeltaTime;
	//_blenderMonkyMaterial._albedo.y += 0.4f * Time::DeltaTime;
	//_blenderMonkyMaterial._albedo.z += 0.4f * Time::DeltaTime;
	//_blenderMonkyMaterial._albedo.x = fmodf(_blenderMonkyMaterial._albedo.x, 1.0f);
	//_blenderMonkyMaterial._albedo.y = fmodf(_blenderMonkyMaterial._albedo.y, 1.0f);
	//_blenderMonkyMaterial._albedo.z = fmodf(_blenderMonkyMaterial._albedo.z,1.0f);

	//_test->UpdateMaterial(_blenderMonkyMaterial);

	if (DirectXInput::GetInstance().IsKeyDown(DIK_SPACE))
	{




	}
	//
	//if (!_IsGenerate)
	//{
	//	_AddTimer->Update();
	//	if (_AddTimer->IsTime())
	//	{
	//		_IsGenerate = true;
	//		_GenerateCount = 0;
	//		_AddTimer->Reset();
	//	}
	//	return;
	//}




	//_GenerateTimer->Update();
	//if (_GenerateTimer->IsTime())
	//{
	//	if (_GenerateCount >= _MaxGenerateCount)
	//	{
	//		_IsGenerate = false;
	//		_GenerateTimer->Reset();
	//		return;
	//	}

	//	auto pos_x = Random::GetRandom(-2.0f, 2.0f);
	//	auto pos_z = Random::GetRandom(-2.0f, 2.0f);
	//	float pos_y = 3.0f;

	//	float scale = Random::GetRandom(0.5f, 1.2f);

	//	int flag = Random::GetRandom(0, 1);


	//	if (flag == 0)
	//	{
	//		//Sphere* sphere = nullptr;
	//		//sphere = new Sphere(SimpleMath::Vector3(pos_x, pos_y, pos_z), Sphere::SphereType_Normal);
	//		//sphere->SetScale(SimpleMath::Vector3(scale));
	//		//ActorManager::GetInstance().AddActor(sphere);
	//		//sphere->Destroy(10.0f);
	//		//sphere->SetActorName("Sphere");
	//		
	//		Sphere* sphere = nullptr;
	//		sphere = new Sphere(SimpleMath::Vector3(pos_x, pos_y, pos_z), Sphere::SphereType_Normal);

	//		sphere->SetScale(SimpleMath::Vector3(scale));
	//		ActorManager::GetInstance().AddActor(sphere);
	//		sphere->Destroy(10.0f);
	//		sphere->SetActorName("Sphere");

	//	}
	//	else if (flag == 1)
	//	{
	//		Sphere* sphere = nullptr;
	//		sphere = new Sphere(SimpleMath::Vector3(pos_x, pos_y, pos_z), Sphere::SphereType_Normal);

	//		sphere->SetScale(SimpleMath::Vector3(scale));
	//		ActorManager::GetInstance().AddActor(sphere);
	//		sphere->Destroy(10.0f);
	//		sphere->SetActorName("Sphere");

	//		//auto cube = new Cube(SimpleMath::Vector3(pos_x, pos_y + 6.0f, pos_z), SimpleMath::Vector3(0.5f), 300.0f, "WhiteCube", false);
	//		//cube->SetActorName("Cube");
	//		//ActorManager::GetInstance().AddActor(cube);
	//		//cube->Destroy(10.0f);
	//		//cube->SetRotation(SimpleMath::Vector3(pos_x, pos_y, pos_z) * scale * pos_y);
	//	}



	//	_GenerateCount++;


	//}



}

void MainGame::Destroy()
{
}
