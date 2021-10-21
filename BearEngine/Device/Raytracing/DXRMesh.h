#pragma once

#include <d3d12.h>
#include <memory>
#include <wrl/client.h>

#include "Device/DirectX/Core/Buffer.h"
#include "Device/DirectX/Core/Model/MeshDatas.h"

using namespace Microsoft::WRL;


class DXRMesh
{
public:
	DXRMesh(const std::wstring& hitGroupName,int hitGroupIndex,std::shared_ptr<MeshData> meshData);
	~DXRMesh() = default;

	void SetMatrix(const SimpleMath::Matrix mat);

	void CreateRaytracingInstanceDesc();

	std::shared_ptr<Buffer> _vertexBuffer;
	std::shared_ptr<Buffer> _indexBuffer;

	D3D12_GPU_DESCRIPTOR_HANDLE m_vbView;
	D3D12_GPU_DESCRIPTOR_HANDLE m_ibView;

	ComPtr<ID3D12Resource> _blas;

	UINT _vertexCount = 0;
	UINT _indexCount = 0;
	UINT _vertexStride = 0;

	std::wstring _hitGroupName;
	SimpleMath::Matrix _matrix;
	D3D12_RAYTRACING_INSTANCE_DESC _raytracingInstanceDesc;
	int _hitGroupIndex;
};
