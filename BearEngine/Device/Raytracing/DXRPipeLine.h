#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <map>
#include <wrl/client.h>
#include <SimpleMath.h>

#include "DXRInstance.h"
#include "../Singleton.h"
#include "../DirectX/DirectXDevice.h"
#include "Device/WindowApp.h"
#include "Device/DirectX/Core/Model/MeshDatas.h"

using namespace Microsoft::WRL;

class Mesh;
class DXRMeshData;
class SkyBox;

// アライメント用
#define align_to(_alignment, _val) (((_val + _alignment - 1) / _alignment) * _alignment)



struct AccelerationStructureBuffers
{
	AccelerationStructureBuffers() {};
	
	// 生成に使う一時的なリソース
	ComPtr<ID3D12Resource> pScratch;
	ComPtr<ID3D12Resource> pResult;
	ComPtr<ID3D12Resource> pUpdate;
	ComPtr<ID3D12Resource> pInstanceDesc;
};

struct RootSignatureDesc
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	std::vector<D3D12_DESCRIPTOR_RANGE> range;
	std::vector<D3D12_ROOT_PARAMETER> rootParams;
};

// ヒットグループを内包している
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
	ShaderConfig(uint32_t maxAttributeSizeInBytes, uint32_t maxPayloadSizeInBytes)
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
		// 再帰呼び出しの回数
		config.MaxTraceRecursionDepth = maxTraceRecursionDepth;

		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		subobject.pDesc = &config;
	}

	D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
	D3D12_STATE_SUBOBJECT subobject = {};
};

struct ExportAssocication
{
	ExportAssocication(const WCHAR* exportNames[], uint32_t exportCount, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
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

	DXRPipeLine();
	~DXRPipeLine() = default;

	bool InitPipeLine();
	bool Init();

	std::shared_ptr<DXRInstance> AddInstance(const std::string& meshDataName, const int hitGroupIndex);
	void AddMeshData(std::shared_ptr<MeshData> pMeshData, const std::wstring& hitGroupName, const std::string& meshDataName);
	void AddMeshData(std::shared_ptr<MeshData> pMeshData, const std::wstring& hitGroupName,
	                 const std::string& meshDataName,
	                 MeshData::RaytraceMaterial material);

	void Render(ID3D12Resource* pRenderResource,SkyBox* pSkyBox);
	void CreateResourceView(std::shared_ptr<MeshData> mesh);
	void DeleteInstance();

private:
	ComPtr<ID3D12Resource> CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps);
	void CreateTopLevelAS();
	void CreateBLAS(std::shared_ptr<DXRMeshData> pDXRMeshData, std::shared_ptr<MeshData> pMeshData);
	void UpdateTLAS();

	void DrawDebugGUI();
	
	void CreateAccelerationStructures();
	void CreateLocalRootSignature();
	ComPtr<ID3D12RootSignature> CreateRayGenRootDesc();
	ComPtr<ID3D12RootSignature> CreateClosestHitRootDesc();
	DXilLibrary CreateDxliLibrary(const wchar_t* shaderPath);
	void CreatePipeleineState(const wchar_t* shaderPath);
	void CreateShaderTable();
	void CreateShaderResource();
	void CreateGlobalRootSignature();
	void CreateSceneCB();
	void CreateMaterialCB();
	void CreateDescriptorHeaps();
	
	UINT WriteShaderIdentifer(void* dst, const void* shaderId);
	UINT WriteGPUDescriptor(void* dst, const D3D12_GPU_DESCRIPTOR_HANDLE handle);
	UINT WriteGPUResourceAddr(void* dst, D3D12_GPU_VIRTUAL_ADDRESS addr);
	uint8_t* WriteMeshShaderRecord(uint8_t* dst, const std::shared_ptr<DXRMeshData> mesh, D3D12_GPU_VIRTUAL_ADDRESS address, UINT recordSize);
	
	D3D12_RAYTRACING_GEOMETRY_DESC GetGeomtryDesc(std::shared_ptr<MeshData> meshData);

	void SceneCBUpdate();

	WindowSize _WindowSize;

	ComPtr<ID3D12Resource> _shaderTable;
	uint32_t _shaderTableEntrySize = 0;

	ComPtr<ID3D12Resource> _vertex_buffer;
	AccelerationStructureBuffers _AccelerationStructureBuffers;
	ComPtr<ID3D12Resource> _TopLevelASResource;
	std::vector<ComPtr<ID3D12Resource>> _instanceDescBuffers;;

	uint64_t _tlasSize;

	ComPtr<ID3D12StateObject> _PipelineState;
	ComPtr<ID3D12RootSignature> _EmptyRootSig;

	ComPtr<ID3D12Resource> _OutPutResource;
	ComPtr<ID3D12DescriptorHeap> _SrvUavHeap;
	ComPtr<ID3D12DescriptorHeap> _MeshSrvHeap;
	const uint32_t kSrvUavHeapSize = 2;

	struct SceneParam
	{
		SimpleMath::Matrix mtxView;       // ビュー行列.
		SimpleMath::Matrix mtxProj;       // プロジェクション行列.
		SimpleMath::Matrix mtxViewInv;    // ビュー逆行列.
		SimpleMath::Matrix mtxProjInv;    // プロジェクション逆行列.
		SimpleMath::Vector4 lightDirection; // 平行光源の向き.
		SimpleMath::Vector4 lightColor;    // 平行光源色.
		SimpleMath::Vector4 ambientColor;  // 環境光.
		SimpleMath::Vector3 eyePos;
	};
	SceneParam m_sceneParam;
	ComPtr<ID3D12Resource> _SceneCB;

	std::vector<std::shared_ptr<DXRMeshData>> _meshDatas;
	
	std::vector<std::shared_ptr<DXRInstance>> _instances;;

	D3D12_DISPATCH_RAYS_DESC _dispathRaysDesc;

	const D3D12_HEAP_PROPERTIES defaultHeapProps = {
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};

	const D3D12_HEAP_PROPERTIES uploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};



	ComPtr<ID3D12RootSignature> _globalRootSignature;
	ComPtr<ID3D12RootSignature> _closesHitLocalRootSignature;
	ComPtr<ID3D12RootSignature> _rayGenerationLocalRootSignature;

	ComPtr<ID3D12Resource> _materialCB;

	const UINT _SRVResourceCount = 2;
	const UINT _MeshDataSize = 3;
	const UINT _MaxMeshCount = 1024;
	UINT _AllocateCount = 0;
	UINT _IncSize;

	const UINT _MaxInstanceCount = 300;
};
