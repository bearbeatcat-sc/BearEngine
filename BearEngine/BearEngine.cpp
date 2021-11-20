#include "BearEngine.h"

#include "imgui/imgui.h"
#include "Device/DirectX/DirectXInput.h"
#include "Device/DirectX/Core/Sounds/SoundManager.h"

#include "Utility/CameraManager.h"
#include "Device/SpriteDrawer.h"
#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Components/Collsions/CollisionManager.h"
#include "Game_Object/ActorManager.h"
#include "Device/ParticleSystems/ParticleManager.h"
#include "Device/Rendering/RenderingPipeline.h"
#include "Device/Rendering/SystemRenderingPipeLine.h"


BearEngine::BearEngine()
{
}

BearEngine::~BearEngine()
{
}

void BearEngine::InitEngine()
{
	CollisionManager::GetInstance().Init(5, SimpleMath::Vector3(0, 0, 0), SimpleMath::Vector3(9000000, 9000000, 9000000));
	SystemRenderingPipeLine::GetInstance().InitPipeLine();
	CameraManager::GetInstance().Init();
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

// 描画準備
void BearEngine::BeginRender()
{
	// アプリケーション描画とポストエフェクトの処理を行う
	RenderingPipeLine::GetInstance().ProcessingPostEffect();

	RenderApplication();
}

void BearEngine::RenderApplication()
{
	// スプライトの描画を行う
	RenderingPipeLine::GetInstance().BeginRenderResult();
	SpriteDrawer::GetInstance().Draw();
	RenderingPipeLine::GetInstance().EndRenderResult();

	// エンジン自体の描画の準備
	SystemRenderingPipeLine::GetInstance().BeginRender();
	// デバッグ以外では直接レンダリングをおこなう
	RenderingPipeLine::GetInstance().DrawPostEffectPolygon();
}

void BearEngine::EndRender()
{
	SystemRenderingPipeLine::GetInstance().EndRender();
}

