#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <SimpleMath.h>

#include "Singleton.h"

using namespace Microsoft::WRL;

struct AccelerationStructureBuffers
{
	// 生成に使う一時的なリソース
	ComPtr<ID3D12Resource> pScratch;
	ComPtr<ID3D12Resource> pResult;
	ComPtr<ID3D12Resource> pInstanceDesc;
};

struct RootSignatureDesc
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	std::vector<D3D12_DESCRIPTOR_RANGE> range;
	std::vector<D3D12_ROOT_PARAMETER> rootParams;
};

struct DXilLibrary
{
	// 複数のシェーダーが含まれている？
	DXilLibrary(IDxcBlob* pBlob,const std::vector<std::wstring>& entryPoint, uint32_t entryPointCount)
		:pShaderBlob(pBlob)
	{
		stateSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		stateSubobject.pDesc = &dxilLibDesc;


		dxilLibDesc = {};
		exportDesc.resize(entryPointCount);
		exportName.resize(entryPointCount);

		if (pBlob)
		{
			dxilLibDesc.DXILLibrary.pShaderBytecode = pBlob->GetBufferPointer();
			dxilLibDesc.DXILLibrary.BytecodeLength = pBlob->GetBufferSize();
			dxilLibDesc.NumExports = entryPointCount;
			dxilLibDesc.pExports = exportDesc.data();

			for (uint32_t i = 0; i < entryPointCount; i++)
			{
				exportName[i] = entryPoint[i];
				exportDesc[i].Name = exportName[i].c_str();
				exportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
				exportDesc[i].ExportToRename = nullptr;
			}
		}

	}


	D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
	D3D12_STATE_SUBOBJECT stateSubobject{};
	IDxcBlob* pShaderBlob;
	std::vector<D3D12_EXPORT_DESC> exportDesc;
	std::vector<std::wstring> exportName;

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
	ID3D12Resource* CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState,const D3D12_HEAP_PROPERTIES& heapProps);
	AccelerationStructureBuffers CreateTopLevelAS(ID3D12Resource* pBottomLevelAS,uint64_t& tlasSize);
	
	void CreateAccelerationStructures();
	RootSignatureDesc CreateRayGenRtooDesc();
	DXilLibrary CreateDxliLibrary(const std::wstring& shaderPath);

	ID3D12Resource* _vertex_buffer;
	AccelerationStructureBuffers _toplevel_as;
	AccelerationStructureBuffers _bottomlevel_as;
	uint64_t _tlasSize;
};
