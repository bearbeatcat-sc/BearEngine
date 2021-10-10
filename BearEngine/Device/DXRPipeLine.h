#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <SimpleMath.h>

#include "Singleton.h"

using namespace Microsoft::WRL;

struct AccelerationStructureBuffers
{
	ComPtr<ID3D12Resource> pScratch;
	ComPtr<ID3D12Resource> pResult;
	ComPtr<ID3D12Resource> pInstanceDesc;
};

class DXRPipeLine
	:public Singleton<DXRPipeLine>

{
public:
	friend class Singleton<DXRPipeLine>;

	DXRPipeLine() = default;
	~DXRPipeLine() = default;
	bool InitPipeLine();

private:
	ID3D12Resource* CreateTriangleVB();

	// ジオメトリレベルで加速構造を作成する
	AccelerationStructureBuffers CreateButtomLevelAS(ID3D12Resource* pVB);
	ID3D12Resource* CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState,
	                             const D3D12_HEAP_PROPERTIES& heapProps);
	AccelerationStructureBuffers CreateTopLevelAS(ID3D12Resource* pBottomLevelAS,uint64_t& tlasSize);
	void CreateAccelerationStructures();
};
