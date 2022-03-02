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
	DXRInstance(const int hitGroupIndex,std::shared_ptr<DXRMeshData> meshData,const int instanceID = 0);
	~DXRInstance() = default;

	void SetMatrix(const SimpleMath::Matrix mat);
	void CreateRaytracingInstanceDesc(UINT instanceMask = 0xFF);
	void Destroy();

	SimpleMath::Matrix _matrix;
	std::shared_ptr<D3D12_RAYTRACING_INSTANCE_DESC> _raytracingInstanceDesc;
	int _hitGroupIndex = -1;
	int _instnaceID = -1;

	std::shared_ptr<DXRMeshData> _pDXRMeshData;
	bool _DestroyFlag;
};
