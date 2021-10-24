#pragma once

#include <d3d12.h>
#include <memory>
#include <wrl/client.h>

#include "Device/DirectX/Core/Buffer.h"
#include "Device/DirectX/Core/Model/MeshDatas.h"

using namespace Microsoft::WRL;

class DXRMeshData;


class DXRInstance
{
public:
	DXRInstance(const int hitGroupIndex,std::shared_ptr<DXRMeshData> meshData);
	~DXRInstance() = default;

	void SetMatrix(const SimpleMath::Matrix mat);
	void CreateRaytracingInstanceDesc();

	SimpleMath::Matrix _matrix;
	D3D12_RAYTRACING_INSTANCE_DESC _raytracingInstanceDesc;
	int _hitGroupIndex;

	std::shared_ptr<DXRMeshData> _pDXRMeshData;
};
