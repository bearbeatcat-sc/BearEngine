#include "DXRInstance.h"

#include "DXRMeshData.h"

DXRInstance::DXRInstance(const int hitGroupIndex, std::shared_ptr<DXRMeshData> meshData)
	:_hitGroupIndex(hitGroupIndex), _pDXRMeshData(meshData)
{

}

void DXRInstance::SetMatrix(const SimpleMath::Matrix mat)
{
	_matrix = mat;
}

void DXRInstance::CreateRaytracingInstanceDesc()
{
	_raytracingInstanceDesc.InstanceMask = 0xFF;
	_raytracingInstanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&_raytracingInstanceDesc.Transform),
		_matrix
	);

	_raytracingInstanceDesc.InstanceContributionToHitGroupIndex = _hitGroupIndex;
	_raytracingInstanceDesc.AccelerationStructure = _pDXRMeshData->_blas->GetGPUVirtualAddress();
}