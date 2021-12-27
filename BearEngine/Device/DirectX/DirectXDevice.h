#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <d3dx12.h>

#include "../Singleton.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;

class DirectXDevice
	:public Singleton<DirectXDevice>
{
public:
	friend class Singleton<DirectXDevice>;
	HRESULT InitDirectX();
	ID3D12Device5* GetDevice();
	IDXGIFactory6* GetDxgiFactory();
	void GetVideoMemoryInfo(DXGI_QUERY_VIDEO_MEMORY_INFO* info);
	ComPtr<ID3D12Resource1> CreateResource(const CD3DX12_RESOURCE_DESC& desc,
		D3D12_RESOURCE_STATES resourceStates, const D3D12_CLEAR_VALUE* clearValue, D3D12_HEAP_TYPE heapType);
	void EnableDebugLayer();
	void DeviceRemovedHandler();
	bool CheckSupportedDXR();
	ComPtr<ID3D12RootSignature> CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible);
	void RenderDebug();

protected:
	DirectXDevice() = default;
	virtual ~DirectXDevice() = default;

private:
	HRESULT CreateDevice();
	HRESULT CreateDxgiFactory();
	void FindAdapter();

private:
	ComPtr<ID3D12Device5> device_;
	ComPtr<IDXGIFactory6> dxgi_factory_;
	ComPtr<IDXGIAdapter4> adapter_;


};

