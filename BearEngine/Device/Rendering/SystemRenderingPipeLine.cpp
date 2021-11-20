#include "SystemRenderingPipeLine.h"

#include "RenderingPipeline.h"
#include "Device/SpriteDrawer.h"
#include "Device/DirectX/Core/ShaderManager.h"
#include "Device/DirectX/Core/Model/DebugDrawer.h"
#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Device/Lights/LightManager.h"
#include "Device/Raytracing/DXRPipeLine.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "Device/ParticleSystems/ParticleManager.h"

void SystemRenderingPipeLine::InitPipeLine()
{

	LoadAssets();

	DirectXDevice::GetInstance().EnableDebugLayer();
	DirectXDevice::GetInstance().InitDirectX();
	DirectXGraphics::GetInstance().Init();
	LightManager::GetInstance().Init();

#ifdef _DEBUG
	DebugDrawer::GetInstance().Init(L"BasicResources/SimpleVertexShader.hlsl", L"BasicResources/SimplePixelShader.hlsl");
#endif
	SpriteDrawer::GetInstance().Init();
	DXRPipeLine::GetInstance().Init();
	MeshManager::GetInstance().Init();
	MeshDrawer::GetInstance().Init();
	
	RenderingPipeLine::GetInstance().Init();
}

void SystemRenderingPipeLine::BeginRender()
{
	DirectXGraphics::GetInstance().Begin();
}

void SystemRenderingPipeLine::EndRender()
{
	DirectXGraphics::GetInstance().FontSystemBegin();
	DirectXGraphics::GetInstance().FontSystemEnd();
	DirectXGraphics::GetInstance().End();
	DirectXGraphics::GetInstance().Present();
	ParticleManager::GetInstance().DeleteParticles();

}

void SystemRenderingPipeLine::LoadAssets()
{
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicSpritePixelShader.hlsl", "ps_5_0", "BasicSpritePixelShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicSpriteVertexShader.hlsl", "vs_5_0", "BasicSpriteVertexShader");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicPixelShader.hlsl", "ps_5_0", "BasicPixelShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicVertexShader.hlsl", "vs_5_0", "BasicVertexShader");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/ParticleComputeShader.hlsl", "cs_5_0", "ParticleUpdateComputeShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/ParticleComputeShader.hlsl", "cs_5_0", "ParticleInitComputeShader", "init");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticleForceComputeShader", "CS_Force");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticleCollisionComputeShader", "CS_Collision");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticlePressureComputeShader", "CS_Pressure");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticlePressureGradientComputeShader", "CS_PressureGradient");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticleMoveParticleComputeShader", "CS_MoveParticle");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticleIntegrateComputeShader", "CS_Integrate");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidParticleComputeShader.hlsl", "cs_5_0", "FluidParticleInitComputeShader", "init");


	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicVertexShader.hlsl", "vs_5_0", "GPUParticleVertexShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/Fluid_Wall_VertexShader.hlsl", "vs_5_0", "Fluid_Wall_VertexShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicPixelShader.hlsl", "ps_5_0", "GPUParticlePixelShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicGeometryShader.hlsl", "gs_5_0", "GPUParticleGeometryShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicGeometryShader.hlsl", "gs_5_0", "GPUParticleCubeGeometryShader", "Cubemain");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/ModelVertexShader.hlsl", "vs_5_0", "ModelVertexShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/ModelVertexShader.hlsl", "vs_5_0", "ShadowVertexShader", "shadowVS");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/ModelPixelShader.hlsl", "ps_5_0", "ModelPixelShader");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicSpriteVertexShader.hlsl", "vs_5_0", "SpriteVertexShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/BasicSpritePixelShader.hlsl", "ps_5_0", "SpritePixelShader");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/PeraPolygonDrawPixelShader.hlsl", "ps_5_0", "PE_PeraPolygonPS");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/PeraPolygonDrawVertexShader.hlsl", "vs_5_0", "PE_PeraPolygonVS");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/PeraPolygonDrawPixelShader.hlsl", "ps_5_0", "PE_RaytracingBlendPolygonPS","RaytracingBlendPS");
	
	// BLOOM
	ShaderManager::GetInstance().LoadShader(L"BasicResources/PostEffectBloomPixelShader.hlsl", "ps_5_0", "PEBloomPS");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/PostEffectBloomVertexShader.hlsl", "vs_5_0", "PEBloomVS");

	

	ShaderManager::GetInstance().LoadShader(L"BasicResources/PostEffectBloomPixelShader.hlsl", "ps_5_0", "PEBloomBlur0", "VerticalBokehPS");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/PostEffectBloomPixelShader.hlsl", "ps_5_0", "PEBloomBlur1", "BlurPS");

	// DepthOfFeild
	ShaderManager::GetInstance().LoadShader(L"BasicResources/PostEffectDOFPixelShader.hlsl", "ps_5_0", "PE_DOF_HorizontalBlur", "Horizontal");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/PostEffectDOFPixelShader.hlsl", "ps_5_0", "PE_DOF_Result", "DepthOfField");

	// Fluid
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidDepthVertexShader.hlsl", "vs_5_0", "FluidDepthVertexShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidDepthPixelShader.hlsl", "ps_5_0", "FluidDepthPixelShader");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidDepthGeometryShader.hlsl", "gs_5_0", "FluidDepthGeometryShader");

	// CubeMap
	ShaderManager::GetInstance().LoadShader(L"BasicResources/CubeMap_ModelVertexShader.hlsl", "vs_5_0", "CubeMap_ModelVertexShader");

}
