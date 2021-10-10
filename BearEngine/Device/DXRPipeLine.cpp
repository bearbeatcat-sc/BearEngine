#include "DXRPipeLine.h"

#include <SimpleMath.h>

#include "WindowApp.h"
#include "DirectX/DirectXDevice.h"
#include "DirectX/DirectXGraphics.h"

bool DXRPipeLine::InitPipeLine()
{
	if(!DirectXDevice::GetInstance().CheckSupportedDXR())
	{
		WindowApp::GetInstance().MsgBox("Not Supported DXR.","ERROR");
		return false;
	}
}

ID3D12Resource* DXRPipeLine::CreateTriangleVB()
{
	return nullptr;
}

AccelerationStructureBuffers DXRPipeLine::CreateButtomLevelAS(ID3D12Resource* pVB)
{
	// ジオメトリ情報を定義？
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Triangles.VertexBuffer.StartAddress = pVB->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(DirectX::SimpleMath::Vector3);
	geomDesc.Triangles.VertexCount = 3;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// 加速構造に入力する情報？
	// 加速構造を作成するための情報かも
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	D3D12_HEAP_PROPERTIES defaultHeapProps = {
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};
	
	// ASBufferの作成
	AccelerationStructureBuffers buffers;
	buffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	buffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, defaultHeapProps);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();

	DirectXGraphics::GetInstance().GetCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = buffers.pResult.Get();

	DirectXGraphics::GetInstance().GetCommandList()->ResourceBarrier(1, &uavBarrier);

	return buffers;
}

ID3D12Resource* DXRPipeLine::CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
{
	D3D12_RESOURCE_DESC bufDesc = {};
	bufDesc.Alignment = 0;
	bufDesc.DepthOrArraySize = 1;
	bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufDesc.Flags = flags;
	bufDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufDesc.Height = 1;
	bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufDesc.MipLevels = 1;
	bufDesc.SampleDesc.Count = 1;
	bufDesc.SampleDesc.Quality = 0;
	bufDesc.Width = size;

	ID3D12Resource* pBuffer;

	
	if(DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer)) != S_OK)
	{
		WindowApp::GetInstance().MsgBox("バッファの生成に失敗", "ERROR");
		assert(0);
	}

	return pBuffer;
}

AccelerationStructureBuffers DXRPipeLine::CreateTopLevelAS(ID3D12Resource* pBottomLevelAS, uint64_t& tlasSize)
{
	return AccelerationStructureBuffers{};
}

void DXRPipeLine::CreateAccelerationStructures()
{
}
