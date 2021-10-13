#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>
#include <SimpleMath.h>

#include "Singleton.h"
#include "DirectX/DirectXDevice.h"
#include "WindowApp.h"

using namespace Microsoft::WRL;

// アライメント用
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)

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

struct HitProgram
{
	HitProgram(const wchar_t* ahsExport, const  wchar_t* chsExport, const  wchar_t* name)
		:exportName(name)
	{
		desc = {};
		desc.AnyHitShaderImport = ahsExport;
		desc.ClosestHitShaderImport = chsExport;
		desc.HitGroupExport = exportName.c_str();

		// この2つはセット？
		subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		subObject.pDesc = &desc;
	}

	std::wstring exportName;
	D3D12_HIT_GROUP_DESC desc;
	D3D12_STATE_SUBOBJECT subObject;
};

struct LocalRootSignature
{

	LocalRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc)
	{
		pRootSig = DirectXDevice::GetInstance().CreateRootSignature(desc);
		pInterface = pRootSig.Get();
		subObject.pDesc = &pInterface;
		subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
	}

	ComPtr<ID3D12RootSignature> pRootSig;
	ID3D12RootSignature* pInterface = nullptr;
	D3D12_STATE_SUBOBJECT subObject = {};
};

struct GlobalRootSignature
{
	GlobalRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc)
	{
		pRootSig = DirectXDevice::GetInstance().CreateRootSignature(desc);
		pInterface = pRootSig.Get();
		subObject.pDesc = &pInterface;
		subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	}

	ComPtr<ID3D12RootSignature> pRootSig;
	ID3D12RootSignature* pInterface = nullptr;
	D3D12_STATE_SUBOBJECT subObject = {};
};

struct ShaderConfig
{
	ShaderConfig(uint32_t maxAttributeSizeInBytes,uint32_t maxPayloadSizeInBytes)
	{
		shaderConfig.MaxAttributeSizeInBytes = maxAttributeSizeInBytes;
		shaderConfig.MaxPayloadSizeInBytes = maxPayloadSizeInBytes;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		subobject.pDesc = &shaderConfig;
	}
	
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
	D3D12_STATE_SUBOBJECT subobject = {};
};

struct PipeLineConfig
{
	PipeLineConfig(uint32_t maxTraceRecursionDepth)
	{
		config.MaxTraceRecursionDepth = maxTraceRecursionDepth;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		subobject.pDesc = &config;
	}

	D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
	D3D12_STATE_SUBOBJECT subobject = {};
};

struct ExportAssocication
{
	ExportAssocication(const WCHAR* exportNames[] ,uint32_t exportCount,const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
	{
		association.NumExports = exportCount;
		association.pExports = exportNames;
		association.pSubobjectToAssociate = pSubobjectToAssociate;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
		subobject.pDesc = &association;
	}
	
	D3D12_STATE_SUBOBJECT subobject = {};
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association = {};
};

struct DXilLibrary
{
	// 複数のシェーダーが含まれている？
	DXilLibrary(ComPtr<IDxcBlob> pBlob, const std::vector<std::wstring>& entryPoint, uint32_t entryPointCount)
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
	ComPtr<IDxcBlob> pShaderBlob;
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
	void Render(ID3D12Resource* pRenderResource);

private:
	ComPtr<ID3D12Resource> CreateTriangleVB();

	// ジオメトリレベルで加速構造を作成する
	AccelerationStructureBuffers CreateButtomLevelAS(ID3D12Resource* pVB);
	ComPtr<ID3D12Resource> CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps);
	AccelerationStructureBuffers CreateTopLevelAS(ID3D12Resource* pBottomLevelAS, uint64_t& tlasSize);

	void CreateAccelerationStructures();
	RootSignatureDesc CreateRayGenRtooDesc();
	DXilLibrary CreateDxliLibrary(const wchar_t* shaderPath);
	void CreatePipeleineState(const wchar_t* shaderPath);
	void CreateShaderTable();
	void CreateShaderResource();

	WindowSize _WindowSize;

	ComPtr<ID3D12Resource> _shaderTable;
	uint32_t _shaderTableEntrySize = 0;

	ComPtr<ID3D12Resource> _vertex_buffer;
	ComPtr<ID3D12Resource> _TopLevelASResource;
	ComPtr<ID3D12Resource> _BottomLevelASResource;
	
	uint64_t _tlasSize;

	ComPtr<ID3D12StateObject> _PipelineState;
	ComPtr<ID3D12RootSignature> _EmptyRootSig;

	ComPtr<ID3D12Resource> _OutPutResource;
	ComPtr<ID3D12DescriptorHeap> _SrvUavHeap;
	const uint32_t kSrvUavHeapSize = 2;
};
