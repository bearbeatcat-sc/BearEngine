#pragma once

#include <d3d12.h>
#include <string>
#include <wrl/client.h>
#include "DXRPipeLine.h"

struct DXRMeshData
{
public:
	DXRMeshData(const std::wstring& hitGroupName,const PhysicsBaseMaterial& mat)
		:_hitGroupName(hitGroupName),_Mat(mat)
	{
		
	};
	
	~DXRMeshData() = default;

	void UpdateMaterial(const PhysicsBaseMaterial& mat)
	{
		_Mat = mat;
		DXRPipeLine::GetInstance().OnUpdateMaterial();
	}
	
	D3D12_GPU_DESCRIPTOR_HANDLE m_vbView;
	D3D12_GPU_DESCRIPTOR_HANDLE m_ibView;

	// DXR用
	Microsoft::WRL::ComPtr<ID3D12Resource> _blas;
	std::shared_ptr<AccelerationStructureBuffers> _Buffer;
	std::wstring _hitGroupName;
	int hitGropIndex;
	std::string meshName;
	PhysicsBaseMaterial _Mat;
};
