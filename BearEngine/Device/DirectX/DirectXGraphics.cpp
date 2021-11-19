#include "DirectXGraphics.h"
#include "DirectXDevice.h"
#include "../WindowApp.h"
#include <DirectXMath.h>
#include "Core/PSOManager.h"
#include "FontSystem.h"
#include "../../imgui/imgui_impl_win32.h"
#include "../../imgui/imgui_impl_dx12.h"
#include "../../DirectXTK12/Inc/GeometricPrimitive.h"
#include "Core/ShaderManager.h"

using namespace DirectX;

constexpr uint32_t shadow_difinition = 1024;


DirectXGraphics::DirectXGraphics()
{
	m_ClearColor[0] = 0.0f;
	m_ClearColor[1] = 0.0f;
	m_ClearColor[2] = 0.0f;
	m_ClearColor[3] = 0.0f;
}

DirectXGraphics::~DirectXGraphics()
{
	delete m_pFontSystem;

	for (auto itr = m_BackBuffers.begin(); itr != m_BackBuffers.end(); itr++)
	{
		(*itr)->Release();
	}

}

HRESULT DirectXGraphics::Init()
{
	HRESULT result = S_OK;

	if (CreateCommandAloocator() != S_OK)
	{
		return result;
	}

	if (CreateCommandList() != S_OK)
	{
		return result;
	}

	if (CreateCommandQueue() != S_OK)
	{
		return result;
	}

	if (CreateSwapChain() != S_OK)
	{
		return result;
	}

	if (InitRenderTargetView() != S_OK)
	{
		return result;
	}

	if (CreateFence() != S_OK)
	{
		return result;
	}

	if (InitScissorRect() != S_OK)
	{
		return result;
	}

	if (InitViewPort() != S_OK)
	{
		return result;
	}

	CreateImGUIDescriptrHeap();
	GenerateDepthResource();

	//if (InitBloom() != S_OK)
	//{
	//	return result;
	//}

	//if (GeneratePeraResource() != S_OK)
	//{
	//	return result;
	//}


	return result;
}

HRESULT DirectXGraphics::SetDefaultScissorRect(const D3D12_RECT& rect)
{
	m_DeffaultScissorcect = rect;
	return S_OK;
}

HRESULT DirectXGraphics::SetDefaultViewPort(const D3D12_VIEWPORT& view)
{
	m_DeffaultViewport = view;
	return S_OK;
}

HRESULT DirectXGraphics::ClearRenderTargetView(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
	m_CommandList.Get()->ClearRenderTargetView(handle, m_ClearColor.data(), 0, nullptr);

	return S_OK;
}

HRESULT DirectXGraphics::SetScissorRect(const D3D12_RECT& rect)
{
	m_CommandList.Get()->RSSetScissorRects(1, &rect);
	return S_OK;
}

HRESULT DirectXGraphics::SetViewPort(const D3D12_VIEWPORT& view)
{
	m_CommandList.Get()->RSSetViewports(1, &view);
	return S_OK;
}

HRESULT DirectXGraphics::SetViewPort()
{
	m_CommandList.Get()->RSSetViewports(1, &m_DeffaultViewport);
	return S_OK;
}

HRESULT DirectXGraphics::SetScissorRect()
{
	m_CommandList.Get()->RSSetScissorRects(1, &m_DeffaultScissorcect);
	return S_OK;
}

HRESULT DirectXGraphics::InitScissorRect()
{
	D3D12_VIEWPORT viewport{};
	int width = WindowApp::GetInstance().GetWindowSize().window_Width;
	int height = WindowApp::GetInstance().GetWindowSize().window_Height;

	viewport.Width = width;
	viewport.Height = height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_DeffaultViewport = viewport;

	return S_OK;
}

HRESULT DirectXGraphics::InitViewPort()
{
	int width = WindowApp::GetInstance().GetWindowSize().window_Width;
	int height = WindowApp::GetInstance().GetWindowSize().window_Height;

	D3D12_RECT scissorcect{};
	scissorcect.left = 0;
	scissorcect.right = scissorcect.left + width;
	scissorcect.top = 0;
	scissorcect.bottom = scissorcect.top + height;

	m_DeffaultScissorcect = scissorcect;

	return S_OK;
}

HRESULT DirectXGraphics::InitFontSystem(const wchar_t* path)
{
	m_pFontSystem = new FontSystem();
	m_pFontSystem->Init(path);
	return S_OK;
}
std::vector<ID3D12Resource*>& DirectXGraphics::GetBackBuffers()
{
	return m_BackBuffers;
}

void DirectXGraphics::DrawFont(const std::string& text, DirectX::SimpleMath::Vector2 pos, DirectX::SimpleMath::Vector2 scale, DirectX::SimpleMath::Color color)
{

	XMFLOAT2 t_pos = XMFLOAT2(pos.x, pos.y);
	XMFLOAT2 t_scale = XMFLOAT2(scale.x, scale.y);
	XMVECTORF32 t_color = XMVECTORF32{ color.R(),color.G(),color.B(),color.A() };

	// 後で分離させる
	m_pFontSystem->Draw(text, t_pos, t_scale, t_color);

}


void DirectXGraphics::DrawFont(const wchar_t* text, DirectX::SimpleMath::Vector2 pos, DirectX::SimpleMath::Vector2 scale, DirectX::SimpleMath::Color color)
{

	XMFLOAT2 t_pos = XMFLOAT2(pos.x, pos.y);
	XMFLOAT2 t_scale = XMFLOAT2(scale.x, scale.y);
	XMVECTORF32 t_color = XMVECTORF32{ color.R(),color.G(),color.B(),color.A() };

	// 後で分離させる
	m_pFontSystem->Draw(text, t_pos, t_scale, t_color);

}

const D3D12_RECT& DirectXGraphics::GetRect()
{
	return m_DeffaultScissorcect;
}

const D3D12_VIEWPORT& DirectXGraphics::GetViewPort()
{
	return m_DeffaultViewport;
}

ID3D12CommandQueue* DirectXGraphics::GetCmdQueue()
{
	return m_CommandQueue.Get();
}


bool DirectXGraphics::Begin()
{
	UINT bbIndex = m_Swapchain.Get()->GetCurrentBackBufferIndex();

	m_BarrierDesc.Transition.pResource = m_BackBuffers[bbIndex];
	m_BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	m_BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	m_CommandList.Get()->ResourceBarrier(1, &m_BarrierDesc);


	// レンダーターゲット指定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH =
		m_RtvHeaps.Get()->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIndex * DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	SetRenderTarget(rtvH);

	m_CommandList.Get()->RSSetScissorRects(1, &m_DeffaultScissorcect);
	m_CommandList.Get()->RSSetViewports(1, &m_DeffaultViewport);



	return true;
}

bool DirectXGraphics::End()
{
	ImGui::Render();

	m_CommandList->SetDescriptorHeaps(
		1, m_ImGUIHeaps.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList.Get());


	UINT bbIndex = m_Swapchain.Get()->GetCurrentBackBufferIndex();
	m_BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	m_CommandList->ResourceBarrier(1, &m_BarrierDesc);
	EndCommand();

	return true;
}

bool DirectXGraphics::OverlapBegin()
{
	UINT bbIndex = m_Swapchain.Get()->GetCurrentBackBufferIndex();

	m_BarrierDesc.Transition.pResource = m_BackBuffers[bbIndex];
	m_BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	m_BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	m_BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	m_CommandList.Get()->ResourceBarrier(1, &m_BarrierDesc);


	// レンダーターゲット指定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH =
		m_RtvHeaps.Get()->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIndex * DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = m_DsvHeaps->GetCPUDescriptorHandleForHeapStart();
	m_CommandList.Get()->OMSetRenderTargets(1, &rtvH, false, &dsvH);

	m_CommandList.Get()->RSSetScissorRects(1, &m_DeffaultScissorcect);
	m_CommandList.Get()->RSSetViewports(1, &m_DeffaultViewport);



	return true;
}

bool DirectXGraphics::Clear()
{
	UINT bbIndex = m_Swapchain.Get()->GetCurrentBackBufferIndex();
	
	D3D12_CPU_DESCRIPTOR_HANDLE rtvH =
		m_RtvHeaps.Get()->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIndex * DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = m_DsvHeaps->GetCPUDescriptorHandleForHeapStart();

	
	m_CommandList.Get()->ClearRenderTargetView(rtvH, m_ClearColor.data(), 0, nullptr);
	m_CommandList.Get()->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	return true;
}

bool DirectXGraphics::BeginShadow()
{
	// ライトデプスにセット
	auto handle = m_DsvHeaps->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	// レンダーターゲットのセットなし(深度値のみ）
	m_CommandList->OMSetRenderTargets(0, nullptr, false, &handle);


	m_CommandList->ClearDepthStencilView(handle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	D3D12_VIEWPORT vp = CD3DX12_VIEWPORT(0.0f, 0.0f, shadow_difinition, shadow_difinition);
	m_CommandList->RSSetViewports(1, &vp);//ビューポート

	CD3DX12_RECT rc(0, 0, 1920, 1080);
	m_CommandList->RSSetScissorRects(1, &rc);//シザー(切り抜き)矩形

	return true;
}

bool DirectXGraphics::EndCommand()
{
	// 命令を閉じる
	m_CommandList->Close();
	ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(1, cmdLists);

	// GPUを待つ
	WaitForGpu();
	WaitForPreviousFrame();


	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	return true;
}

bool DirectXGraphics::FontSystemBegin()
{
	ID3D12DescriptorHeap* heaps[] = { m_pFontSystem->GetDescHeap().Get() };

	m_CommandList->SetDescriptorHeaps(1, heaps);

	m_pFontSystem->Begin();
	m_pFontSystem->PushCommand();
	return true;
}

bool DirectXGraphics::FontSystemEnd()
{
	m_pFontSystem->End();
	return true;
}

const SimpleMath::Color& DirectXGraphics::GetClearColor()
{
	return SimpleMath::Color(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
}

void DirectXGraphics::CreateImGUIDescriptrHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_ImGUIHeaps.GetAddressOf()));
}

const int DirectXGraphics::GetBackBufferIndex()
{
	return m_Swapchain.Get()->GetCurrentBackBufferIndex();
}

ID3D12DescriptorHeap* DirectXGraphics::GetImGUIDescriptrHeap()
{
	return m_ImGUIHeaps.Get();
}

ID3D12DescriptorHeap* DirectXGraphics::GetDepthSRVHeap()
{
	return m_DepthSRVHeap.Get();
}

ID3D12DescriptorHeap* DirectXGraphics::GetDSVHeap()
{
	return m_DsvHeaps.Get();
}

ID3D12DescriptorHeap* DirectXGraphics::GetRTVHeap()
{
	return m_RtvHeaps.Get();
}

void DirectXGraphics::SetClearColor(const DirectX::SimpleMath::Color& color)
{
	m_ClearColor[0] = color.R();
	m_ClearColor[1] = color.G();
	m_ClearColor[2] = color.B();
	m_ClearColor[3] = color.A();
}

ID3D12GraphicsCommandList5* DirectXGraphics::GetCommandList()
{
	return m_CommandList.Get();
}

IDXGISwapChain4* DirectXGraphics::GetSwapChain()
{
	return m_Swapchain.Get();
}

void DirectXGraphics::ResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore,
	D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = pResource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;

	m_CommandList->ResourceBarrier(1, &barrier);
}

bool DirectXGraphics::WaitForPreviousFrame()
{
	if (m_Fence->GetCompletedValue() != m_FenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		m_Fence->SetEventOnCompletion(m_FenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	return true;
}

bool DirectXGraphics::WaitForGpu()
{
	m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal);

	return true;
}

void DirectXGraphics::Present()
{
	if(GetSwapChain()->Present(1, 0) == DXGI_ERROR_DEVICE_REMOVED)
	{
		DirectXDevice::GetInstance().DeviceRemovedHandler();
	}
	m_pFontSystem->Commit();
}

HRESULT DirectXGraphics::InitRenderTargetView()
{
	HRESULT result = S_OK;

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	result = m_Swapchain.Get()->GetDesc1(&desc);




	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = 2;
	heapDesc.NodeMask = 0;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_RtvHeaps));


	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = m_Swapchain.Get()->GetDesc(&swcDesc);
	m_BackBuffers.resize(swcDesc.BufferCount);



	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_RtvHeaps.Get()->GetCPUDescriptorHandleForHeapStart();


	for (int i = 0; i < 2; i++)
	{
		result = m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]));

		//rtvDesc.Format = m_BackBuffers[i]->GetDesc().Format;
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
			m_BackBuffers[i],
			&rtvDesc,
			handle
		);

		handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// 深度バッファ作成
	D3D12_RESOURCE_DESC depthResDesc{};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = WindowApp::GetInstance().GetWindowSize().window_Width;
	depthResDesc.Height = WindowApp::GetInstance().GetWindowSize().window_Height;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES depthHeapProp{};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&m_Default_DepthBuffer)
	);

	result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(m_FluidDepthBuffer.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(m_LightDepthBuffer.ReleaseAndGetAddressOf()));



	
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 3; // 通常深度とライト深度（影用）と通常深度（流体用）
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	result = DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeaps)
	);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	handle = m_DsvHeaps->GetCPUDescriptorHandleForHeapStart();

	
	// 通常デプス
	DirectXDevice::GetInstance().GetDevice()->CreateDepthStencilView(
		m_Default_DepthBuffer.Get(),
		&dsvDesc,
		handle
	);

	// ライトデプス
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	DirectXDevice::GetInstance().GetDevice()->CreateDepthStencilView(
		m_LightDepthBuffer.Get(),
		&dsvDesc,
		handle
	);

	// 流体用通常デプス（通常デプスの場合、書き換えが想定されるため）
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	DirectXDevice::GetInstance().GetDevice()->CreateDepthStencilView(
		m_FluidDepthBuffer.Get(),
		&dsvDesc,
		handle
	);




	return result;
}


void DirectXGraphics::SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, bool isClear)
{
	auto dsvH = m_DsvHeaps->GetCPUDescriptorHandleForHeapStart();
	m_CommandList.Get()->OMSetRenderTargets(1, &handle, false, &dsvH);

	if(isClear)
	{
		// 画面をクリア
		ClearRenderTargetView(handle);
	}

	ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}



void DirectXGraphics::SetRenderTarget(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& handles)
{
	auto dsvH = m_DsvHeaps->GetCPUDescriptorHandleForHeapStart();
	m_CommandList.Get()->OMSetRenderTargets(handles.size(), handles.data(), false, &dsvH);

	for (auto handle : handles)
	{
		// 画面をクリア
		ClearRenderTargetView(handle);
	}

	ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void DirectXGraphics::ClearDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, D3D12_CLEAR_FLAGS _clearFlag, float _depth, UINT8 _stencil, UINT _numRects, const D3D12_RECT* _pRect)
{
	m_CommandList->ClearDepthStencilView(handle, _clearFlag, _depth, _stencil, _numRects, _pRect);
}

void DirectXGraphics::GenerateDepthResource()
{
	// ヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	// TODO:後で、Depth用のSRVは分離すべきだと思う
	heapDesc.NumDescriptors = 3; // 通常デプス ▶ テクスチャ　＆　ライトデプス ▶ テクスチャ  & 通常デブス　▶テクスチャ（流体レンダリング用）

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	auto result = DirectXDevice::GetInstance().GetDevice()
		->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DepthSRVHeap));

	// ビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
	resDesc.Texture2D.MipLevels = 1;
	resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	auto handle = m_DepthSRVHeap->GetCPUDescriptorHandleForHeapStart();

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_Default_DepthBuffer.Get(),
		&resDesc,
		handle);

	// シャドウマップ用リソース
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_LightDepthBuffer.Get(),
		&resDesc,
		handle);

	// シャドウマップ用リソース
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_FluidDepthBuffer.Get(),
		&resDesc,
		handle);

}



HRESULT DirectXGraphics::CreateCommandAloocator()
{
	HRESULT result = S_OK;

	result = DirectXDevice::GetInstance().GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
	                                                                          IID_PPV_ARGS(&m_CommandAllocator));

	return result;
}

HRESULT DirectXGraphics::CreateCommandQueue()
{
	HRESULT result = S_OK;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = DirectXDevice::GetInstance().GetDevice()->CreateCommandQueue(&cmdQueueDesc,
	                                                                      IID_PPV_ARGS(&m_CommandQueue));

	return result;
}

HRESULT DirectXGraphics::CreateCommandList()
{
	HRESULT result = S_OK;

	result = DirectXDevice::GetInstance().GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
	                                                                     m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));

	return result;
}

HRESULT DirectXGraphics::CreateSwapChain()
{
	HRESULT result = S_OK;



	// 各種設定してスワップチェーンを生成
	DXGI_SWAP_CHAIN_DESC1 swapcChainDesc{};

	swapcChainDesc.Width = WindowApp::GetInstance().GetWindowSize().window_Width;
	swapcChainDesc.Height = WindowApp::GetInstance().GetWindowSize().window_Height;
	swapcChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapcChainDesc.SampleDesc.Count = 1;
	swapcChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapcChainDesc.BufferCount = 2;
	swapcChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapcChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapchain1;

	result = DirectXDevice::GetInstance().GetDxgiFactory()->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),
		WindowApp::GetInstance().GetHWND(),
		&swapcChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&swapchain1);

	if (result != S_OK)
	{
		assert(0);
	}

	swapchain1.As(&m_Swapchain);

	return result;
}

HRESULT DirectXGraphics::CreateFence()
{
	HRESULT result = S_OK;

	result = DirectXDevice::GetInstance().GetDevice()->CreateFence(m_FenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

	return result;
}