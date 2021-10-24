#pragma once

#include <d3d12.h>
#include <string>
#include <wrl/client.h>

struct DXRMeshData
{
public:
	DXRMeshData(const std::wstring& hitGroupName)
		:_hitGroupName(hitGroupName)
	{};
	
	~DXRMeshData() = default;
	
	D3D12_GPU_DESCRIPTOR_HANDLE m_vbView;
	D3D12_GPU_DESCRIPTOR_HANDLE m_ibView;

	// DXR用
	Microsoft::WRL::ComPtr<ID3D12Resource> _blas;
	std::wstring _hitGroupName;
};
