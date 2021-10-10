#ifndef _DIRECTX_GRAPHICS_H_
#define _DIRECTX_GRAPHICS_H_

#include <d3dcompiler.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <map>
#include <string>
#include <vector>
#include <array>
#include <wrl/client.h>

#include "../Singleton.h"
#include "../../DirectXTK12/Inc/SimpleMath.h"
#include "../../DirectXTK12/Inc/GeometricPrimitive.h"
#include "Core/PSOManager.h"

#pragma comment(lib,"d3dcompiler.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;
class FontSystem;

class DirectXGraphics :
	public Singleton<DirectXGraphics>
{
public:
	friend class Singleton<DirectXGraphics>;
	HRESULT Init();

	bool Begin();
	bool End();

	// 以前の描画結果を初期化しない
	// ポストエフェクトをする前提で使用する
	bool OverlapBegin();
	bool Clear();

	bool BeginShadow();
	bool EndCommand();

	bool FontSystemBegin();
	bool FontSystemEnd();

	void SetClearColor(const DirectX::SimpleMath::Color& color);

	const SimpleMath::Color& GetClearColor();


	void SetRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE& handle,bool isClear = true);
	void SetRenderTarget(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& handles);

	void ClearDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, D3D12_CLEAR_FLAGS _clearFlag, float _depth, UINT8 _stencil, UINT _numRects,const D3D12_RECT* _pRect = nullptr);
	void CreateImGUIDescriptrHeap();

	ID3D12DescriptorHeap* GetImGUIDescriptrHeap();
	ID3D12DescriptorHeap* GetDepthSRVHeap();
	ID3D12DescriptorHeap* GetDSVHeap();
	ID3D12DescriptorHeap* GetRTVHeap();

	ID3D12GraphicsCommandList5* GetCommandList();
	IDXGISwapChain4* GetSwapChain();

	HRESULT SetDefaultScissorRect(const D3D12_RECT& rect);
	HRESULT SetDefaultViewPort(const D3D12_VIEWPORT& view);

	HRESULT ClearRenderTargetView(const D3D12_CPU_DESCRIPTOR_HANDLE& handle);

	HRESULT SetScissorRect(const D3D12_RECT& rect);
	HRESULT SetScissorRect();

	HRESULT SetViewPort(const D3D12_VIEWPORT& view);
	HRESULT SetViewPort();

	HRESULT InitScissorRect();
	HRESULT InitViewPort();
	HRESULT InitFontSystem(const wchar_t * path);

	std::vector<ID3D12Resource*>& GetBackBuffers();

	void DrawFont(const std::string& text, DirectX::SimpleMath::Vector2 pos, DirectX::SimpleMath::Vector2 scale, DirectX::SimpleMath::Color color);
	void DrawFont(const wchar_t* text, DirectX::SimpleMath::Vector2 pos, DirectX::SimpleMath::Vector2 scale, DirectX::SimpleMath::Color color);

	const D3D12_RECT& GetRect();
	const D3D12_VIEWPORT& GetViewPort();

	ID3D12CommandQueue* GetCmdQueue();

	void Present();

protected:
	DirectXGraphics();
	virtual ~DirectXGraphics();


private:

	bool WaitForPreviousFrame();
	bool WaitForGpu();

	HRESULT CreateCommandList();
	HRESULT CreateCommandAloocator();
	HRESULT CreateCommandQueue();
	HRESULT CreateSwapChain();
	HRESULT CreateFence();
	HRESULT InitRenderTargetView();

	void GenerateDepthResource();

private:

	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<ID3D12DescriptorHeap> m_RtvHeaps;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeaps;
	ComPtr<ID3D12GraphicsCommandList5> m_CommandList;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12DescriptorHeap> m_ImGUIHeaps;
	ComPtr<IDXGISwapChain4> m_Swapchain;
	std::vector<ID3D12Resource*> m_BackBuffers;
	UINT64 m_FenceVal;
	std::array<float, 4> m_ClearColor;
	D3D12_RESOURCE_BARRIER m_BarrierDesc;


	D3D12_RECT m_DeffaultScissorcect;
	D3D12_VIEWPORT m_DeffaultViewport;

	//　通常の描画を行う用の深度バッファ
	ComPtr<ID3D12Resource> m_Default_DepthBuffer;

	FontSystem* m_pFontSystem;

private:

	ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap; // レンダーターゲットビューヒープ
	ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap; // SRVヒープ

	ComPtr<ID3D12DescriptorHeap> m_DepthSRVHeap; // 深度値テクスチャ用

	ComPtr<ID3D12Resource> m_LightDepthBuffer;

	ComPtr<ID3D12Resource> m_FluidDepthBuffer;

};

#endif
