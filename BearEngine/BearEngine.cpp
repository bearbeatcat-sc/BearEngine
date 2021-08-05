#include "BearEngine.h"

#include "Device/DirectX/DirectXGraphics.h"
#include "Device/DirectX/DirectXInput.h"
#include "Device/DirectX/Core/Sounds/SoundManager.h"

#include "Utility/CameraManager.h"
#include "Device/DirectX/Core/Model/DebugDrawer.h"
#include "Device/SpriteDrawer.h"
#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Components/Collsions/CollisionManager.h"
#include "Game_Object/ActorManager.h"
#include "Device/ParticleSystems/ParticleManager.h"
#include "Device/RenderingPipeline.h"


BearEngine::BearEngine()
{
}

BearEngine::~BearEngine()
{
}

void BearEngine::InitEngine()
{
	CollisionManager::GetInstance().Init(5, SimpleMath::Vector3(0, 0, 0), SimpleMath::Vector3(9000000, 9000000, 9000000));
	CameraManager::GetInstance().Init();
	RenderingPipeLine::GetInstance().Init();
	SoundManager::GetInstance().Init();
	ActorManager::GetInstance().Init();
	DirectXInput::GetInstance().InitDirectInput();
}

void BearEngine::EngineUpdate()
{
	DirectXInput::GetInstance().UpdateInput();
	ActorManager::GetInstance().Update();
	CollisionManager::GetInstance().Update();
	SpriteDrawer::GetInstance().Update();
	ParticleManager::GetInstance().Update();
	MeshDrawer::GetInstance().Update();
}

// ï`âÊèÄîı
void BearEngine::EngineDrawBegin()
{
	RenderingPipeLine::GetInstance().DrawBegin();
}

void BearEngine::EngineDraw()
{
	RenderingPipeLine::GetInstance().Draw();
}

void BearEngine::EngineDrawEnd()
{
	RenderingPipeLine::GetInstance().DrawEnd();
}
