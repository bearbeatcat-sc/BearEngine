#include "DXRPipeLine.h"

#include <SimpleMath.h>
#include <dxcapi.h>
#include <fstream>
#include <sstream>

#include "../WindowApp.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/DirectXGraphics.h"
#include "Device/DirectX/Core/Model/Mesh.h"
#include "Device/DirectX/Core/Model/MeshDatas.h"

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

bool DXRPipeLine::InitPipeLine()
{
	if(!DirectXDevice::GetInstance().CheckSupportedDXR())
	{
		WindowApp::GetInstance().MsgBox("Not Supported DXR.","ERROR");
		return false;
	}

	CreateAccelerationStructures();
	CreatePipeleineState(L"Resources\\Shaders\\TestShader.hlsl");
	CreateShaderResource();
	CreateShaderTable();

	return true;
}

void DXRPipeLine::Render(ID3D12Resource* pRenderResource)
{
	auto commandList = DirectXGraphics::GetInstance().GetCommandList();

	ID3D12DescriptorHeap* heaps[] = { _SrvUavHeap.Get() };
	commandList->SetDescriptorHeaps(ARRAYSIZE(heaps), heaps);	
	
	
	DirectXGraphics::GetInstance().ResourceBarrier(
		_OutPutResource.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);


	commandList->SetComputeRootSignature(_EmptyRootSig.Get());
	commandList->SetPipelineState1(_PipelineState.Get());
	commandList->DispatchRays(&_dispathRaysDesc);

	
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

void DXRPipeLine::CreateInstance(const std::shared_ptr<Mesh> mesh)
{
	auto instance = std::make_shared<DXRInstance>();

	auto vertexBuffer = mesh->GetVertexBuffer()->getBuffer();
	auto indexBuffer = mesh->GetIndexBuffer()->getBuffer();

	D3D12_RAYTRACING_GEOMETRY_DESC desc;
	desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	desc.Triangles.Transform3x4 = 0;
	desc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
	desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Triangles.VertexCount = static_cast<UINT>(vertexBuffer->GetDesc().Width) / sizeof(MeshData::Vertex);
	desc.Triangles.VertexBuffer.StrideInBytes = sizeof(MeshData::Vertex);
	desc.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
	desc.Triangles.IndexCount = static_cast<UINT>(indexBuffer->GetDesc().Width) / sizeof(int);
	desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	instance->_geometryDesc = desc;
	instance->_vertexBuffer = mesh->GetVertexBuffer();
	instance->_indexBuffer = mesh->GetIndexBuffer();

	_instances.push_back(instance);

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

void DXRPipeLine::CreateButtomLevelAS()
{
	for(auto& instance : _instances)
	{
		// 加速構造に入力する情報？
		// 加速構造を作成するための情報かも
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		inputs.NumDescs = 1;
		inputs.pGeometryDescs = &instance->_geometryDesc;
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
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = 1;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs,&info);

	D3D12_HEAP_PROPERTIES defaultHeapProps = {
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};

	D3D12_HEAP_PROPERTIES uploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};
	
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

	const int instanceCount = _instances.size();
	
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
	inputs.NumDescs = instanceCount;
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
	DirectXDevice::GetInstance().GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	D3D12_HEAP_PROPERTIES defaultHeapProps = {
	D3D12_HEAP_TYPE_DEFAULT,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};

	D3D12_HEAP_PROPERTIES uploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};

	AccelerationStructureBuffers buffers;
	buffers.pScratch = CreateBuffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, defaultHeapProps);
	buffers.pResult = CreateBuffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, defaultHeapProps);
	tlasSize = info.ResultDataMaxSizeInBytes;

	buffers.pInstanceDesc = CreateBuffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC),
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeapProps);

	
	D3D12_RAYTRACING_INSTANCE_DESC* p_instance_desc;
	buffers.pInstanceDesc->Map(0, nullptr, (void**)&p_instance_desc);
	ZeroMemory(p_instance_desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instanceCount);

	for(int i = 0; i < instanceCount; i++)
	{
		p_instance_desc[i].InstanceID = i;
		p_instance_desc[i].InstanceContributionToHitGroupIndex = static_cast<UINT>(0);
		p_instance_desc[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		SimpleMath::Matrix m;
		memcpy(p_instance_desc->Transform, &m, sizeof(p_instance_desc->Transform));

		p_instance_desc->AccelerationStructure = _BottomLevelASResources[i].pResult->GetGPUVirtualAddress();
		p_instance_desc->InstanceMask = 0xFF;
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
	_vertex_buffer = CreateTriangleVB();

	AccelerationStructureBuffers bottom_level_buffers = CreateButtomLevelAS(_vertex_buffer.Get());
	AccelerationStructureBuffers top_level_buffers = CreateTopLevelAS(bottom_level_buffers.pResult.Get(),_tlasSize);

	_BottomLevelASResource = bottom_level_buffers.pResult;
	_TopLevelASResource = top_level_buffers.pResult;
}

RootSignatureDesc DXRPipeLine::CreateRayGenRtooDesc()
{
	RootSignatureDesc desc;
	desc.range.resize(2);

	// AS
	desc.range[0].BaseShaderRegister = 0;
	desc.range[0].NumDescriptors = 1;
	desc.range[0].RegisterSpace = 0;
	desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	desc.range[0].OffsetInDescriptorsFromTableStart = 0;

	// レンダリング先のテクスチャ
	desc.range[1].BaseShaderRegister = 0;
	desc.range[1].NumDescriptors = 1;
	desc.range[1].RegisterSpace = 0;
	desc.range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desc.range[1].OffsetInDescriptorsFromTableStart = 1;

	desc.rootParams.resize(1);
	desc.rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	desc.rootParams[0].DescriptorTable.NumDescriptorRanges = 2;
	desc.rootParams[0].DescriptorTable.pDescriptorRanges = desc.range.data();


	desc.desc.NumParameters = 1;
	desc.desc.pParameters = desc.rootParams.data();

	// レイトレで使うフラグらしい
	desc.desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

	return desc;
}

DXilLibrary DXRPipeLine::CreateDxliLibrary(const wchar_t* shaderPath)
{
	auto p_blob = CompileLibrary(shaderPath, L"lib_6_3");

	// 後で指定できるように変更
	std::vector<std::wstring> entryPoints = {L"rayGen",L"miss",L"chs" };

	return DXilLibrary(p_blob, entryPoints,entryPoints.size());
}

void DXRPipeLine::CreatePipeleineState(const wchar_t* shaderPath)
{
	// 後でここ対応できるように
	const WCHAR* missShader = L"miss";
	const WCHAR* rayGenShader = L"rayGen";
	const WCHAR* hitGroup = L"HitGroup";
	const WCHAR* closeHitShader = L"chs";

	
	std::array<D3D12_STATE_SUBOBJECT, 10> subobjects;
	uint32_t index = 0;

	DXilLibrary dxillibrary = CreateDxliLibrary(shaderPath);
	subobjects[index++] = dxillibrary.stateSubobject;

	// ヒットグループ
	HitProgram hitProgram(nullptr, closeHitShader, hitGroup);
	subobjects[index++] = hitProgram.subObject;

	// レイ生成シェーダー
	LocalRootSignature local_root_signature(CreateRayGenRtooDesc().desc);
	subobjects[index] = local_root_signature.subObject;

	uint32_t rgsRootIndex = index++;
	ExportAssocication rgsRootAssociation(&rayGenShader, 1, &(subobjects[rgsRootIndex]));
	subobjects[index++] = rgsRootAssociation.subobject;

	// ヒットミスシェーダー
	D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
	emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	LocalRootSignature hitMissRootSignature(emptyDesc);
	subobjects[index] = hitMissRootSignature.subObject;

	uint32_t hitMissRootIndex = index++;
	const WCHAR* missHitExportName[] = { missShader,closeHitShader };
	ExportAssocication missHitRootAssociation(missHitExportName, 2, &(subobjects[hitMissRootIndex]));
	subobjects[index++] = missHitRootAssociation.subobject;


	// ペイロードの要素数？
	ShaderConfig shaderConfig(sizeof(float) * 2, sizeof(float) * 3);
	subobjects[index] = shaderConfig.subobject;


	uint32_t shaderConfigIndex = index++;
	const WCHAR* shaderExports[] = { missShader,closeHitShader,rayGenShader };
	ExportAssocication configAssocication(shaderExports, 3, &(subobjects[shaderConfigIndex]));
	subobjects[index++] = configAssocication.subobject;


	// パイプラインのコンフィグを生成
	PipeLineConfig config((uint32_t)1);
	subobjects[index++] = config.subobject;

	// GlobalRootSignatureを生成
	GlobalRootSignature root({});
	_EmptyRootSig = root.pRootSig;
	subobjects[index++] = root.subObject;


	D3D12_STATE_OBJECT_DESC desc;
	desc.NumSubobjects = index;
	desc.pSubobjects = subobjects.data();
	desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	DirectXDevice::GetInstance().GetDevice()->CreateStateObject(
		&desc, IID_PPV_ARGS(&_PipelineState));
	
}

void DXRPipeLine::CreateShaderTable()
{
	const WCHAR* kRayGenShader = L"rayGen";
	const WCHAR* kMissShader = L"miss";
	 const WCHAR* kHitGroup = L"HitGroup";

	D3D12_HEAP_PROPERTIES uploadHeapProps = {
	D3D12_HEAP_TYPE_UPLOAD,
	D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	D3D12_MEMORY_POOL_UNKNOWN,
	0,
	0
	};
	
	_shaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	_shaderTableEntrySize += 8;

	// アライメントを64バイトに
	_shaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,_shaderTableEntrySize);
	uint32_t shaderTableSize = _shaderTableEntrySize * 3; // 3つ分のシェーダーなので


	// シェーダーテーブルの生成
	_shaderTable = CreateBuffer(shaderTableSize,
		D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ,
		uploadHeapProps);

	// データコピー

	ComPtr<ID3D12StateObjectProperties> pRtsoProps;
	_PipelineState->QueryInterface(IID_PPV_ARGS(&pRtsoProps));
	
	uint8_t* pData;
	_shaderTable->Map(0, nullptr, (void**)&pData);

	// RayGeneration用のシェーダーレコードを書き込み
	memcpy(pData, pRtsoProps->GetShaderIdentifier(kRayGenShader),D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	// MissShader用のシェーダーレコードを書き込み
	uint8_t* pMissEntry = pData + _shaderTableEntrySize ;
	memcpy(pMissEntry, pRtsoProps->GetShaderIdentifier(kMissShader), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	// HitGroup用のシェーダーレコードを書き込み
	uint8_t* pHitEntry = pData + _shaderTableEntrySize * 2;
	memcpy(pHitEntry, pRtsoProps->GetShaderIdentifier(kHitGroup), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	_shaderTable->Unmap(0, nullptr);

	// Dispatch用の設定
	_dispathRaysDesc = {};
	_dispathRaysDesc.Width = _WindowSize.window_Width;
	_dispathRaysDesc.Height = _WindowSize.window_Height;
	_dispathRaysDesc.Depth = 1;


	// シェーダーの設定？

	auto startAddress = _shaderTable->GetGPUVirtualAddress();
	
	_dispathRaysDesc.RayGenerationShaderRecord.StartAddress = startAddress;
	_dispathRaysDesc.RayGenerationShaderRecord.SizeInBytes = _shaderTableEntrySize;

	size_t missOffSet = 1 * _shaderTableEntrySize;
	_dispathRaysDesc.MissShaderTable.StartAddress = startAddress + missOffSet;
	_dispathRaysDesc.MissShaderTable.StrideInBytes = _shaderTableEntrySize;
	_dispathRaysDesc.MissShaderTable.SizeInBytes = _shaderTableEntrySize;

	size_t hitOffset = 2 * _shaderTableEntrySize;
	_dispathRaysDesc.HitGroupTable.StartAddress = startAddress + hitOffset;
	_dispathRaysDesc.HitGroupTable.StrideInBytes = _shaderTableEntrySize;
	_dispathRaysDesc.HitGroupTable.SizeInBytes = _shaderTableEntrySize;
	
}

void DXRPipeLine::CreateShaderResource()
{
	static const D3D12_HEAP_PROPERTIES kDefaultHeapProps =
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};

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
		&kDefaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&_OutPutResource));

	//　デスクリプタヒープの作成
	_SrvUavHeap = DirectXDevice::GetInstance().CreateDescriptorHeap(
		2, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);


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