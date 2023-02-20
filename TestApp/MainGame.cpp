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
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "cat.obj", "Cat");
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "cube0.obj", "Cube0");
	TextureManager::GetInstance().AddTexture("Resources/bear.png", "Bear");
	TextureManager::GetInstance().AddTexture("Resources/flog.png", "Flog");
	TextureManager::GetInstance().AddTexture("Resources/TestUI.png", "TestUI");

	auto cubeMeshData = MeshManager::GetInstance().GetMeshData("CubeModelData");
	cubeMeshData->SetPhysicsBaseMaterial(PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 0.0f, 0.0f, 1.0f), SimpleMath::Vector4(1.0f, 10.0f, 1.0f, 0.5f), 0.5f));

	auto sphereMeshData = MeshManager::GetInstance().FindSpehere(12);
	auto planeMeshData = MeshManager::GetInstance().GetPlaneMeshData();
	//sphereMeshData->SetRaytraceMaterial(MeshData::RaytraceMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f));

	auto catMeshData = MeshManager::GetInstance().GetMeshData("Cat");
	catMeshData->SetPhysicsBaseMaterial(PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(1.0f, 10.0f, 1.0f, 1.0f), 1.0f));


	//sphereMeshData->SetTestMaterial(MeshData::TestMat{ true });
	_blenderMonkyMaterial = PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.6f), 0.8f, 1.0f, 1.22f);


	//_test = DXRPipeLine::GetInstance().AddMeshData(planeMeshData, L"HitGroup", "WhiteCube", _blenderMonkyMaterial,"TestUI");
	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "GrayCube", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 0.0f), 1.0f));
	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "WhiteCube", PhysicsBaseMaterial(SimpleMath::Vector4(0.4f, 0.4f, 0.4f, 1.0f), SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 0.0f), 1.0f));
	DXRPipeLine::GetInstance().AddMeshData(cubeMeshData, L"HitGroup", "MirrorCube", PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f));

	DXRPipeLine::GetInstance().AddMeshData(catMeshData, L"HitGroup", "Cat", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f), 0.5f, 0.01f, 2.6f));
	DXRPipeLine::GetInstance().AddMeshData(catMeshData, L"HitGroup", "MirrorCat", PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f));
	DXRPipeLine::GetInstance().AddMeshData(catMeshData, L"HitGroup", "WhiteCat", PhysicsBaseMaterial(SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f), 0.5f));

	DXRPipeLine::GetInstance().AddMeshData(sphereMeshData, L"HitGroup", "Sphere", PhysicsBaseMaterial(SimpleMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f), SimpleMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 0.2f),"Bear");
	_blenderMonkey = DXRPipeLine::GetInstance().AddMeshData(sphereMeshData, L"HitGroup", "Sphere2", _blenderMonkyMaterial,"Bear");

	// 必ず、メッシュデータを追加してからパイプラインの初期化を行う。
	DXRPipeLine::GetInstance().InitPipeLine();



	auto dirlight = LightManager::GetInstance().GetDirectionalLight();
	auto dir = SimpleMath::Vector3(-0.169f, -0.840, -0.515);
	dirlight->UpdateDirectionalLight(dir, SimpleMath::Color(1, 1, 1, 1));

	m_CameraAsistant = new CameraAsistant();


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




	//auto sphere = new Sphere(SimpleMath::Vector3(-2.0f, 0.0f, 0.6f), Sphere::SphereType_Normal);
	//sphere->SetScale(SimpleMath::Vector3(1.0f));
	//sphere->SetRotation(SimpleMath::Vector3(3.0f, 0.0f, 0.0f));
	//ActorManager::GetInstance().AddActor(sphere);


	auto rightWall = new Cube(SimpleMath::Vector3(-1000, 0, 0), SimpleMath::Vector3(1, 1000, 1000), 300.0f, "WhiteCube", false);
	rightWall->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
	rightWall->SetActorName("RightWall");
	ActorManager::GetInstance().AddActor(rightWall);
	rightWall->OnStatic(true, true);

	auto backWall = new Cube(SimpleMath::Vector3(0, 0, 1000), SimpleMath::Vector3(1000, 1000, 1), 300.0f, "WhiteCube", false);
	backWall->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
	backWall->SetActorName("BackWall");
	ActorManager::GetInstance().AddActor(backWall);
	backWall->OnStatic(true, true);

	auto frontWall = new Cube(SimpleMath::Vector3(0, 0, -1000), SimpleMath::Vector3(1000, 1000, 1), 300.0f, "WhiteCube", false);
	frontWall->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
	frontWall->SetActorName("FrontWall");
	ActorManager::GetInstance().AddActor(frontWall);
	frontWall->OnStatic(true, true);

	auto topWall = new Cube(SimpleMath::Vector3(0, 1000, 0), SimpleMath::Vector3(1000, 1, 1000), 300.0f, "WhiteCube", false);
	topWall->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
	topWall->SetActorName("TopWall");
	ActorManager::GetInstance().AddActor(topWall);
	topWall->OnStatic(true, true);

	auto mirror = new Cube(SimpleMath::Vector3(0, 0, 0), SimpleMath::Vector3(10, 10, 1), 300.0f, "MirrorCube", false);
	mirror->SetRotation(SimpleMath::Vector3(0, 0.0f, 0.0f));
	mirror->SetActorName("Mirror");
	ActorManager::GetInstance().AddActor(mirror);
	mirror->OnStatic(true, true);

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

	auto cat = new Cube(SimpleMath::Vector3(0, 3.0f, 0), SimpleMath::Vector3(10.0f), 300.0f, "Cat", false);
	cat->SetActorName("Cat");
	cat->SetRotation(SimpleMath::Vector3(0, 42.4f, 0));
	cat->OnStatic(true, false);
	ActorManager::GetInstance().AddActor(cat);

	auto mirrorCat = new Cube(SimpleMath::Vector3(10.0f, 3.0f, 0), SimpleMath::Vector3(10.0f), 300.0f, "MirrorCat", false);
	mirrorCat->SetActorName("MirrorCat");
	mirrorCat->SetRotation(SimpleMath::Vector3(0, 42.4f, 0));
	mirrorCat->OnStatic(true, false);
	ActorManager::GetInstance().AddActor(mirrorCat);

	auto whiteCat = new Cube(SimpleMath::Vector3(10.0f, 3.0f, 0), SimpleMath::Vector3(10.0f), 300.0f, "WhiteCat", false);
	whiteCat->SetActorName("WhiteCat");
	whiteCat->SetRotation(SimpleMath::Vector3(0, 42.4f, 0));
	whiteCat->OnStatic(true, false);
	ActorManager::GetInstance().AddActor(whiteCat);

	for (auto i = 0; i < 3; ++i)
	{
		auto pos_x = Random::GetRandom(-60.0f , 60.0f);
		auto pos_z = Random::GetRandom(20.0f, 30.0f);
		float pos_y = 3.0f;

		float rotateX = Random::GetRandom(-3.0f, 3.0f);
		
		auto cat = new Cube(SimpleMath::Vector3(pos_x, pos_y, pos_z), SimpleMath::Vector3(4.0f), 300.0f, "MirrorCat", false);
		cat->SetActorName("WhiteCat");
		cat->SetRotation(SimpleMath::Vector3(rotateX, 42.4f, 0));
		cat->OnStatic(true, false);
		ActorManager::GetInstance().AddActor(cat);
	}
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
