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
}


void MainGame::Update()
{
	m_CameraAsistant->Update();

	_AddTimer->Update();

	if(_AddTimer->IsTime())
	{
		for (int i = 0; i < 6; ++i)
		{

			auto pos_x = Random::GetRandom(-10.0f, 10.0f);
			auto pos_z = Random::GetRandom(-10.0f, 10.0f);
			float pos_y = 2.0f;

			auto cube = new Cube(SimpleMath::Vector3(pos_x, pos_y, pos_z), SimpleMath::Vector3(1.0f), 4.0f);
			ActorManager::GetInstance().AddActor(cube);

			_AddTimer->Reset();
		}
	}
}

void MainGame::Destroy()
{
}
