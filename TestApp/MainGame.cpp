#include "MainGame.h"
#include "Utility/Camera.h"
#include "Device/DirectX/DirectXInput.h"
#include "Device/DirectX/DirectXGraphics.h"

#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Device/DirectX/Core/ShaderManager.h"
#include "Game_Object/ActorManager.h"
#include "Components/Collsions/CollisionTagManager.h"

#include <time.h>

#include "Device/DirectX/Core/EffectManager.h"

#include "CameraAsistant.h"
#include "GameObjects/Sphere.h"

#include <Device/Lights/DirectionalLight.h>
#include <Device/Lights/LightManager.h>

#include <Device/DirectX/Core/ParticleMeshEffect.h>
#include <Device/DirectX/Core/Sounds/SoundManager.h>

#include <Device/RenderingPipeline.h>
#include <imgui/imgui.h>

#include "Device/Raytracing/DXRPipeLine.h"
#include "GameObjects/Cube.h"
#include "GameObjects/Flor.h"

#include "Utility/Timer.h"


#include "GameObjects/Wall.h"
#include "Utility/Random.h"

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




	auto dirlight = LightManager::GetInstance().GetDirectionalLight();
	auto dir = SimpleMath::Vector3(-0.103f, -0.513f, -0.852f);
	dirlight->SetDirection(dir);

	m_CameraAsistant = new CameraAsistant();


	const SimpleMath::Vector3 base_position = Vector3(2.5f, 0.0f, 2.5f);

	RenderingPipeLine::GetInstance().SetSkyBox("Resources/SkyBox.dds", SimpleMath::Vector3(10.0f));
	RenderingPipeLine::GetInstance().SetDrawFluidFlag(false);


	//auto meshData = MeshManager::GetInstance().FindSpehere(6);

	_AddTimer = std::make_shared<Timer>(3.0f);
	_GenerateTimer = std::make_shared<Timer>(0.2f);
	_IsGenerate = false;



	auto sphere = new Sphere(SimpleMath::Vector3(0, -20, 30), Sphere::SphereType_Normal);
	sphere->SetScale(SimpleMath::Vector3(30, 1, 30));
	ActorManager::GetInstance().AddActor(sphere);

	auto cube = new Cube(SimpleMath::Vector3(0, 10, 30),SimpleMath::Vector3(3.0f),60.0f,false);
	ActorManager::GetInstance().AddActor(cube);
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
	
	_GenerateTimer->Update();
	if(_GenerateTimer->IsTime())
	{
		_GenerateTimer->Reset();
		auto pos_x = Random::GetRandom(-10.0f, 10.0f);
		auto pos_z = Random::GetRandom(10.0f, 30.0f);
		float pos_y = 2.0f;

		float rotateX = Random::GetRandom(-1.0f, 1.0f);
		float rotateY = Random::GetRandom(-1.0f, 1.0f);
		float rotateZ = Random::GetRandom(-1.0f, 1.0f);

		float scale = Random::GetRandom(1.0f, 3.0f);

		auto cube = new Cube(SimpleMath::Vector3(pos_x, pos_y, pos_z), SimpleMath::Vector3(scale), 4.0f);
		cube->SetRotation(Quaternion::CreateFromYawPitchRoll(rotateX, rotateY, rotateZ));
		ActorManager::GetInstance().AddActor(cube);

		_GenerateCount++;

		if(_GenerateCount >= _MaxGenerateCount)
		{
			_IsGenerate = false;
		}
	}



}

void MainGame::Destroy()
{
}
