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

#include "GameObjects/Flor.h"

#include "Utility/Timer.h"


#include "GameObjects/Wall.h"

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

	ShaderManager::GetInstance().LoadShader(L"BasicResources/MeshParticleComputeShader.hlsl", "cs_5_0", "MeshParticleInitComputeShader", "init");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/MeshEmitterVertexShader.hlsl", "vs_5_0", "MeshEmitterGPUParticleVertexShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/MeshEmitterPixelShader.hlsl", "ps_5_0", "MeshEmitterGPUParticlePixelShader");

	//MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "sphere.obj", "Sphere");
	//MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "SpaceShip.obj", "SpaceShip");
	//MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "blenderMonkey.obj", "blenderMonkey");
	//MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "TLEX.obj", "TLEX");


	auto particleMeshEffect = std::shared_ptr<ParticleMeshEffect>(new ParticleMeshEffect());
	particleMeshEffect->Init("MeshEmitterGPUParticleVertexShader", "MeshEmitterGPUParticlePixelShader", "");
	EffectManager::GetInstance().AddEffect(particleMeshEffect, "ParticleMeshEffect");

	auto dirlight = LightManager::GetInstance().GetDirectionalLight();
	auto dir = SimpleMath::Vector3(-0.103f, -0.513f, -0.852f);
	dirlight->SetDirection(dir);

	m_CameraAsistant = new CameraAsistant();


	const SimpleMath::Vector3 base_position = Vector3(2.5f, 0.0f, 2.5f);

	RenderingPipeLine::GetInstance().SetSkyBox("Resources/SkyBox.dds", SimpleMath::Vector3(10.0f));
	RenderingPipeLine::GetInstance().SetDrawFluidFlag(false);



}


void MainGame::Update()
{
	m_CameraAsistant->Update();
}

void MainGame::Destroy()
{
}
