#include "DXRPipeLine.h"

#include <SimpleMath.h>
#include <dxcapi.h>
#include <fstream>
#include <sstream>

#include "DXRInstance.h"
#include "DXRMeshData.h"
#include "../WindowApp.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/DirectXGraphics.h"
#include "Device/SkyBox.h"
#include "Device/DirectX/Core/Model/Mesh.h"
#include "Device/DirectX/Core/Model/MeshDatas.h"
#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Device/Lights/DirectionalLight.h"
#include "Device/Lights/LightManager.h"
#include "Utility/Camera.h"
#include "Utility/CameraManager.h"
#include "imgui/imgui.h"

#pragma comment(lib, "dxcompiler")

// TODO:あとで別のクラスに分離する
// ヘルパー関数的なやつ
ComPtr<IDxcBlob> CompileLibrary(const WCHAR* filename, const WCHAR* targetString)
{
	static ComPtr<IDxcCompiler> p_dxc_compiler = nullptr;
	static ComPtr<IDxcLibrary> p_dxc_library = nullptr;
	static ComPtr<IDxcIncludeHandler> p_dxc_include_handler = nullptr;

	HRESULT hr;

	DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)&p_dxc_compiler);
	DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)&p_dxc_library);
	p_dxc_library->CreateIncludeHandler(&p_dxc_include_handler);


	// ファイルを読み込む
	std::ifstream shaderFile(filename);

	if (shaderFile.good() == false)
	{
		throw std::logic_error("シェーダーファイルの読み込みに失敗");
	}

	std::stringstream strStream;
	strStream << shaderFile.rdbuf();
	std::string sShader = strStream.str();

	// Blobの作成
	ComPtr<IDxcBlobEncoding> p_blob_encoding;
	p_dxc_library->CreateBlobWithEncodingFromPinned(
		(LPBYTE)sShader.c_str(), (uint32_t)sShader.size(), 0, &p_blob_encoding);

	// コンパイル
	ComPtr<IDxcOperationResult> p_result;
	p_dxc_compiler->Compile(p_blob_encoding.Get(), filename, L"", L"lib_6_3",
		nullptr, 0, nullptr, 0, p_dxc_include_handler.Get(), &p_result);


	HRESULT resultcode;
	p_result->GetStatus(&resultcode);

	// エラーの解析
	if (FAILED(resultcode))
	{
		ComPtr<IDxcBlobEncoding> p_error;
		hr = p_result->GetErrorBuffer(&p_error);

		if (FAILED(hr))
		{
			throw std::logic_error("コンパイルエラーの取得することができませんでした。");
		}


		std::vector<char> infoLog(p_error->GetBufferSize() + 1);
		memcpy(infoLog.data(), p_error->GetBufferPointer(), p_error->GetBufferSize());
		infoLog[p_error->GetBufferSize()] = 0;

		std::string errorMsg = "シェーダーコンパイルエラー:\n";
		errorMsg.append(infoLog.data());

		MessageBoxA(nullptr, errorMsg.c_str(), "ERROR", MB_OK);
		throw std::logic_error("シェーダーのコンパイルに失敗");
	}

	ComPtr<IDxcBlob> p_blob;
	HRESULT result = p_result->GetResult(p_blob.GetAddressOf());

	const void* check = p_blob->GetBufferPointer();

	return p_blob;

}

DXRPipeLine::DXRPipeLine()
{
	_meshDatas.clear();
	_instances.clear();

	_AllocateCount = 2;
}

bool DXRPipeLine::InitPipeLine()
{
	if (!DirectXDevice::GetInstance().CheckSupportedDXR())
	{
		WindowApp::GetInstance().MsgBox("Not Supported DXR.", "ERROR");
		return false;
	}

	CreateAccelerationStructures();
	CreateGlobalRootSignature();
	CreateLocalRootSignature();
	CreatePipeleineState(L"Resources\\Shaders\\TestShader.hlsl");
	CreateShaderResource();
	CreateSceneCB();
	CreateMaterialCB();
	CreateShaderTable();

	return true;
}

bool DXRPipeLine::Init()
{
	CreateDescriptorHeaps();

	return true;
}

std::shared_ptr<DXRInstance> DXRPipeLine::AddInstance(const std::string& meshDataName, const int hitGroupIndex)
{
	std::shared_ptr<DXRMeshData> findMesh;
	
	for(auto mesh : _meshDatas)
	{
		if(mesh->meshName == meshDataName)
		{
			findMesh = mesh;
		}
	}

	if(findMesh == nullptr)
	{
		throw std::runtime_error("Not Regist MeshData");
	}

	if (_instances.size() >= _MaxInstanceCount)
	{
		throw std::runtime_error("exceeded the number can register.");
	}

	auto instance = std::make_shared<DXRInstance>(hitGroupIndex, findMesh);
	instance->_hitGroupIndex = findMesh->hitGropIndex;

	_instances.push_back(instance);

	// 後でTLASの構築するかも

	return instance;
}

void DXRPipeLine::AddMeshData(std::shared_ptr<MeshData> pMeshData, const std::wstring& hitGroupName, const std::string& meshDataName)
{
	if(pMeshData == nullptr)
	{
		throw std::runtime_error("MeshDatas is null.");
	}

	CreateResourceView(pMeshData);
	auto dxrMesh = std::make_shared<DXRMeshData>(hitGroupName, pMeshData->GetPhysicsBaseMaterial());

	dxrMesh->m_ibView = pMeshData->m_ib_h_gpu_descriptor_handle;
	dxrMesh->m_vbView = pMeshData->m_vb_h_gpu_descriptor_handle;

	// 現状は1メッシュデータにつき、一つのヒットグループを扱っているので
	dxrMesh->hitGropIndex = _meshDatas.size();
	dxrMesh->meshName = meshDataName;

	CreateBLAS(dxrMesh, pMeshData);

	_meshDatas.push_back(dxrMesh);
}

void DXRPipeLine::AddMeshData(std::shared_ptr<MeshData> pMeshData, const std::wstring& hitGroupName, const std::string& meshDataName,const PhysicsBaseMaterial material)
{
	if (pMeshData == nullptr)
	{
		throw std::runtime_error("MeshDatas is null.");
	}

	CreateResourceView(pMeshData);
	auto dxrMesh = std::make_shared<DXRMeshData>(hitGroupName, material);

	dxrMesh->m_ibView = pMeshData->m_ib_h_gpu_descriptor_handle;
	dxrMesh->m_vbView = pMeshData->m_vb_h_gpu_descriptor_handle;
	//dxrMesh->m_matView = pMeshData->m_mat_h_gpu_descriptor_handle;

	// 現状は1メッシュデータにつき、一つのヒットグループを扱っているので
	dxrMesh->hitGropIndex = _meshDatas.size();
	dxrMesh->meshName = meshDataName;

	CreateBLAS(dxrMesh, pMeshData);

	_meshDatas.push_back(dxrMesh);
}


void DXRPipeLine::CreateBLAS(std::shared_ptr<DXRMeshData> pDXRMeshData, std::shared_ptr<MeshData> pMeshData)
{

	auto geomtryDesc = GetGeomtryDesc(pMeshData);

	// 加速構造に入力する情報？
	// 加速構造を作成するための情報かも
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomtryDesc;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;


	// 必要なメモリ量を求める
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
	std::shared_ptr<AccelerationStructureBuffers> buffers = std::make_shared<AccelerationStructureBuffers>();
	buffers->pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	buffers->pScratch->SetName(L"BLAS_ScratchBuffer");
	
	buffers->pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, defaultHeapProps);
	buffers->pResult->SetName(L"BLAS_ResultBuffer");

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.DestAccelerationStructureData = buffers->pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = buffers->pScratch->GetGPUVirtualAddress();

	DirectXGraphics::GetInstance().GetCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = buffers->pResult.Get();

	DirectXGraphics::GetInstance().GetCommandList()->ResourceBarrier(1, &uavBarrier);

	pDXRMeshData->_Buffer = buffers;
	pDXRMeshData->_blas = buffers->pResult;
}

void DXRPipeLine::UpdateTLAS()
{
	// 死亡処理を行う
	DeleteInstance();

	auto instanceCount = _instances.size();
	auto size = _MaxInstanceCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);

	const int frameIndex = DirectXGraphics::GetInstance().GetBackBufferIndex();



	D3D12_RAYTRACING_INSTANCE_DESC* p_instance_desc;
	_instanceDescBuffers[frameIndex]->Map(0, nullptr, (void**)&p_instance_desc);
	ZeroMemory(p_instance_desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * _MaxInstanceCount);

	for (int i = 0; i < instanceCount; i++)
	{
		p_instance_desc[i].InstanceID = i;
		p_instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		p_instance_desc[i].InstanceMask = 0xFF;
		p_instance_desc[i].InstanceContributionToHitGroupIndex = _instances[i]->_raytracingInstanceDesc->InstanceContributionToHitGroupIndex;
		p_instance_desc[i].AccelerationStructure = _instances[i]->_raytracingInstanceDesc->AccelerationStructure;

		SimpleMath::Matrix mat = _instances[i]->_matrix;
		//memcpy(p_instance_desc[i].Transform, &mat, sizeof(_instances[i]->_raytracingInstanceDesc->Transform));

		XMStoreFloat3x4(
			reinterpret_cast<XMFLOAT3X4*>(p_instance_desc[i].Transform),
			_instances[i]->_matrix
		);

		//p_instance_desc[i].InstanceContributionToHitGroupIndex = static_cast<UINT>(0);
		//p_instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;



		//p_instance_desc->AccelerationStructure = _BottomLevelASResources[i].pResult->GetGPUVirtualAddress();
		//p_instance_desc->InstanceMask = 0xFF;
	}

	_instanceDescBuffers[frameIndex]->Unmap(0, nullptr);

	// TLASの作成
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};

	asDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	asDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	asDesc.Inputs.NumDescs = _MaxInstanceCount;
	asDesc.Inputs.InstanceDescs = _instanceDescBuffers[frameIndex]->GetGPUVirtualAddress();

	// TLAS の更新処理を行うためのフラグを設定する.
	// 今回は、インスタンスの追加もするので、新しく構築する。
	asDesc.Inputs.Flags =
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

	//asDesc.SourceAccelerationStructureData = _AccelerationStructureBuffers.pResult->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = _AccelerationStructureBuffers.pResult->GetGPUVirtualAddress();

	asDesc.ScratchAccelerationStructureData = _AccelerationStructureBuffers.pUpdate->GetGPUVirtualAddress();


	DirectXGraphics::GetInstance().GetCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = _AccelerationStructureBuffers.pResult.Get();

	DirectXGraphics::GetInstance().GetCommandList()->ResourceBarrier(1, &uavBarrier);


}

void DXRPipeLine::DrawDebugGUI()
{
	if(ImGui::BeginTabItem("DXRPipeline"))
	{
		ImGui::Text("InstanceCount:%i", _instances.size());
		ImGui::EndTabItem();
	}
}

void DXRPipeLine::Render(ID3D12Resource* pRenderResource, SkyBox* pSkyBox)
{
	auto commandList = DirectXGraphics::GetInstance().GetCommandList();
	UpdateTLAS();

	// カメラ用のCBの更新
	// 後でパラメータの変更したときだけ変えるようにする。
	SceneCBUpdate();
	void* dst = nullptr;
	_SceneCB->Map(0, nullptr, &dst);
	memcpy(dst, &m_sceneParam, sizeof(SceneParam));
	_SceneCB->Unmap(0, nullptr);

	commandList->SetPipelineState1(_PipelineState.Get());
	commandList->SetComputeRootSignature(_globalRootSignature.Get());

	auto skyBoxDescHeap = pSkyBox->GETDescHeap();

	// SkyBoxのハンドルをセット
	ID3D12DescriptorHeap* skyBoxHeaps[] = { skyBoxDescHeap };
	commandList->SetDescriptorHeaps(ARRAYSIZE(skyBoxHeaps), skyBoxHeaps);
	auto skyboxHandle = skyBoxDescHeap->GetGPUDescriptorHandleForHeapStart();
	skyboxHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	commandList->SetComputeRootDescriptorTable(2, skyboxHandle);



	ID3D12DescriptorHeap* heaps[] = { _SrvUavHeap.Get() };
	commandList->SetDescriptorHeaps(ARRAYSIZE(heaps), heaps);

	// 加速構造のセット
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = _SrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	commandList->SetComputeRootDescriptorTable(0, srvHandle);

	// カメラCBのセット
	commandList->SetComputeRootConstantBufferView(1, _SceneCB->GetGPUVirtualAddress());
	commandList->DispatchRays(&_dispathRaysDesc);

	DirectXGraphics::GetInstance().ResourceBarrier(
		_OutPutResource.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);

	DirectXGraphics::GetInstance().ResourceBarrier(
		_OutPutResource.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE

	);

	DirectXGraphics::GetInstance().ResourceBarrier(
		pRenderResource,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COPY_DEST
	);

	DirectXGraphics::GetInstance().GetCommandList()->CopyResource(
		pRenderResource, _OutPutResource.Get()
	);

}


ComPtr<ID3D12Resource> DXRPipeLine::CreateBuffer(uint64_t size, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_HEAP_PROPERTIES& heapProps)
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

	ComPtr<ID3D12Resource> resource;

	if (DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())) != S_OK)
	{
		WindowApp::GetInstance().MsgBox("バッファの生成に失敗", "ERROR");
		assert(0);
	}

	return resource;
}

void DXRPipeLine::CreateTopLevelAS()
{
	uint64_t tlasSize;

	const int instanceCount = _instances.size();
	
	_instanceDescBuffers.resize(2);

	auto size = UINT(_MaxInstanceCount * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));

	for (int i = 0; i < 2; i++)
	{
		_instanceDescBuffers[i] = CreateBuffer(size, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeapProps);
		_instanceDescBuffers[i]->SetName(L"RayTracing_InstanceBuffer");
	}

	const int currentBuffer = DirectXGraphics::GetInstance().GetBackBufferIndex();


	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = _MaxInstanceCount;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);



	_AccelerationStructureBuffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	_AccelerationStructureBuffers.pScratch->SetName(L"TLAS_ScratchBuffer");

	_AccelerationStructureBuffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, defaultHeapProps);
	_AccelerationStructureBuffers.pResult->SetName(L"TLAS_ResultBuffer");


	_AccelerationStructureBuffers.pUpdate = CreateBuffer(info.UpdateScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	_AccelerationStructureBuffers.pUpdate->SetName(L"TLAS_UpdateBuffer");

	
	tlasSize = info.ResultDataMaxSizeInBytes;

	_AccelerationStructureBuffers.pInstanceDesc = _instanceDescBuffers[currentBuffer];

	D3D12_RAYTRACING_INSTANCE_DESC* p_instance_desc;
	_AccelerationStructureBuffers.pInstanceDesc->Map(0, nullptr, (void**)&p_instance_desc);
	ZeroMemory(p_instance_desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * _MaxInstanceCount);

	for (int i = 0; i < instanceCount; i++)
	{
		p_instance_desc[i].InstanceID = i;
		p_instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		p_instance_desc[i].InstanceMask = 0xFF;
		p_instance_desc[i].InstanceContributionToHitGroupIndex = _instances[i]->_hitGroupIndex;
		p_instance_desc[i].AccelerationStructure = _instances[i]->_raytracingInstanceDesc->AccelerationStructure;

		XMStoreFloat3x4(
			reinterpret_cast<XMFLOAT3X4*>(p_instance_desc[i].Transform),
			_instances[i]->_matrix
		);
		//p_instance_desc[i].InstanceContributionToHitGroupIndex = static_cast<UINT>(0);
		//p_instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		//SimpleMath::Matrix m;
		//memcpy(p_instance_desc->Transform, &m, sizeof(p_instance_desc->Transform));

		//p_instance_desc->AccelerationStructure = _BottomLevelASResources[i].pResult->GetGPUVirtualAddress();
		//p_instance_desc->InstanceMask = 0xFF;
	}



	_AccelerationStructureBuffers.pInstanceDesc->Unmap(0, nullptr);


	// TLASの作成
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = _AccelerationStructureBuffers.pInstanceDesc->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = _AccelerationStructureBuffers.pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = _AccelerationStructureBuffers.pScratch->GetGPUVirtualAddress();


	DirectXGraphics::GetInstance().GetCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = _AccelerationStructureBuffers.pResult.Get();

	DirectXGraphics::GetInstance().GetCommandList()->ResourceBarrier(1, &uavBarrier);
}

void DXRPipeLine::CreateAccelerationStructures()
{
	//_vertex_buffer = CreateTriangleVB();
	CreateTopLevelAS();

	//_BottomLevelASResource = bottom_level_buffers.pResult;
	_TopLevelASResource = _AccelerationStructureBuffers.pResult;
}

void DXRPipeLine::CreateLocalRootSignature()
{
	_closesHitLocalRootSignature = CreateClosestHitRootDesc();
	_rayGenerationLocalRootSignature = CreateRayGenRootDesc();
}

ComPtr<ID3D12RootSignature> DXRPipeLine::CreateRayGenRootDesc()
{
	D3D12_DESCRIPTOR_RANGE descUAV{};
	descUAV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descUAV.BaseShaderRegister = 0;
	descUAV.NumDescriptors = 1;

	std::array<D3D12_ROOT_PARAMETER, 1> rootParams;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &descUAV;

	ComPtr<ID3DBlob> blob, errBlob;
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	return DirectXDevice::GetInstance().CreateRootSignature(rootSigDesc);
}

ComPtr<ID3D12RootSignature> DXRPipeLine::CreateClosestHitRootDesc()
{
	D3D12_DESCRIPTOR_RANGE rangeIB{};
	rangeIB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeIB.BaseShaderRegister = 0;
	rangeIB.NumDescriptors = 1;
	rangeIB.RegisterSpace = 1;

	D3D12_DESCRIPTOR_RANGE rangeVB{};
	rangeVB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	rangeVB.BaseShaderRegister = 1;
	rangeVB.NumDescriptors = 1;
	rangeVB.RegisterSpace = 1;

	//D3D12_DESCRIPTOR_RANGE rangeMatB{};
	//rangeMatB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//rangeMatB.BaseShaderRegister = 0;
	//rangeMatB.NumDescriptors = 1;
	//rangeMatB.RegisterSpace = 1;



	//D3D12_DESCRIPTOR_RANGE rangeMat{};
	//rangeMat.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//rangeMat.BaseShaderRegister = 2;
	//rangeMat.NumDescriptors = 1;
	//rangeMat.RegisterSpace = 1;


	std::array<D3D12_ROOT_PARAMETER, 3> rootParams;

	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &rangeIB;

	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[1].DescriptorTable.pDescriptorRanges = &rangeVB;

	rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParams[2].Descriptor.ShaderRegister = 0;
	rootParams[2].Descriptor.RegisterSpace = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	return DirectXDevice::GetInstance().CreateRootSignature(rootSigDesc);
}

DXilLibrary DXRPipeLine::CreateDxliLibrary(const wchar_t* shaderPath)
{
	auto p_blob = CompileLibrary(shaderPath, L"lib_6_3");

	// 後で指定できるように変更
	std::vector<std::wstring> entryPoints = { L"rayGen",L"miss",L"chs" };

	return DXilLibrary(p_blob, entryPoints, entryPoints.size());
}

void DXRPipeLine::CreatePipeleineState(const wchar_t* shaderPath)
{
	// 後でここ対応できるように
	const WCHAR* missShader = L"miss";
	const WCHAR* rayGenShader = L"rayGen";
	const WCHAR* hitGroup = L"HitGroup";
	const WCHAR* closeHitShader = L"chs";


	CD3DX12_STATE_OBJECT_DESC subObjects;
	subObjects.SetStateObjectType(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

	// シェーダーのコンパイル
	auto shaderBin = CompileLibrary(shaderPath, L"lib_6_3");;

	D3D12_SHADER_BYTECODE shaderByteCode{ shaderBin->GetBufferPointer(),shaderBin->GetBufferSize() };

	auto dxilLib = subObjects.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

	// 各関数のレコードを登録
	dxilLib->SetDXILLibrary(&shaderByteCode);
	dxilLib->DefineExport(L"rayGen");
	dxilLib->DefineExport(L"miss");
	dxilLib->DefineExport(L"chs");
	dxilLib->DefineExport(L"shadowMiss");

	// ヒットグループの設定
	CD3DX12_HIT_GROUP_SUBOBJECT* hit_group_subobject = subObjects.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hit_group_subobject->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	hit_group_subobject->SetClosestHitShaderImport(L"chs");
	hit_group_subobject->SetHitGroupExport(L"HitGroup");

	// Global RootSigの設定
	auto rootSig = subObjects.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	rootSig->SetRootSignature(_globalRootSignature.Get());


	// Local RootSigの設定
	auto rsModel = subObjects.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	rsModel->SetRootSignature(_closesHitLocalRootSignature.Get());

	auto lrsAssocModel = subObjects.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	lrsAssocModel->AddExport(L"HitGroup");
	lrsAssocModel->SetSubobjectToAssociate(*rsModel);


	auto rsRayGen = subObjects.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	rsRayGen->SetRootSignature(_rayGenerationLocalRootSignature.Get());

	auto lrsAssocRGS = subObjects.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	lrsAssocRGS->AddExport(L"rayGen");
	lrsAssocRGS->SetSubobjectToAssociate(*rsRayGen);


	// シェーダーの設定
	auto shaderConfig = subObjects.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	shaderConfig->Config(sizeof(XMFLOAT3) + sizeof(UINT), sizeof(XMFLOAT2));

	auto pipelineConfig = subObjects.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();

	// 再帰回数
	pipelineConfig->Config(16);

	// 生成
	auto device = DirectXDevice::GetInstance().GetDevice();
	device->CreateStateObject(
		subObjects, IID_PPV_ARGS(_PipelineState.ReleaseAndGetAddressOf()));

}

void DXRPipeLine::CreateShaderTable()
{
	const WCHAR* kRayGenShader = L"rayGen";
	const WCHAR* kMissShader = L"miss";
	const WCHAR* kHitGroup = L"HitGroup";

	const auto ShaderRecordAlignment = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	UINT rayGenRecordSize = 0;

	// ShaderIdentifierとローカルルートシグネチャの分
	rayGenRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	rayGenRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	rayGenRecordSize = align_to(ShaderRecordAlignment, rayGenRecordSize);


	// HitGroup
	// ShaderIdenfierとVB/IB/Materialのデスクリプタ
	UINT hitGroupRecordSize = 0;
	hitGroupRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	hitGroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	hitGroupRecordSize += sizeof(D3D12_GPU_DESCRIPTOR_HANDLE);
	hitGroupRecordSize = align_to(ShaderRecordAlignment, hitGroupRecordSize);

	// MissShader
	// 現状はローカルルートシグネチャ使用なし
	UINT missRecordSize = 0;
	missRecordSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	missRecordSize = align_to(ShaderRecordAlignment, missRecordSize);

	// 使用するシェーダの個数が影響する
	UINT hitGroupCount = _meshDatas.size();
	UINT rayGenSize = 1 * rayGenRecordSize;
	UINT missSize = 2 * missRecordSize; // 通常描画とシャドウの2つ
	UINT hitGroupSize = hitGroupCount * hitGroupRecordSize;

	UINT tableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	UINT rayGenRegion = align_to(tableAlign, rayGenSize);
	UINT missRegion = align_to(tableAlign, missSize);
	UINT hitGroupRegion = align_to(tableAlign, hitGroupSize);

	UINT tableSize = rayGenRegion + missRegion + hitGroupRegion;
	_shaderTable = CreateBuffer(tableSize,
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
		uploadHeapProps);
	_shaderTable->SetName(L"ShaderTableBuffer");


	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	_PipelineState->QueryInterface(IID_PPV_ARGS(&rtsoProps));


	uint8_t* mapped = nullptr;
	_shaderTable->Map(0, nullptr, (void**)&mapped);
	uint8_t* pStart = mapped;

	// RayGeneration 用のシェーダーレコードを書き込み.
	uint8_t* rgsStart = pStart;
	{
		uint8_t* p = rgsStart;
		auto id = rtsoProps->GetShaderIdentifier(kRayGenShader);
		if (id == nullptr) {
			throw std::logic_error("Not found ShaderIdentifier");
		}
		p += WriteShaderIdentifer(p, id);

		// ローカルルートシグネチャで u0 (出力先) を設定しているため
		// 対応するディスクリプタを書き込む.
		p += WriteGPUDescriptor(p, _SrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	}

	// Miss Shader 用のシェーダーレコードを書き込み.
	uint8_t* missStart = pStart + rayGenRegion;
	{
		auto recordStart = missStart;
		uint8_t* p = missStart;
		auto id = rtsoProps->GetShaderIdentifier(kMissShader);
		if (id == nullptr) {
			throw std::logic_error("Not found ShaderIdentifier");
		}
		p += WriteShaderIdentifer(p, id);

		// 次の開始位置
		recordStart += missRecordSize;

		// シャドウの判定
		p = recordStart;

		// エントリポイントみたいな感じ
		id = rtsoProps->GetShaderIdentifier(L"shadowMiss");
		if (id == nullptr) {
			throw std::logic_error("Not found ShaderIdentifier");
		}

		p += WriteShaderIdentifer(p, id);
	}

	// Hit Group 用のシェーダーレコードを書き込み.
	uint8_t* hitgroupStart = pStart + rayGenRegion + missRegion;
	{
		uint8_t* pRecord = hitgroupStart;

		auto cbAddress = _materialCB->GetGPUVirtualAddress();
		auto stride = sizeof(PhysicsBaseMaterial);

		for (auto mesh : _meshDatas)
		{
			pRecord = WriteMeshShaderRecord(pRecord, mesh, cbAddress, hitGroupRecordSize);

			// アドレスをずらして定数バッファのアドレスを渡す
			cbAddress += stride;
		}
	}

	_shaderTable->Unmap(0, nullptr);


	// シェーダーの設定
	auto startAddress = _shaderTable->GetGPUVirtualAddress();

	auto& shaderRecordRG = _dispathRaysDesc.RayGenerationShaderRecord;
	shaderRecordRG.StartAddress = startAddress;
	shaderRecordRG.SizeInBytes = rayGenRecordSize;
	startAddress += rayGenRegion;

	auto& shaderRecordMS = _dispathRaysDesc.MissShaderTable;
	shaderRecordMS.StartAddress = startAddress;
	shaderRecordMS.SizeInBytes = missSize;
	shaderRecordMS.StrideInBytes = missRecordSize;
	startAddress += missRegion;

	auto& shaderRecordHG = _dispathRaysDesc.HitGroupTable;
	shaderRecordHG.StartAddress = startAddress;
	shaderRecordHG.SizeInBytes = hitGroupSize;
	shaderRecordHG.StrideInBytes = hitGroupRecordSize;
	startAddress += hitGroupRegion;

	_dispathRaysDesc.Width = _WindowSize.window_Width;
	_dispathRaysDesc.Height = _WindowSize.window_Height;
	_dispathRaysDesc.Depth = 1;





}

void DXRPipeLine::CreateShaderResource()
{

	_WindowSize = WindowApp::GetInstance().GetWindowSize();



	// アウトプット用のリソース？

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	resDesc.Width = _WindowSize.window_Width;
	resDesc.Height = _WindowSize.window_Height;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;

	// リソース生成
	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&_OutPutResource));

	// UAVビューの生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	DirectXDevice::GetInstance().GetDevice()->CreateUnorderedAccessView(
		_OutPutResource.Get(), nullptr, &uavDesc, _SrvUavHeap->GetCPUDescriptorHandleForHeapStart());


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	// レイトレの加速構造
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = _TopLevelASResource->GetGPUVirtualAddress();

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = _SrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);

}

void DXRPipeLine::CreateResourceView(std::shared_ptr<MeshData> mesh)
{

	_IncSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//auto cpu_start = _SrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	//auto gpu_start = _SrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	//cpu_start.ptr += _IncSize * _SRVResourceCount;
	//gpu_start.ptr += _IncSize * _SRVResourceCount;

	//auto vp_handle = 


	// VertexBufferView IndexBufferView MatBufferView
	//cpu_start.ptr += index * (_IncSize * 2);
	//gpu_start.ptr += index * (_IncSize * 2);


	auto vb_cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		_SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),
		_AllocateCount,
		_IncSize
	);

	auto vb_gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		_SrvUavHeap->GetGPUDescriptorHandleForHeapStart(),
		_AllocateCount,
		_IncSize
	);;

	_AllocateCount++;

	auto ib_cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		_SrvUavHeap->GetCPUDescriptorHandleForHeapStart(),
		_AllocateCount,
		_IncSize
	);
	;
	auto ib_gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
		_SrvUavHeap->GetGPUDescriptorHandleForHeapStart(),
		_AllocateCount,
		_IncSize
	);;

	_AllocateCount++;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc2.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.Buffer.FirstElement = 0;
	srvDesc2.Buffer.NumElements = mesh->m_indexCount;
	srvDesc2.Buffer.StructureByteStride = UINT(sizeof(UINT));

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		mesh->GetIndexBuffer()->getBuffer(),
		&srvDesc2,
		ib_cpuHandle);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = mesh->m_VertexCount;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.StructureByteStride = UINT(sizeof(MeshData::Vertex));

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		mesh->GetVertexBuffer()->getBuffer(),
		&srvDesc,
		vb_cpuHandle);


	mesh->m_vb_h_cpu_descriptor_handle = vb_cpuHandle;
	mesh->m_ib_h_cpu_descriptor_handle = ib_cpuHandle;

	mesh->m_vb_h_gpu_descriptor_handle = vb_gpuHandle;
	mesh->m_ib_h_gpu_descriptor_handle = ib_gpuHandle;

}

void DXRPipeLine::DeleteInstance()
{
	for (auto itr = _instances.begin(); itr != _instances.end();)
	{
		if ((*itr)->_DestroyFlag)
		{
			itr = _instances.erase(itr);
			continue;
		}
		++itr;
	}

}

void DXRPipeLine::CreateGlobalRootSignature()
{
	std::array<CD3DX12_ROOT_PARAMETER, 3> rootParams;

	// TLAS を t0 レジスタに割り当てて使用する設定.
	CD3DX12_DESCRIPTOR_RANGE descRangeTLAS;
	descRangeTLAS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// CubeMapテクスチャをt1レジスタに
	CD3DX12_DESCRIPTOR_RANGE descRangeSkyBoxTex;
	descRangeSkyBoxTex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);


	rootParams[0].InitAsDescriptorTable(1, &descRangeTLAS);
	rootParams[1].InitAsConstantBufferView(0); // b0
	rootParams[2].InitAsDescriptorTable(1, &descRangeSkyBoxTex);

	CD3DX12_STATIC_SAMPLER_DESC desc;
	desc.Init(
		0
	);

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	rootSigDesc.pStaticSamplers = &desc;
	rootSigDesc.NumStaticSamplers = UINT(1);

	//RootSignatureDesc desc;
	//desc.range.resize(2);

	// AS
	//desc.range[0].BaseShaderRegister = 0;
	//desc.range[0].NumDescriptors = 1;
	//desc.range[0].RegisterSpace = 0;
	//desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//desc.range[0].OffsetInDescriptorsFromTableStart = 0;

	////　カメラ用定数バッファ
	//desc.range[1].BaseShaderRegister = 0;
	//desc.range[1].NumDescriptors = 1;
	//desc.range[1].RegisterSpace = 0;
	//desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//desc.range[1].OffsetInDescriptorsFromTableStart = 0;

	//desc.rootParams.resize(1);
	//desc.rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//desc.rootParams[0].DescriptorTable.NumDescriptorRanges = 2;
	//desc.rootParams[0].DescriptorTable.pDescriptorRanges = desc.range.data();
	//
	//desc.desc.NumParameters = UINT(desc.rootParams.size());
	//desc.desc.pParameters = desc.rootParams.data();

	_globalRootSignature = DirectXDevice::GetInstance().CreateRootSignature(rootSigDesc);
}

void DXRPipeLine::CreateSceneCB()
{
	auto sceneCBSize = sizeof(SceneParam);
	sceneCBSize = (sceneCBSize + 0xff) & ~0xff;
	_SceneCB = CreateBuffer(sceneCBSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
		uploadHeapProps);
}

void DXRPipeLine::CreateMaterialCB()
{
	const UINT meshCount = _meshDatas.size();
	std::vector<PhysicsBaseMaterial> mats;
	mats.resize(meshCount);

	for (int i = 0; i < meshCount; ++i)
	{
		mats[i] = _meshDatas[i]->_Mat;
	}

	auto bufferSize = sizeof(PhysicsBaseMaterial) * mats.size();

	// アライメント調整
	bufferSize = (bufferSize + 0xff) & ~0xff;
	
	
	_materialCB = CreateBuffer(bufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
		uploadHeapProps);

	void* mapped = nullptr;
	_materialCB->Map(0, nullptr, &mapped);
	memcpy(mapped, mats.data(), bufferSize);
	_materialCB->Unmap(0, nullptr);
}

void DXRPipeLine::CreateDescriptorHeaps()
{
	//　デスクリプタヒープの作成
	_SrvUavHeap = DirectXDevice::GetInstance().CreateDescriptorHeap(
		_SRVResourceCount + (_MaxMeshCount * _MeshDataSize), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

	_MeshSrvHeap = DirectXDevice::GetInstance().CreateDescriptorHeap(
		_MaxMeshCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

}

UINT DXRPipeLine::WriteShaderIdentifer(void* dst, const void* shaderId)
{
	memcpy(dst, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	return D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
}

UINT DXRPipeLine::WriteGPUDescriptor(void* dst, const D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	memcpy(dst, &handle, sizeof(handle));
	return UINT(sizeof(handle));
}

UINT DXRPipeLine::WriteGPUResourceAddr(void* dst,const D3D12_GPU_VIRTUAL_ADDRESS addr)
{
	memcpy(dst, &addr, sizeof(addr));

	return UINT(sizeof(addr));
}

uint8_t* DXRPipeLine::WriteMeshShaderRecord(uint8_t* dst, const std::shared_ptr<DXRMeshData> mesh, D3D12_GPU_VIRTUAL_ADDRESS address, UINT recordSize)
{
	ComPtr<ID3D12StateObjectProperties> rtsoprops;
	_PipelineState.As(&rtsoprops);

	auto entryBegin = dst;
	auto shader = mesh->_hitGroupName;

	auto id = rtsoprops->GetShaderIdentifier(shader.c_str());
	if (id == nullptr) {
		throw std::logic_error("Not found ShaderIdentifier");
	}

	dst += WriteShaderIdentifer(dst, id);
	
	dst += WriteGPUDescriptor(dst, mesh->m_ibView);
	dst += WriteGPUDescriptor(dst, mesh->m_vbView);

	// バッファのアドレスを直接書き込み
	dst += WriteGPUResourceAddr(dst, address);

	dst = entryBegin + recordSize;

	return dst;
}

D3D12_RAYTRACING_GEOMETRY_DESC DXRPipeLine::GetGeomtryDesc(std::shared_ptr<MeshData> meshData)
{
	auto geomtryDesc = D3D12_RAYTRACING_GEOMETRY_DESC{};
	geomtryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomtryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	auto vertexBufferDesc = meshData->GetVertexBuffer()->GetResourceDesc();
	auto indexBufferDesc = meshData->GetIndexBuffer()->GetResourceDesc();

	auto& triangles = geomtryDesc.Triangles;
	triangles.VertexBuffer.StartAddress = meshData->GetVertexBuffer()->GetVirtualAdress();
	triangles.VertexBuffer.StrideInBytes = static_cast<UINT>(sizeof(MeshData::Vertex));
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	triangles.VertexCount = static_cast<UINT>(vertexBufferDesc.Width / sizeof(MeshData::Vertex));

	triangles.IndexBuffer = meshData->GetIndexBuffer()->GetVirtualAdress();
	triangles.IndexCount = static_cast<UINT>(indexBufferDesc.Width / sizeof(UINT));
	triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

	return geomtryDesc;
}

void DXRPipeLine::SceneCBUpdate()
{
	auto camera = CameraManager::GetInstance().GetMainCamera();
	auto light = LightManager::GetInstance().GetDirectionalLight();

	SimpleMath::Vector3 lightDir{ light->GetDirection() }; // ワールド座標系での光源の向き.

	m_sceneParam.mtxView = camera->GetViewMat();
	m_sceneParam.mtxProj = camera->GetProjectMat();
	m_sceneParam.mtxViewInv = XMMatrixInverse(nullptr, m_sceneParam.mtxView);
	m_sceneParam.mtxProjInv = XMMatrixInverse(nullptr, m_sceneParam.mtxProj);

	m_sceneParam.lightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	m_sceneParam.lightDirection = XMVector3Normalize(XMLoadFloat3(&lightDir));
	m_sceneParam.ambientColor = XMVectorSet(0.4f, 0.4f, 0.4f, 0.0f);
}

