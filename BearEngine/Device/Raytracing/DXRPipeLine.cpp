#include "DXRPipeLine.h"

#include <SimpleMath.h>
#include <dxcapi.h>
#include <fstream>
#include <sstream>

#include "DXRMesh.h"
#include "../WindowApp.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/DirectXGraphics.h"
#include "Device/DirectX/Core/Model/Mesh.h"
#include "Device/DirectX/Core/Model/MeshDatas.h"
#include "Device/DirectX/Core/Model/MeshManager.h"
#include "Utility/Camera.h"
#include "Utility/CameraManager.h"

#pragma comment(lib, "dxcompiler")

// TODO:あとで別のクラスに分離する
// ヘルパー関数的なやつ
ComPtr<IDxcBlob> CompileLibrary(const WCHAR* filename, const WCHAR* targetString)
{
	static IDxcCompiler* p_dxc_compiler = nullptr;
	static IDxcLibrary* p_dxc_library = nullptr;
	static IDxcIncludeHandler* p_dxc_include_handler = nullptr;

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
	IDxcBlobEncoding* p_blob_encoding;
	p_dxc_library->CreateBlobWithEncodingFromPinned(
		(LPBYTE)sShader.c_str(), (uint32_t)sShader.size(), 0, &p_blob_encoding);

	// コンパイル
	IDxcOperationResult* p_result;
	p_dxc_compiler->Compile(p_blob_encoding, filename, L"", L"lib_6_3",
		nullptr, 0, nullptr, 0, p_dxc_include_handler, &p_result);


	HRESULT resultcode;
	p_result->GetStatus(&resultcode);

	// エラーの解析
	if (FAILED(resultcode))
	{
		IDxcBlobEncoding* p_error;
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
	CreateShaderTable();

	return true;
}

bool DXRPipeLine::Init()
{
	CreateDescriptorHeaps();

	return true;
}

void DXRPipeLine::AddMesh(std::shared_ptr<DXRMesh> mesh)
{
	_meshs.push_back(mesh);
}

void DXRPipeLine::Render(ID3D12Resource* pRenderResource)
{
	auto commandList = DirectXGraphics::GetInstance().GetCommandList();

	// カメラ用のCBの更新
	// 後でパラメータの変更したときだけ変えるようにする。
	SceneCBUpdate();
	void* dst = nullptr;
	_SceneCB->Map(0, nullptr, &dst);
	memcpy(dst, &m_sceneParam, sizeof(SceneParam));
	_SceneCB->Unmap(0, nullptr);

	commandList->SetPipelineState1(_PipelineState.Get());

	ID3D12DescriptorHeap* heaps[] = { _SrvUavHeap.Get() };
	commandList->SetDescriptorHeaps(ARRAYSIZE(heaps), heaps);
	commandList->SetComputeRootSignature(_globalRootSignature.Get());

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

ComPtr<ID3D12Resource> DXRPipeLine::CreateTriangleVB()
{
	const SimpleMath::Vector3 vertices[] =
	{
		SimpleMath::Vector3(0,1,0),
		SimpleMath::Vector3(0.866f,-0.5f,0),
		SimpleMath::Vector3(-0.866f,-0.5f,0)
	};

	D3D12_HEAP_PROPERTIES uploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};

	ComPtr<ID3D12Resource> p_buffer = CreateBuffer(sizeof(vertices), D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeapProps);

	uint8_t* pData;
	p_buffer->Map(0, nullptr, (void**)&pData);
	memcpy(pData, vertices, sizeof(vertices));
	p_buffer->Unmap(0, nullptr);

	return p_buffer;
}

AccelerationStructureBuffers DXRPipeLine::CreateButtomLevelAS(ID3D12Resource* pVB)
{
	// ジオメトリ情報を定義？
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomDesc.Triangles.VertexBuffer.StartAddress = pVB->GetGPUVirtualAddress();
	geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(DirectX::SimpleMath::Vector3);
	geomDesc.Triangles.VertexCount = 3;
	geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// 加速構造に入力する情報？
	// 加速構造を作成するための情報かも
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &geomDesc;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

	// 必要なメモリ量を求める
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

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

void DXRPipeLine::CreateButtomLevelAS()
{
	for (auto& mesh : _meshs)
	{
		auto geomtryDesc = GetGeomtryDesc(mesh);

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

		_BottomLevelASResources.push_back(buffers);

		mesh->_blas = buffers.pResult;
		mesh->CreateRaytracingInstanceDesc();
	}
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

	ID3D12Resource* pBuffer;


	if (DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc, initState, nullptr, IID_PPV_ARGS(&pBuffer)) != S_OK)
	{
		WindowApp::GetInstance().MsgBox("バッファの生成に失敗", "ERROR");
		assert(0);
	}

	return pBuffer;
}

AccelerationStructureBuffers DXRPipeLine::CreateTopLevelAS(ID3D12Resource* pBottomLevelAS, uint64_t& tlasSize)
{
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	AccelerationStructureBuffers buffers;
	buffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	buffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, defaultHeapProps);
	tlasSize = info.ResultDataMaxSizeInBytes;

	buffers.pInstanceDesc = CreateBuffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeapProps);

	D3D12_RAYTRACING_INSTANCE_DESC* p_instance_desc;
	buffers.pInstanceDesc->Map(0, nullptr, (void**)&p_instance_desc);

	p_instance_desc->InstanceID = 0;
	p_instance_desc->InstanceContributionToHitGroupIndex = 0;
	p_instance_desc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

	SimpleMath::Matrix m;
	memcpy(p_instance_desc->Transform, &m, sizeof(p_instance_desc->Transform));

	p_instance_desc->AccelerationStructure = pBottomLevelAS->GetGPUVirtualAddress();
	p_instance_desc->InstanceMask = 0xFF;

	buffers.pInstanceDesc->Unmap(0, nullptr);


	// TLASの作成
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = buffers.pInstanceDesc->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();


	DirectXGraphics::GetInstance().GetCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = buffers.pResult.Get();

	DirectXGraphics::GetInstance().GetCommandList()->ResourceBarrier(1, &uavBarrier);


	return buffers;
}

AccelerationStructureBuffers DXRPipeLine::CreateTopLevelAS()
{
	uint64_t tlasSize;

	const int instanceCount = _meshs.size();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = instanceCount;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);



	AccelerationStructureBuffers buffers;
	buffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	buffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, defaultHeapProps);
	tlasSize = info.ResultDataMaxSizeInBytes;

	buffers.pInstanceDesc = CreateBuffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeapProps);


	D3D12_RAYTRACING_INSTANCE_DESC* p_instance_desc;
	buffers.pInstanceDesc->Map(0, nullptr, (void**)&p_instance_desc);
	ZeroMemory(p_instance_desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instanceCount);

	for (int i = 0; i < instanceCount; i++)
	{
		p_instance_desc[i] = _meshs[i]->_raytracingInstanceDesc;
		p_instance_desc[i].InstanceID = i;
		//p_instance_desc[i].InstanceContributionToHitGroupIndex = static_cast<UINT>(0);
		//p_instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		//SimpleMath::Matrix m;
		//memcpy(p_instance_desc->Transform, &m, sizeof(p_instance_desc->Transform));

		//p_instance_desc->AccelerationStructure = _BottomLevelASResources[i].pResult->GetGPUVirtualAddress();
		//p_instance_desc->InstanceMask = 0xFF;
	}



	buffers.pInstanceDesc->Unmap(0, nullptr);


	// TLASの作成
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = buffers.pInstanceDesc->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = buffers.pResult->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = buffers.pScratch->GetGPUVirtualAddress();


	DirectXGraphics::GetInstance().GetCommandList()->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = buffers.pResult.Get();

	DirectXGraphics::GetInstance().GetCommandList()->ResourceBarrier(1, &uavBarrier);


	return buffers;
}

void DXRPipeLine::CreateAccelerationStructures()
{
	//_vertex_buffer = CreateTriangleVB();

	CreateButtomLevelAS();
	AccelerationStructureBuffers top_level_buffers = CreateTopLevelAS();

	//_BottomLevelASResource = bottom_level_buffers.pResult;
	_TopLevelASResource = top_level_buffers.pResult;
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


	std::array<D3D12_ROOT_PARAMETER, 2> rootParams;
	rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[0].DescriptorTable.pDescriptorRanges = &rangeIB;

	rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
	rootParams[1].DescriptorTable.pDescriptorRanges = &rangeVB;

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
	shaderConfig->Config(sizeof(float) * 3, sizeof(float) * 2);

	auto pipelineConfig = subObjects.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();

	// 再帰回数
	pipelineConfig->Config(1);

	// 生成
	auto device = DirectXDevice::GetInstance().GetDevice();
	device->CreateStateObject(
		subObjects, IID_PPV_ARGS(_PipelineState.ReleaseAndGetAddressOf()));

	//std::array<D3D12_STATE_SUBOBJECT, 10> subobjects;
	//uint32_t index = 0;

	//DXilLibrary dxillibrary = CreateDxliLibrary(shaderPath);
	//subobjects[index++] = dxillibrary.stateSubobject;

	//// ヒットグループ
	//HitProgram hitProgram(nullptr, closeHitShader, hitGroup);
	//subobjects[index++] = hitProgram.subObject;

	//// レイ生成シェーダー
	//LocalRootSignature local_root_signature(CreateRayGenRootDesc().desc);
	//subobjects[index] = local_root_signature.subObject;

	//uint32_t rgsRootIndex = index++;
	//ExportAssocication rgsRootAssociation(&rayGenShader, 1, &(subobjects[rgsRootIndex]));
	//subobjects[index++] = rgsRootAssociation.subobject;

	//// ヒットミスシェーダー
	//D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
	//emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	//LocalRootSignature hitMissRootSignature(CreateClosestHitRootDesc().desc);
	//subobjects[index] = hitMissRootSignature.subObject;

	//uint32_t hitMissRootIndex = index++;
	//const WCHAR* missHitExportName[] = { missShader,closeHitShader };
	//ExportAssocication missHitRootAssociation(missHitExportName, 2, &(subobjects[hitMissRootIndex]));
	//subobjects[index++] = missHitRootAssociation.subobject;


	//// ペイロードの要素数？
	//ShaderConfig shaderConfig(sizeof(float) * 2, sizeof(float) * 3);
	//subobjects[index] = shaderConfig.subobject;


	//uint32_t shaderConfigIndex = index++;
	//const WCHAR* shaderExports[] = { missShader,closeHitShader,rayGenShader };
	//ExportAssocication configAssocication(shaderExports, 3, &(subobjects[shaderConfigIndex]));
	//subobjects[index++] = configAssocication.subobject;


	//// パイプラインのコンフィグを生成
	//PipeLineConfig config((uint32_t)1);
	//subobjects[index++] = config.subobject;

	//// GlobalRootSignatureを生成
	//GlobalRootSignature root({});
	//_EmptyRootSig = root.pRootSig;
	//subobjects[index++] = root.subObject;


	//D3D12_STATE_OBJECT_DESC desc;
	//desc.NumSubobjects = index;
	//desc.pSubobjects = subobjects.data();
	//desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	//DirectXDevice::GetInstance().GetDevice()->CreateStateObject(
	//	&desc, IID_PPV_ARGS(&_PipelineState));

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
	// ShaderIdenfierとVB/IBのデスクリプタ
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
	UINT hitGroupCount = _meshs.size();
	UINT rayGenSize = 1 * rayGenRecordSize;
	UINT missSize = 1 * rayGenRecordSize;
	UINT hitGroupSize = hitGroupCount * hitGroupRecordSize;

	auto tableAlign = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	UINT rayGenRegion = align_to(rayGenRecordSize, tableAlign);
	auto missRegion = align_to(missSize, tableAlign);
	auto hitGroupRegion = align_to(hitGroupSize, tableAlign);

	auto tableSize = rayGenRegion + missRegion + hitGroupRegion;
	_shaderTable = CreateBuffer(tableSize,
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
		uploadHeapProps);


	ComPtr<ID3D12StateObjectProperties> rtsoProps;
	_PipelineState->QueryInterface(IID_PPV_ARGS(&rtsoProps));


	// レコードに書き込み
	void* mapped = nullptr;
	_shaderTable->Map(0, nullptr, &mapped);
	uint8_t* pStart = static_cast<uint8_t*>(mapped);

	// RayGeneration用のシェーダーレコードを書き込み
	auto rgsStart = pStart;
	{
		uint8_t* p = rgsStart;
		auto id = rtsoProps->GetShaderIdentifier(kRayGenShader);

		// RayGenerationシェーダーの識別子
		// 出力先のデスクリプタ
		p += WriteShaderIdentifer(p, id);
		p += WriteGPUDescriptor(p, _SrvUavHeap.Get()->GetGPUDescriptorHandleForHeapStart());
	}

	// MissShader用のシェーダーレコードを書き込み
	auto missStart = pStart + rayGenRegion;
	{
		uint8_t* p = missStart;
		auto id = rtsoProps->GetShaderIdentifier(kMissShader);

		// Missシェーダーの識別子
		p += WriteShaderIdentifer(p, id);
	}

	auto hitGroupStart = pStart + rayGenRegion + missRegion;
	{
		uint8_t* p = hitGroupStart;
		
		// メッシュの数分だけ
		for (auto mesh : _meshs)
		{
			p = WriteMeshShaderRecord(p, mesh, hitGroupRecordSize);
		}

	}

	_shaderTable->Unmap(0, nullptr);

	// シェーダーの設定
	auto startAddress = _shaderTable->GetGPUVirtualAddress();

	auto& shaderRecordRG = _dispathRaysDesc.RayGenerationShaderRecord;
	shaderRecordRG.StartAddress = startAddress;
	shaderRecordRG.SizeInBytes = rayGenSize;
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
	
	_RegistMeshCount++;

	int index = _RegistMeshCount;
	auto cpu_start = _SrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	auto gpu_start = _SrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	cpu_start.ptr += _IncSize * _SRVResourceCount;
	gpu_start.ptr += _IncSize * _SRVResourceCount;


	cpu_start.ptr += index * (_IncSize * 2);
	gpu_start.ptr += index * (_IncSize * 2);

	auto vb_cpuHandle = cpu_start;
	auto vb_gpuHandle = gpu_start;
	
	cpu_start.ptr += _IncSize;
	gpu_start.ptr += _IncSize;

	
	
	auto ib_cpuHandle = cpu_start;
	auto ib_gpuHandle = gpu_start;
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = mesh->m_VertexCount;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.StructureByteStride = sizeof(MeshData::Vertex);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		mesh->GetVertexBuffer()->getBuffer(),
		&srvDesc,
		vb_cpuHandle);

	srvDesc.Buffer.NumElements = mesh->m_indexCount;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.StructureByteStride = sizeof(UINT);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		mesh->GetIndexBuffer()->getBuffer(),
		&srvDesc,
		ib_cpuHandle);

	mesh->m_vb_h_gpu_descriptor_handle = vb_gpuHandle;
	mesh->m_vb_h_cpu_descriptor_handle = vb_cpuHandle;
	mesh->m_ib_h_cpu_descriptor_handle = ib_cpuHandle;
	mesh->m_ib_h_gpu_descriptor_handle = ib_gpuHandle;
}

void DXRPipeLine::CreateGlobalRootSignature()
{
	std::array<CD3DX12_ROOT_PARAMETER, 2> rootParams;

	// TLAS を t0 レジスタに割り当てて使用する設定.
	CD3DX12_DESCRIPTOR_RANGE descRangeTLAS;
	descRangeTLAS.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	rootParams[0].InitAsDescriptorTable(1, &descRangeTLAS);
	rootParams[1].InitAsConstantBufferView(0); // b0

	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.NumParameters = UINT(rootParams.size());
	rootSigDesc.pParameters = rootParams.data();
	
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
	_SceneCB = CreateBuffer(sizeof(SceneParam), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
		uploadHeapProps);
}

void DXRPipeLine::CreateDescriptorHeaps()
{
	//　デスクリプタヒープの作成
	_SrvUavHeap = DirectXDevice::GetInstance().CreateDescriptorHeap(
		_SRVResourceCount + _MaxMeshCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

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

uint8_t* DXRPipeLine::WriteMeshShaderRecord(uint8_t* dst, const std::shared_ptr<DXRMesh> mesh, UINT recordSize)
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

	// インデックスバッファ
	// 頂点バッファ
	dst += WriteGPUDescriptor(dst, mesh->m_ibView);
	dst += WriteGPUDescriptor(dst, mesh->m_vbView);

	dst = entryBegin + recordSize;

	return dst;
}

D3D12_RAYTRACING_GEOMETRY_DESC DXRPipeLine::GetGeomtryDesc(std::shared_ptr<DXRMesh> mesh)
{
	auto geomtryDesc = D3D12_RAYTRACING_GEOMETRY_DESC{};
	geomtryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geomtryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	auto& triangles = geomtryDesc.Triangles;
	triangles.VertexBuffer.StartAddress = mesh->_vertexBuffer->getBuffer()->GetGPUVirtualAddress();
	triangles.VertexBuffer.StrideInBytes = mesh->_vertexStride;
	triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	triangles.VertexCount = mesh->_vertexCount;
	
	triangles.IndexBuffer = mesh->_indexBuffer->getBuffer()->GetGPUVirtualAddress();
	triangles.IndexCount = mesh->_indexCount;
	triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

	return geomtryDesc;
}

void DXRPipeLine::SceneCBUpdate()
{
	auto camera = CameraManager::GetInstance().GetMainCamera();
	
	XMFLOAT3 lightDir{ -0.5f,-1.0f, -0.5f }; // ワールド座標系での光源の向き.

	m_sceneParam.mtxView = camera->GetViewMat();
	m_sceneParam.mtxProj = camera->GetProjectMat();
	m_sceneParam.mtxViewInv = XMMatrixInverse(nullptr, m_sceneParam.mtxView);
	m_sceneParam.mtxProjInv = XMMatrixInverse(nullptr, m_sceneParam.mtxProj);

	m_sceneParam.lightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	m_sceneParam.lightDirection = XMVector3Normalize(XMLoadFloat3(&lightDir));
	m_sceneParam.ambientColor = XMVectorSet(0.2f, 0.2f, 0.2f, 0.0f);
}

