#include "RenderingPipeline.h"

#include "../DirectX/DirectXDevice.h"
#include "../DirectX/DirectXGraphics.h"
#include "../../Device/WindowApp.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../../imgui/imgui_impl_dx12.h"
#include "../DirectX/Core/ShaderManager.h"
#include "../../Utility/Math/MathUtility.h"
#include "../../Device/DirectX/Core/Model/MeshDrawer.h"
#include "../../Device/ParticleSystems/ParticleManager.h"
#include "../../Device/DirectX/Core/Model/DebugDrawer.h"
#include "../../Device/SpriteDrawer.h"
#include "../../Components/Collsions/CollisionManager.h"
#include "../../Utility/LogSystem.h"
#include "../../Device/Lights/LightManager.h"
#include "../../Utility/CameraManager.h"
#include "../../Utility/Camera.h"
#include "../SkyBox.h"
#include <array>

#include "../Raytracing/DXRPipeLine.h"
#include "../DirectX/Core/Buffer.h"
#include "../DirectX/Core/EffectManager.h"
#include "../DirectX/Core/ParticleSpriteEffect.h"
#include "../DirectX/Core/SpriteEffect.h"
#include "../DirectX/Core/Model/MeshManager.h"
#include "../Raytracing/DXRInstance.cpp"
#include "Utility/Random.h"

// シャドウマップ（深度バッファ）の解像度
constexpr uint32_t shadow_difinition = 1024;

RenderingPipeLine::RenderingPipeLine()
	:m_isDrawFluid(false)
{

}

RenderingPipeLine::~RenderingPipeLine()
{
	if (m_pSkyBox)
	{
		delete m_pSkyBox;
	}
}

void RenderingPipeLine::CreateEffects()
{
	auto effect = std::shared_ptr<MeshEffect>(new MeshEffect());
	effect->Init("ModelVertexShader", "ModelPixelShader", "");
	EffectManager::GetInstance().AddEffect(effect, "NormalMeshEffect");

	auto particleSpriteEffect = std::shared_ptr<ParticleSpriteEffect>(new ParticleSpriteEffect());
	particleSpriteEffect->Init("GPUParticleVertexShader", "GPUParticlePixelShader", "GPUParticleGeometryShader");
	EffectManager::GetInstance().AddEffect(particleSpriteEffect, "ParticleSpriteEffect");

	auto particleCubeEffect = std::shared_ptr<ParticleSpriteEffect>(new ParticleSpriteEffect());
	particleCubeEffect->Init("GPUParticleVertexShader", "GPUParticlePixelShader", "GPUParticleCubeGeometryShader");
	EffectManager::GetInstance().AddEffect(particleCubeEffect, "ParticleCubeEffect");

	auto spriteEffect = std::shared_ptr<SpriteEffect>(new SpriteEffect());
	spriteEffect->Init("SpriteVertexShader", "SpritePixelShader", "");
	EffectManager::GetInstance().AddEffect(spriteEffect, "NormalSpriteEffect");

	auto cubemapMeshEffect = std::shared_ptr<MeshEffect>(new MeshEffect());
	cubemapMeshEffect->Init("CubeMap_ModelVertexShader", "ModelPixelShader", "");
	EffectManager::GetInstance().AddEffect(cubemapMeshEffect, "CubeMapMeshEffect");
}

HRESULT RenderingPipeLine::Init()
{
	m_pCommandList = DirectXGraphics::GetInstance().GetCommandList();

	//各種リソース生成

	if (CreateRenderResource() != S_OK)
	{
		return S_FALSE;
	}

	if (CreateBloomResource() != S_OK)
	{
		return S_FALSE;
	}

	if (CreateBlurWeightResource() != S_OK)
	{
		return S_FALSE;
	}


	if (CreatePeraPolygon() != S_OK)
	{
		return S_FALSE;
	}

	if (CreateRTV() != S_OK)
	{
		return S_FALSE;
	}

	if (CreateSRV() != S_OK)
	{
		return S_FALSE;
	}

	if (CreatePipeLines() != S_OK)
	{
		return S_FALSE;
	}

	if (InitCubeMapResource() != S_OK)
	{
		return S_FALSE;
	}


	CreateEffects();
}

void RenderingPipeLine::SetSkyBox(const std::string& texturePath, const SimpleMath::Vector3& scale)
{
	m_pSkyBox = new SkyBox(texturePath, CameraManager::GetInstance().GetMainCamera());
	m_pSkyBox->Init();
	m_pSkyBox->SetScale(scale);
	m_pSkyBox->SetColor(SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

void RenderingPipeLine::ProcessingPostEffect()
{
	// レイトレーシングでの描画結果を取得
	DXRPipeLine::GetInstance().Render(m_RaytracingResource.Get(), m_pSkyBox);

	// 通常のレンダリング
	DefaultRendering();

	DrawPostEffect();
}

void RenderingPipeLine::Draw()
{
	// ポストエフェクト後の板ポリ描画
	//DrawPostEffectPolygon();




	// デバッグ表示用
//#ifdef _DEBUG
//	ImGui::Begin("Rendering_System", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysVerticalScrollbar);
//
//	CollisionManager::GetInstance().Draw();
//
//	ImGui::Dummy(ImVec2(0, 30));
//
//	ImGui::BeginTabBar("DebugTabs");
//	UpdateConstantBuffers();
//
//	LogSystem::DrawLog();
//	MeshDrawer::GetInstance().DrawDebug();
//	LightManager::GetInstance().Draw();
//	DXRPipeLine::GetInstance().DrawDebugGUI();
//
//	ImGui::EndTabBar();
//
//
//	ImGui::End();
//	DebugDrawer::GetInstance().Draw();
//
//
//#endif
}


void RenderingPipeLine::DrawEnd()
{

}

bool RenderingPipeLine::DefaultRenderingBegin()
{

	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_OutputRenderResource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	
	DirectXGraphics::GetInstance().SetRenderTarget(m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());


	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();

	return true;
}

bool RenderingPipeLine::DefaultRenderingEnd()
{
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_OutputRenderResource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	return true;
}

// 最終的なリザルト結果を描画する
void RenderingPipeLine::DrawPostEffectPolygon()
{

	m_pCommandList->SetGraphicsRootSignature(posteffect_result_pso.rootSignature.Get());
	m_pCommandList->SetDescriptorHeaps(1, m_peraSRVHeap.GetAddressOf());

	// 最終的なポストエフェクトの結果
	auto srvHandle = m_peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (OutputRenderResouce + m_BlurWeights + m_BloomBufferCount);
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	m_pCommandList->SetPipelineState(posteffect_result_pso.pso.Get());
	m_pCommandList->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);
	m_pCommandList->DrawInstanced(4, 1, 0, 0);


}

void RenderingPipeLine::RenderGUIImage()
{
	auto windowSize = WindowApp::GetInstance().GetDebugGameWindowSize();
	
	ImGui::Begin("Game");
	ImGui::Image((ImTextureID)_processed_resource_guiImage_Handle.ptr, ImVec2(windowSize.x, windowSize.y));
	ImGui::End();
}

void RenderingPipeLine::EffectBloom()
{
	RenderingHighLight();


	// ブラーをかける
	m_pCommandList->SetPipelineState(posteffect_bloomblur_horizontal_pso.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(posteffect_bloom_hightLight_pso.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);



	// 高輝度成分のバッファをシェーダーリソースに
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_BloomBuffer[0].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// 縮小バッファをレンダーターゲットに
	for (int i = 1; i < m_BloomBufferCount; ++i)
	{
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_BloomBuffer[i].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));
	}

	auto bloomBufferPointer = m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	// 高輝度縮小バッファ1に描画先を指定
	bloomBufferPointer.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * 2;

	// デスクリプタヒープのセット
	m_pCommandList->SetDescriptorHeaps(1, m_peraSRVHeap.GetAddressOf());


	auto srvHandle = m_peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	// (高輝度）をテクスチャとしてセット 
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_pCommandList->SetGraphicsRootDescriptorTable(Bloom_RootParamter_HighLight, srvHandle);

	// ハンドルを定数バッファまで移動
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (m_BloomBufferCount);
	m_pCommandList->SetGraphicsRootDescriptorTable(Bloom_RootParamter_BlurParameter, srvHandle);

	auto desc = m_BloomBuffer[0]->GetDesc();
	D3D12_VIEWPORT vp = {};
	D3D12_RECT sr = {};

	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;
	vp.Height = desc.Height / 2;
	vp.Width = desc.Width / 2;
	sr.top = 0;
	sr.left = 0;
	sr.right = vp.Width;
	sr.bottom = vp.Height;

	auto w = desc.Width / 2;
	auto h = desc.Height / 2;

	for (int i = 1; i < m_BloomBufferCount; ++i)
	{
		//auto desc = m_BloomBuffer[i]->GetDesc();
		vp.Height = float(h);
		vp.Width = float(w);
		//sr.right = vp.Width;
		//sr.bottom = vp.Height;

		// レンダリングターゲットのセット
		m_pCommandList->OMSetRenderTargets(1, &bloomBufferPointer, false, nullptr);
		m_pCommandList->ClearRenderTargetView(bloomBufferPointer, clerColorArray.data(), 0, nullptr);


		m_pCommandList->RSSetViewports(1, &vp);
		m_pCommandList->RSSetScissorRects(1, &sr);
		m_pCommandList->DrawInstanced(4, 1, 0, 0);

		w >>= 1;
		h >>= 1;


		// 次の縮小バッファ
		bloomBufferPointer.ptr += DirectXDevice::GetInstance().GetDevice()
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	//　縮小バッファをシェーダーリソースに
	for (int i = 1; i < m_BloomBufferCount; ++i)
	{
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_BloomBuffer[i].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		));
	}

	// 最終的なBloomのリザルトを描画
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_processed_resource.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->OMSetRenderTargets(1, &bloomBufferPointer, false, nullptr);
	m_pCommandList->ClearRenderTargetView(bloomBufferPointer, clerColorArray.data(), 0, nullptr);

	DirectXGraphics::GetInstance().SetViewPort();
	DirectXGraphics::GetInstance().SetScissorRect();

	m_pCommandList->SetGraphicsRootSignature(posteffect_bloom_hightLight_pso.rootSignature.Get());

	srvHandle = m_peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (m_BloomBufferCount + m_BlurWeights + ResultBloomResource + RaytracingRenderResource);

	m_pCommandList->SetGraphicsRootDescriptorTable(Bloom_RootParamter_NormalColor, srvHandle);

	m_pCommandList->SetDescriptorHeaps(1, m_peraSRVHeap.GetAddressOf());
	srvHandle = m_peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	// 縮小バッファ
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 2;
	m_pCommandList->SetGraphicsRootDescriptorTable(Bloom_RootParamter_HighLightReduction, srvHandle);

	m_pCommandList->SetPipelineState(posteffect_bloomblur_vertical_pso.pso.Get());
	m_pCommandList->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);
	m_pCommandList->DrawInstanced(4, 1, 0, 0);

	//m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_processed_resource.Get(),
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void RenderingPipeLine::SetDrawFluidFlag(bool flag)
{
	m_isDrawFluid = flag;
}

// ポストエフェクト後にレンダリングする際に使用する
void RenderingPipeLine::BeginRenderResult()
{
	//m_pCommandList->ResourceBarrier(
	//	1, &CD3DX12_RESOURCE_BARRIER::Transition(
	//		_processed_resource.Get(),
	//		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//		D3D12_RESOURCE_STATE_RENDER_TARGET));

	//　加工されていない状態を描画する
	//
	DirectXGraphics::GetInstance().SetRenderTarget(_processed_resource_RTV_CPUHANDLE,false);


	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();
}

void RenderingPipeLine::EndRenderResult()
{
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_processed_resource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

// 加工前の3Dレンダリング結果を渡すよう
ID3D12Resource* RenderingPipeLine::GetOutputRenderResource()
{
	return m_OutputRenderResource.Get();
}

// 他のシェーダーで加工するのを考慮
ID3D12DescriptorHeap* RenderingPipeLine::GetPeraSRVHeap()
{
	return m_peraSRVHeap.Get();
}

void RenderingPipeLine::BeginDrawShadow()
{
	// ライトデプスにセット

	auto handle = DirectXGraphics::GetInstance().GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// レンダーターゲットのセットなし(深度値のみ）
	m_pCommandList->OMSetRenderTargets(0, nullptr, false, &handle);


	DirectXGraphics::GetInstance().ClearDepthStencilView(handle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, shadow_difinition, shadow_difinition);
	DirectXGraphics::GetInstance().SetViewPort(vp);

	CD3DX12_RECT rc(0, 0, 1920, 1080);
	DirectXGraphics::GetInstance().SetScissorRect(rc);

}


HRESULT RenderingPipeLine::CreateRTV()
{
	auto heapDesc = DirectXGraphics::GetInstance().GetRTVHeap()->GetDesc();

	// RTV用ヒープの作成
	heapDesc.NumDescriptors = OutputRenderResouce + m_BloomBufferCount + ResultBloomResource; // Bloom 
	auto result = DirectXDevice::GetInstance().GetDevice()
		->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_peraRTVHeap.ReleaseAndGetAddressOf()));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	auto handle = m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart();


	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_OutputRenderResource.Get(),
		&rtvDesc,
		handle
	);

	// 高輝度のRTV
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_BloomBuffer[0].Get(),
		&rtvDesc,
		handle
	);

	for (int i = 1; i < m_BloomBufferCount; ++i)
	{
		// 縮小高輝度のRTV
		handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
		(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
			m_BloomBuffer[i].Get(),
			&rtvDesc,
			handle
		);

	}


	// Bloomの最終的な結果
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	_processed_resource_RTV_CPUHANDLE = handle;

	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		_processed_resource.Get(),
		&rtvDesc,
		handle
	);

	return result;
}

HRESULT RenderingPipeLine::CreateSRV()
{

	auto heapDesc = DirectXGraphics::GetInstance().GetRTVHeap()->GetDesc();

	// SRV用ヒープの作成
	heapDesc.NumDescriptors = OutputRenderResouce + m_BloomBufferCount + m_BlurWeights + ResultBloomResource + RaytracingRenderResource;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto result = DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(m_peraSRVHeap.ReleaseAndGetAddressOf()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	auto handle = m_peraSRVHeap->GetCPUDescriptorHandleForHeapStart();

	// 最初の描画先
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_OutputRenderResource.Get(),
		&srvDesc,
		handle);


	// 高輝度テクスチャ
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_BloomBuffer[0].Get(),
		&srvDesc,
		handle);

	for (int i = 1; i < m_BloomBufferCount; ++i)
	{
		// 高輝度縮小テクスチャ
		handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
		(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
			m_BloomBuffer[i].Get(),
			&srvDesc,
			handle);
	}

	// ブラー用ウェイト
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_BlurPramResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_BlurPramResource->GetDesc().Width;

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		handle);

	BlurParamResource_SRVHandle = handle;

	// 最終的なBloomの結果
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		_processed_resource.Get(),
		&srvDesc,
		handle);


	// レイトレーシングでの描画結果
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_RaytracingResource.Get(),
		&srvDesc,
		handle);


	_processed_resource_guiImage_Handle = DirectXGraphics::GetInstance().AllocateImGuiResource(_processed_resource);

	return result;
}

HRESULT RenderingPipeLine::CreateRenderResource()
{
	auto heapDesc = DirectXGraphics::GetInstance().GetRTVHeap()->GetDesc();

	auto& backbuff = DirectXGraphics::GetInstance().GetBackBuffers()[0];
	auto resDesc = backbuff->GetDesc();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	auto clearColor = DirectXGraphics::GetInstance().GetClearColor();


	clerColorArray[0] = clearColor.R();
	clerColorArray[1] = clearColor.G();
	clerColorArray[2] = clearColor.B();
	clerColorArray[3] = clearColor.A();

	D3D12_CLEAR_VALUE claeValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clerColorArray.data());

	D3D12_HEAP_PROPERTIES heapProp =
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	// リソースの生成
	auto result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_OutputRenderResource.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(_processed_resource.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_RaytracingResource.ReleaseAndGetAddressOf()));

	m_OutputRenderResource->SetName(L"OutputRenderResource");
	_processed_resource->SetName(L"ProcessedResource");
	m_RaytracingResource->SetName(L"RaytracingResource");

	return result;
}

HRESULT RenderingPipeLine::CreateBloomResource()
{
	// バッファの生成
	m_BloomBuffer.resize(m_BloomBufferCount);
	auto& bbuff = DirectXGraphics::GetInstance().GetBackBuffers()[0];
	auto resDesc = bbuff->GetDesc();

	auto clearColor = DirectXGraphics::GetInstance().GetClearColor();
	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	//clearValue.Format = resDesc.Format;
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


	for (auto& res : m_BloomBuffer)
	{
		DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&clearValue,
			IID_PPV_ARGS(res.ReleaseAndGetAddressOf()));

		// これをすればメモリ削減になる
		resDesc.Width /= 2;
		resDesc.Height /= 2;
	}

	return S_OK;
}

HRESULT RenderingPipeLine::CreateBlurWeightResource()
{
	// ブラー用パラメータのバッファ作成
	auto weights = MathUtility::GetGaussianWeights(8, 1.2f);
	int size = (sizeof(weights[0]) * weights.size() + 0xff) & ~0xff;
	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_BlurPramResource.ReleaseAndGetAddressOf()));


	// リソースの転送
	float* mappedWeight = nullptr;
	m_BlurPramResource->Map(0, nullptr, (void**)&mappedWeight);
	std::copy(weights.begin(), weights.end(), mappedWeight);
	m_BlurPramResource->Unmap(0, nullptr);

	return S_OK;
}

HRESULT RenderingPipeLine::CreatePipeLines()
{
	// あとで　ここに並べていく

	// Bloom
	if (CreateBloomPSO() != S_OK)
	{
		throw std::runtime_error("PSOの生成が出来ませんでした。");
	}

	if(CreateResultPSO() != S_OK)
	{
		throw std::runtime_error("PSOの生成が出来ませんでした。");
	}

	// 最終的なリザルト結果
	return S_OK;
}

HRESULT RenderingPipeLine::CreateBloomPSO()
{
	// パイプラインの作成
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;



	D3D12_DESCRIPTOR_RANGE range[4] = {};

	// 通常カラー
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;

	// 高輝度
	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[1].BaseShaderRegister = 1;
	range[1].NumDescriptors = 1;

	// 縮小高輝度、
	range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[2].BaseShaderRegister = 2;
	range[2].NumDescriptors = 4;

	// ブラー用の定数
	range[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[3].BaseShaderRegister = 0;
	range[3].NumDescriptors = 1;



	D3D12_ROOT_PARAMETER rp[4] = {};

	rp[Bloom_RootParamter_NormalColor].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[Bloom_RootParamter_NormalColor].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[Bloom_RootParamter_NormalColor].DescriptorTable.pDescriptorRanges = &range[0];
	rp[Bloom_RootParamter_NormalColor].DescriptorTable.NumDescriptorRanges = 1;

	rp[Bloom_RootParamter_HighLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[Bloom_RootParamter_HighLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[Bloom_RootParamter_HighLight].DescriptorTable.pDescriptorRanges = &range[1];
	rp[Bloom_RootParamter_HighLight].DescriptorTable.NumDescriptorRanges = 1;


	rp[Bloom_RootParamter_HighLightReduction].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[Bloom_RootParamter_HighLightReduction].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[Bloom_RootParamter_HighLightReduction].DescriptorTable.pDescriptorRanges = &range[2];
	rp[Bloom_RootParamter_HighLightReduction].DescriptorTable.NumDescriptorRanges = 1;

	rp[Bloom_RootParamter_BlurParameter].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[Bloom_RootParamter_BlurParameter].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[Bloom_RootParamter_BlurParameter].DescriptorTable.pDescriptorRanges = &range[3];
	rp[Bloom_RootParamter_BlurParameter].DescriptorTable.NumDescriptorRanges = 1;




	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;


	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(rp);
	rsDesc.pParameters = rp;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	ComPtr<ID3DBlob> rsBlob;
	ComPtr<ID3DBlob> errBlob;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());

	result = DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(posteffect_bloom_hightLight_pso.rootSignature.ReleaseAndGetAddressOf()));

	// シェーダー
	auto vs = ShaderManager::GetInstance().GetShader("PEBloomVS");
	auto ps = ShaderManager::GetInstance().GetShader("PEBloomPS");

	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 通常カラー
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	gpsDesc.pRootSignature = posteffect_bloom_hightLight_pso.rootSignature.Get();
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(posteffect_bloom_hightLight_pso.pso.ReleaseAndGetAddressOf()));

	auto ps_2 = ShaderManager::GetInstance().GetShader("PEBloomBlur0");

	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps_2);

	// 2つ目のパイプラインを生成
	result = DirectXDevice::GetInstance().GetDevice()
		->CreateGraphicsPipelineState(&gpsDesc,
			IID_PPV_ARGS(posteffect_bloomblur_vertical_pso.pso.ReleaseAndGetAddressOf()));

	auto ps_3 = ShaderManager::GetInstance().GetShader("PEBloomBlur1");
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps_3);

	// 3つ目のパイプラインを生成
	result = DirectXDevice::GetInstance().GetDevice()
		->CreateGraphicsPipelineState(&gpsDesc,
			IID_PPV_ARGS(posteffect_bloomblur_horizontal_pso.pso.ReleaseAndGetAddressOf()));

	return result;
}

HRESULT RenderingPipeLine::CreateResultPSO()
{
	// パイプラインの作成
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;

	D3D12_DESCRIPTOR_RANGE range[1] = {};

	// レンダリング結果
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;

	D3D12_ROOT_PARAMETER rp[1] = {};

	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[0].DescriptorTable.pDescriptorRanges = &range[0];
	rp[0].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;


	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(rp);
	rsDesc.pParameters = rp;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	ComPtr<ID3DBlob> rsBlob;
	ComPtr<ID3DBlob> errBlob;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());

	result = DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(posteffect_result_pso.rootSignature.ReleaseAndGetAddressOf()));

	// シェーダー
	// 頂点シェーダーはBloomを再利用
	auto vs = ShaderManager::GetInstance().GetShader("PE_PeraPolygonVS");
	auto ps = ShaderManager::GetInstance().GetShader("PE_PeraPolygonPS");

	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 通常カラー
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	gpsDesc.pRootSignature = posteffect_result_pso.rootSignature.Get();
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(posteffect_result_pso.pso.ReleaseAndGetAddressOf()));

	return result;
}


HRESULT RenderingPipeLine::CreatePeraPolygon()
{
	struct PeraVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	PeraVertex pv[4] = { {{-1,-1,0.1},{0,1}},
						{{-1,1,0.1},{0,0}},
						{{1,-1,0.1},{1,1}},
						{{1,1,0.1},{1,0}} };


	// 頂点バッファの準備
	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_peraVB.ReleaseAndGetAddressOf()));



	PeraVertex* mappedPera = nullptr;
	m_peraVB->Map(0, nullptr, (void**)&mappedPera);
	std::copy(std::begin(pv), std::end(pv), mappedPera);

	m_peraVB->Unmap(0, nullptr);

	m_PeraVBV.BufferLocation = m_peraVB->GetGPUVirtualAddress();
	m_PeraVBV.SizeInBytes = sizeof(pv);
	m_PeraVBV.StrideInBytes = sizeof(PeraVertex);


	return result;
}

void RenderingPipeLine::RenderingHighLight()
{
	// 高輝度
	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_BloomBuffer[0].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_RaytracingResource.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


	auto bloomBufferPointer = m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart();
	bloomBufferPointer.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_pCommandList->OMSetRenderTargets(1, &bloomBufferPointer, false, nullptr);
	m_pCommandList->ClearRenderTargetView(bloomBufferPointer, clerColorArray.data(), 0, nullptr);


	m_pCommandList->SetPipelineState(posteffect_bloom_hightLight_pso.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(posteffect_bloom_hightLight_pso.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	auto srvHandle = m_peraSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * (m_BloomBufferCount + m_BlurWeights + ResultBloomResource + RaytracingRenderResource);

	m_pCommandList->SetDescriptorHeaps(1, m_peraSRVHeap.GetAddressOf());
	// 生リソースをセット
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);
	m_pCommandList->DrawInstanced(4, 1, 0, 0);

}

void RenderingPipeLine::DefaultRendering()
{
	DefaultRenderingBegin();
	if (m_pSkyBox)
	{
		m_pSkyBox->Draw();
	}
	SpriteDrawer::GetInstance().Render3DSprite();
	MeshDrawer::GetInstance().Draw();
	ParticleManager::GetInstance().Draw();
	DefaultRenderingEnd();
}

void RenderingPipeLine::DrawPostEffect()
{
	//RenderingCubeMap();

	//BeginDrawShadow();
	//MeshDrawer::GetInstance().ShadowDraw();


	//// 深度だけ書き込み
	//auto dsvHeaps = DirectXGraphics::GetInstance().GetDSVHeap();
	//auto dsvH = dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	//dsvH.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//dsvH.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//m_pCommandList->OMSetRenderTargets(0, nullptr, false, &dsvH);
	//m_pCommandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//MeshDrawer::GetInstance().Draw();

	// DOF 被写界深度
	//EffectDepthOfField();
	//Bloom
	
	EffectBloom();
}

HRESULT RenderingPipeLine::InitCubeMapResource()
{


	D3D12_RESOURCE_DESC texDesc;

	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = 1920;
	texDesc.Height = 1080;

	// 6つのテクスチャ配列
	texDesc.DepthOrArraySize = 6;

	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	if (DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mCubeMapTex)) != S_OK)
	{
		return S_FALSE;
	}

	mCubeMapTex->SetName(L"CubeMapTexture");



	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = 6; // テクスチャ分
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// RTVデクスクリプタヒープ作成
	if (DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(mCubeMapDescRTVHeaps.GetAddressOf())) != S_OK)
	{
		return S_FALSE;
	}

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NodeMask = 0;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// SRVデクスクリプタヒープ作成
	if (DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&srvHeapDesc, IID_PPV_ARGS(mCubeMapDescSRVHeaps.GetAddressOf())) != S_OK)
	{
		return S_FALSE;
	}

	auto rtvHandle = mCubeMapDescRTVHeaps->GetCPUDescriptorHandleForHeapStart();
	auto srvHandle = mCubeMapDescSRVHeaps->GetCPUDescriptorHandleForHeapStart();
	const auto rtvSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.PlaneSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;

	// RTViewの作成
	for (int i = 0; i < 6; ++i)
	{
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE
		(
			rtvHandle, i, rtvSize
		);

		rtvDesc.Texture2DArray.FirstArraySlice = i;
		DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
			mCubeMapTex.Get(),
			&rtvDesc,
			handle
		);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// ここ大事やで
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	// SRviewの作成
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		mCubeMapTex.Get(),
		&srvDesc,
		srvHandle
	);


	// カメラのセッティング
	mCubeMapCameraTargets.resize(6);
	mCubeMapCameraUps.resize(6);

	mCubeMapCameraTargets[0] = SimpleMath::Vector3(mCubeMapCameraCenterPos.x + 1.0f, mCubeMapCameraCenterPos.y, mCubeMapCameraCenterPos.z); // +X
	mCubeMapCameraTargets[1] = SimpleMath::Vector3(mCubeMapCameraCenterPos.x - 1.0f, mCubeMapCameraCenterPos.y, mCubeMapCameraCenterPos.z); // -X

	mCubeMapCameraTargets[2] = SimpleMath::Vector3(mCubeMapCameraCenterPos.x, mCubeMapCameraCenterPos.y + 1.0f, mCubeMapCameraCenterPos.z); // +Y
	mCubeMapCameraTargets[3] = SimpleMath::Vector3(mCubeMapCameraCenterPos.x, mCubeMapCameraCenterPos.y - 1.0f, mCubeMapCameraCenterPos.z); // -Y

	mCubeMapCameraTargets[4] = SimpleMath::Vector3(mCubeMapCameraCenterPos.x, mCubeMapCameraCenterPos.y, mCubeMapCameraCenterPos.z + 1.0f); // +Z
	mCubeMapCameraTargets[5] = SimpleMath::Vector3(mCubeMapCameraCenterPos.x, mCubeMapCameraCenterPos.y, mCubeMapCameraCenterPos.z - 1.0f); // -Z


	mCubeMapCameraUps[0] = SimpleMath::Vector3(0, 1.0f, 0); // +X
	mCubeMapCameraUps[1] = SimpleMath::Vector3(0, 1.0f, 0); // -X

	mCubeMapCameraUps[2] = SimpleMath::Vector3(0, 0.0f, -1.0f); // +Y
	mCubeMapCameraUps[3] = SimpleMath::Vector3(0, 0.0f, 1.0f); // -Y

	mCubeMapCameraUps[4] = SimpleMath::Vector3(0, 1.0f, 0.0f); // +Z
	mCubeMapCameraUps[5] = SimpleMath::Vector3(0, 1.0f, 0.0f); // -Z

	mCubeMapRenderRect = { 0,0,1920,1080 };
	mCubeMapRenderViewPort = { 0.0f,0.0f,(float)1920,(float)1080,0.0f,1.0f };

	return S_OK;
}

void RenderingPipeLine::RenderingCubeMap()
{
	auto pCommandList = DirectXGraphics::GetInstance().GetCommandList();

	// キューブマップを描画可能に
	pCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			mCubeMapTex.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

	pCommandList->RSSetScissorRects(1, &mCubeMapRenderRect);
	pCommandList->RSSetViewports(1, &mCubeMapRenderViewPort);

	DirectXGraphics::GetInstance().SetViewPort();

	auto tempclearColor = DirectXGraphics::GetInstance().GetClearColor();
	auto rtvHandle = mCubeMapDescRTVHeaps->GetCPUDescriptorHandleForHeapStart();
	const auto rtvSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 今回は毎回カメラを切り替える
	auto camera = CameraManager::GetInstance().GetMainCamera();
	auto initpos = camera->GetPosition();
	auto initaspect = camera->GetAspect();
	auto initFov = camera->GetFov();
	auto initCameraTarget = camera->GetTarget();
	auto initCameraUP = camera->GetUp();

	//camera->SetFov(XMConvertToRadians(45.0f));
	//camera->SetAspect(1.0f);
	camera->SetPosition(SimpleMath::Vector3(0, 0, 0));

	auto dsvHeap = DirectXGraphics::GetInstance().GetDSVHeap();
	auto dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();

	std::array<float, 4> clearColor0{ 1,0,0,1 };
	std::array<float, 4> clearColor1{ 0,1,0,1 };
	std::array<float, 4> clearColor2{ 0,0,1,1 };
	std::array<float, 4> clearColor3{ 1,1,0,1 };
	std::array<float, 4> clearColor4{ 0,1,1,1 };
	std::array<float, 4> clearColor5{ 1,1,1,1 };


	std::vector<std::array<float, 4>> clearColors;
	clearColors.push_back(clearColor0);
	clearColors.push_back(clearColor1);
	clearColors.push_back(clearColor2);
	clearColors.push_back(clearColor3);
	clearColors.push_back(clearColor4);
	clearColors.push_back(clearColor5);

	UpdateCubeMapCameraTargets(camera->GetPosition());


	for (int i = 0; i < 6; ++i)
	{

		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE
		(
			rtvHandle, i, rtvSize
		);
		m_pCommandList->OMSetRenderTargets(1, &handle, false, &dsvHandle);
		m_pCommandList->ClearRenderTargetView(handle, clearColors[i].data(), 0, nullptr);
		m_pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		camera->SetTarget(mCubeMapCameraTargets[i]);
		camera->SetUp(mCubeMapCameraUps[i]);
		//auto view = SimpleMath::Matrix::CreateLookAt(camera->GetPosition(), mCubeMapCameraTargets[i], SimpleMath::Vector3::Up);
		//auto proj = camera->GetProjectMat();

		if (m_pSkyBox)
		{
			m_pSkyBox->Draw();
		}
		// TODO:ここから、MehsDrawer側の関数を呼び出して、定数のセットを行う予定
		MeshDrawer::GetInstance().CubeMapDraw(i);
	}

	// カメラの状態を初期に戻す
	camera->SetPosition(initpos);
	camera->SetTarget(initCameraTarget);
	camera->SetUp(initCameraUP);
	camera->SetFov(initFov);
	camera->SetAspect(initaspect);

	pCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			mCubeMapTex.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_GENERIC_READ
		)
	);
}

void RenderingPipeLine::UpdateCubeMapCameraTargets(const SimpleMath::Vector3& cameraPos)
{
	mCubeMapCameraCenterPos = cameraPos;

	mCubeMapCameraTargets[0] = SimpleMath::Vector3(+1.0f, 0, 0); // +X
	mCubeMapCameraTargets[1] = SimpleMath::Vector3(-1.0f, 0, 0); // -X

	mCubeMapCameraTargets[2] = SimpleMath::Vector3(0, +1.0f, 0); // +Y
	mCubeMapCameraTargets[3] = SimpleMath::Vector3(0, -1.0f, 0); // -Y

	mCubeMapCameraTargets[4] = SimpleMath::Vector3(0, 0, +1.0f); // +Z
	mCubeMapCameraTargets[5] = SimpleMath::Vector3(0, 0, -1.0f); // -Z


}

