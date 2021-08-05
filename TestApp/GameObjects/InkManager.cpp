#include "InkManager.h"

#include "Device/ParticleSystems/ParticleManager.h"
#include "Device/ParticleSystems/ParticleEmitter.h"
#include "Device/ParticleSystems/FluidParticleAction.h"
#include "Device/ParticleSystems/FluidRenderingPipeLine.h"

#include "imgui/imgui.h"

InkManager::InkManager(int maxParticleCount)
	:mParticleMaxCount(maxParticleCount), m_isPushWallParameters(false), mCurrentParticleCount(0)
{

}

InkManager::~InkManager()
{

}

void InkManager::PushWallData(ParticleEmitter::WallPalam wallParam)
{
	m_WallParameters.push_back(wallParam);
	m_isPushWallParameters = true;
}

void InkManager::AddParticle(int inkParticleCount)
{
	mCurrentParticleCount = 
		std::clamp(mCurrentParticleCount + inkParticleCount, 0, mParticleMaxCount);
	
	mParticleEmitter->SetPosition(m_Position);
	mParticleEmitter->SetParticleCount(mCurrentParticleCount);
}

void InkManager::GenerateEmitter()
{
	mParticleEmitter = std::make_shared<ParticleEmitter>("Fluid_Wall_Effect", "Action1");
	mParticleEmitter->SetScale(SimpleMath::Vector3(50.0f));
	mParticleEmitter->SetColor(SimpleMath::Color(1, 1, 1, 1));
	mParticleEmitter->SetTimeScale(1.0f);
	mParticleEmitter->SetLifeTime(3.0f);
	mParticleEmitter->SetParticleCount(mParticleMaxCount);
	mParticleEmitter->SetPosition(m_Position);
	mParticleEmitter->SetParticleMode(ParticleEmitter::DrawParticleMode_FluidDepth);
	mParticleEmitter->SetWallParameters(m_WallParameters);
	ParticleManager::GetInstance().AddParticleEmiiter(mParticleEmitter);	

	mParticleEmitter->SetParticleCount(mCurrentParticleCount);
	m_action->SetWallCount(m_WallParameters.size());
}

void InkManager::UpdateActor()
{
	if (m_isPushWallParameters)
	{
		GenerateEmitter();
		m_isPushWallParameters = false;
	}

	auto data = FluidRendringPipeLine::GetInstance().GetResultBufferData();

	float result = 0.0f;

	if(data.wallOnInkCount > 0)
	{
		result = ((float)data.wallOnInkCount / (float)data.wallPixelCount) * 100.0f;
	}
	
	ImGui::Begin("ResultData", false, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("WallPixelCount:%i", data.wallPixelCount);
	ImGui::Text("WallOnInkPixelCount:%i", data.wallOnInkCount);
	ImGui::Text("Result:%f", result);
	ImGui::End();

}

void InkManager::Init()
{
	m_action = std::make_shared<FluidParticleAction>();
	m_action->SetPositionRange(SimpleMath::Vector3(6.0f, 1.0f, 6.0f));
	m_action->SetVelocityRange(SimpleMath::Vector3(0.01f, 0.00f, 0.01f));
	//action2->SetAddVelocity(SimpleMath::Vector3(0.0f, -0.0001f, 0));
	ParticleManager::GetInstance().AddAction(m_action, "Action1");
}

void InkManager::Shutdown()
{
	mParticleEmitter->Destroy();
}

void InkManager::OnCollsion(Actor* other)
{
}


