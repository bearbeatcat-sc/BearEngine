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
	:Game(L"FluidDemo", 1920, 1080)
	, mCurrentParticleCount(1)
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

	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "sphere.obj", "Sphere");
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "SpaceShip.obj", "SpaceShip");
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "blenderMonkey.obj", "blenderMonkey");
	MeshManager::GetInstance().loadMesh("Resources/Models/Model/", "TLEX.obj", "TLEX");


	auto particleMeshEffect = std::shared_ptr<ParticleMeshEffect>(new ParticleMeshEffect());
	particleMeshEffect->Init("MeshEmitterGPUParticleVertexShader", "MeshEmitterGPUParticlePixelShader", "");
	EffectManager::GetInstance().AddEffect(particleMeshEffect, "ParticleMeshEffect");

	auto dirlight = LightManager::GetInstance().GetDirectionalLight();
	auto dir = SimpleMath::Vector3(-0.103f, -0.513f, -0.852f);
	dirlight->SetDirection(dir);

	m_CameraAsistant = new CameraAsistant();

	mParticleTimer = std::make_shared<Timer>(mParticleTime);

	const SimpleMath::Vector3 base_position = Vector3(2.5f, 0.0f, 2.5f);

	RenderingPipeLine::GetInstance().SetSkyBox("Resources/SkyBox.dds", SimpleMath::Vector3(10.0f));
	RenderingPipeLine::GetInstance().SetDrawFluidFlag(true);

	//for(int i = 0; i < generateCount; ++i)
	//{
	//	float x = Random::GetRandom(-3.0f, 3.0f);
	//	float y = Random::GetRandom(-3.0f, 3.0f);
	//	float z = Random::GetRandom(-3.0f, 3.0f);

	//	auto m_GPUParticleEmitter = std::make_shared<ParticleEmitter>("ParticleCubeEffect", "Action1");
	//	m_GPUParticleEmitter->SetScale(SimpleMath::Vector3(40.0f));
	//	m_GPUParticleEmitter->SetColor(SimpleMath::Color(1, 1, 1, 1));
	//	m_GPUParticleEmitter->SetTimeScale(1.0f);
	//	m_GPUParticleEmitter->SetLifeTime(3.0f);
	//	m_GPUParticleEmitter->SetParticleCount(particle_count);
	//	m_GPUParticleEmitter->SetPosition(base_position+ Vector3(x,y,z));
	//	m_GPUParticleEmitter->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	//	GPUParticleManager::GetInstance().AddParticleEmiiter(m_GPUParticleEmitter);
	//}

	std::vector<bool> objectTagTable =
	{
		false,true
	};
	
	std::vector<bool> ballTagTable = 
	{
		true,false
	};
	
	CollisionTagManagaer::GetInstance().AddTag("Object", objectTagTable);
	CollisionTagManagaer::GetInstance().AddTag("Ball",ballTagTable);


	

	//m_GPUParticleEmitter = std::make_shared<ParticleEmitter>("ParticleCubeEffect", "Action1");
	//m_GPUParticleEmitter->SetScale(SimpleMath::Vector3(50.0f));
	//m_GPUParticleEmitter->SetColor(SimpleMath::Color(1, 1, 1, 1));
	//m_GPUParticleEmitter->SetTimeScale(1.0f);
	//m_GPUParticleEmitter->SetLifeTime(3.0f);
	//m_GPUParticleEmitter->SetParticleCount(total_particle_count);
	//m_GPUParticleEmitter->SetPosition(base_position);
	//m_GPUParticleEmitter->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	//ParticleManager::GetInstance().AddParticleEmiiter(m_GPUParticleEmitter);


	//auto m_GPUParticleEmitter2 = std::make_shared<ParticleEmitter>("ParticleCubeEffect", "Action1");
	//m_GPUParticleEmitter2->SetScale(SimpleMath::Vector3(10.0f));
	//m_GPUParticleEmitter2->SetColor(SimpleMath::Color(1, 1, 1, 1));
	//m_GPUParticleEmitter2->SetTimeScale(1.0f);
	//m_GPUParticleEmitter2->SetLifeTime(3.0f);
	//m_GPUParticleEmitter2->SetParticleCount(200000);
	//m_GPUParticleEmitter2->SetPosition(SimpleMath::Vector3(0, 1, -1));
	//m_GPUParticleEmitter2->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	//GPUParticleManager::GetInstance().AddParticleEmiiter(m_GPUParticleEmitter);



	//auto m_GPUParticleEmitter2 = std::make_shared<ParticleEmitter>("ParticleCubeEffect", "Action1");
	//m_GPUParticleEmitter2->SetScale(SimpleMath::Vector3(100.0f));
	//m_GPUParticleEmitter2->SetColor(SimpleMath::Color(1, 1, 1, 1));
	//m_GPUParticleEmitter2->SetTimeScale(1.0f);
	//m_GPUParticleEmitter2->SetLifeTime(3.0f);
	//m_GPUParticleEmitter2->SetParticleCount(3000);
	//m_GPUParticleEmitter2->SetPosition(SimpleMath::Vector3(0, -2, 0));
	//m_GPUParticleEmitter2->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	//GPUParticleManager::GetInstance().AddParticleEmiiter(m_GPUParticleEmitter2);
	//
	//auto m_GPUParticleEmitter3 = std::make_shared<ParticleEmitter>("ParticleCubeEffect", "Action1");
	//m_GPUParticleEmitter3->SetScale(SimpleMath::Vector3(100.0f));
	//m_GPUParticleEmitter3->SetColor(SimpleMath::Color(1, 1, 1, 1));
	//m_GPUParticleEmitter3->SetTimeScale(1.0f);
	//m_GPUParticleEmitter3->SetLifeTime(3.0f);
	//m_GPUParticleEmitter3->SetParticleCount(3000);
	//m_GPUParticleEmitter3->SetPosition(SimpleMath::Vector3(0.3f, -2, 0));
	//m_GPUParticleEmitter3->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	//GPUParticleManager::GetInstance().AddParticleEmiiter(m_GPUParticleEmitter3);


	auto spehere2 = new Sphere(base_position + Vector3(0.0f, 0.0f, 0.0f), Sphere::SphereType::SphereType_NormalMesh);
	spehere2->SetColor(SimpleMath::Color(0.6f, 0.2f, 0.2f));
	spehere2->SetScale(SimpleMath::Vector3(0.01f));
	spehere2->SetRotation(Quaternion::CreateFromYawPitchRoll(30.0f, 0, 0));
	ActorManager::GetInstance().AddActor(spehere2);

	mSpeherePosition = base_position;
	mSpehere = new Sphere(mSpeherePosition, Sphere::SphereType::SphereType_NormalMesh);
	mSpehere->SetColor(SimpleMath::Color(1, 0, 0, 1));
	mSpehere->SetScale(SimpleMath::Vector3(0.1f));
	mSpehere->SetRotation(Quaternion::CreateFromYawPitchRoll(30.0f, 0, 0));
	ActorManager::GetInstance().AddActor(mSpehere);

	auto flor = new Flor(Vector3::Zero, Vector3::Zero);
	ActorManager::GetInstance().AddActor(flor);

	auto wall = new Wall(Vector3(1.0f, 0.0f, 1.0f), Vector3::Zero);
	ActorManager::GetInstance().AddActor(wall);


	//auto wallColor = SimpleMath::Color(0.8f, 0.8f, 0.8f);

	//auto wall0 = new ::Plane(basePos + Vector3(5.5f, 3, 11), Vector3(12, 8,1));
	//ActorManager::GetInstance().AddActor(wall0);
	//wall0->SetColor(wallColor);

	//auto wall1 = new ::Plane(basePos + Vector3(11, 3, 5.5f), Vector3(12, 8, 1));
	//ActorManager::GetInstance().AddActor(wall1);
	//wall1->SetColor(wallColor);
	//wall1->SetRotation(Quaternion::CreateFromYawPitchRoll(-30.0f, 0, 0));


}


void MainGame::Update()
{
	m_CameraAsistant->Update();


	//mParticleTimer->Update();
	//if (mParticleTimer->IsTime())
	//{
	//	mParticleTimer->Reset();
	//	mCurrentParticleCount = std::clamp(mCurrentParticleCount + 256, 1, total_particle_count);
	//	m_GPUParticleEmitter->SetParticleCount(mCurrentParticleCount);
	//}


	//float generate_position[3] = { m_generate_position.x,m_generate_position.y,m_generate_position.z };
	//float add_velocity[3] = { m_add_velocity.x,m_add_velocity.y,m_add_velocity.z };
	//float velocity[3] = { m_velocity.x,m_velocity.y,m_velocity.z };
	//float generate_range[3] = { m_generate_range.x,m_generate_range.y,m_generate_range.z };

	//// 後でクラスに独立する。
	//ImGui::Begin("Particles", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	//ImGui::InputFloat("Generate_Particle_Size", &m_particle_size, 1);
	//ImGui::InputInt("Generate_Count", &m_partcile_count, 1);
	//ImGui::DragFloat3("Generate_Position", generate_position, 0.01f, 0.0f, 10.0f);
	//ImGui::DragFloat3("Generate_Range", generate_range, 0.001f, 0.001f, 10.0f);
	//ImGui::DragFloat3("AddVelocity", add_velocity, 0.001f, 0.001f, 10.0f);
	//ImGui::DragFloat3("Velocity", velocity, 0.001f, 0.001f, 10.0f);
	//if (ImGui::Button("Generate_Particle", ImVec2(300, 30)))
	//{
	//	m_action->SetAddVelocity(m_add_velocity);
	//	m_action->SetPositionRange(m_generate_range);
	//	auto m_GPUParticleEmitter = std::make_shared<ParticleEmitter>("ParticleCubeEffect", "Action0");
	//	m_GPUParticleEmitter->SetScale(SimpleMath::Vector3(m_particle_size));
	//	m_GPUParticleEmitter->SetColor(SimpleMath::Color(1, 1, 1, 1));
	//	//m_GPUParticleEmitter->SetVelocity(SimpleMath::Vector3(x * 0.01f, y * 0.01f, z * 0.01f));
	//	m_GPUParticleEmitter->SetTimeScale(1.0f);
	//	m_GPUParticleEmitter->SetLifeTime(3.0f);
	//	m_GPUParticleEmitter->SetParticleCount(m_partcile_count);
	//	m_GPUParticleEmitter->SetPosition(m_generate_position);
	//	m_GPUParticleEmitter->SetVelocity(m_velocity);
	//	m_GPUParticleEmitter->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	//	GPUParticleManager::GetInstance().AddParticleEmiiter(m_GPUParticleEmitter);
	//	m_particles.push_back(m_GPUParticleEmitter);
	//}

	//ImGui::BeginChild("Particles", ImVec2(300, 90), ImGuiWindowFlags_NoTitleBar);
	//for (int i = 0; i < m_particles.size(); ++i)
	//{
	//	ImGui::Text("Particle%i", i);
	//	if (ImGui::Button("Delete", ImVec2(90, 30)))
	//	{
	//		auto itr = std::next(m_particles.begin(), i);
	//		(*itr)->Destroy();
	//		m_particles.erase(itr);
	//	}
	//}
	//ImGui::EndChild();

	//ImGui::End();

	//m_generate_position = SimpleMath::Vector3(generate_position);
	//m_velocity = SimpleMath::Vector3(velocity);
	//m_add_velocity = SimpleMath::Vector3(add_velocity);
	//m_generate_range = SimpleMath::Vector3(generate_range);
}

void MainGame::Destroy()
{
}
