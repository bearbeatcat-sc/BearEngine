#include "DXRMesh.h"

DXRMesh::DXRMesh(const std::wstring& hitGroupName, int hitGroupIndex, std::shared_ptr<MeshData> meshData) :
	_hitGroupName(hitGroupName), _hitGroupIndex(hitGroupIndex)
{
	_vertexStride = (UINT)(sizeof(MeshData::Vertex));
	m_vbView = meshData->m_vb_h_gpu_descriptor_handle;
	m_ibView = meshData->m_ib_h_gpu_descriptor_handle;
	_vertexCount = meshData->m_VertexCount;
	_indexCount = meshData->m_indexCount;
	_vertexBuffer = meshData->GetVertexBuffer();
	_indexBuffer = meshData->GetIndexBuffer();
}

void DXRMesh::SetMatrix(const SimpleMath::Matrix mat)
{
	_matrix = mat;
}

void DXRMesh::CreateRaytracingInstanceDesc()
{
	_raytracingInstanceDesc.InstanceMask = 0xFF;
	_raytracingInstanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

	XMStoreFloat3x4(
		reinterpret_cast<XMFLOAT3X4*>(&_raytracingInstanceDesc.Transform),
		_matrix
	);

	_raytracingInstanceDesc.InstanceContributionToHitGroupIndex = _hitGroupIndex;
	_raytracingInstanceDesc.AccelerationStructure = _blas->GetGPUVirtualAddress();
}