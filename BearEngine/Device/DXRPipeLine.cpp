#include "DXRPipeLine.h"

#include <SimpleMath.h>
#include <dxcapi.h>
#include <fstream>
#include <sstream>

#include "WindowApp.h"
#include "DirectX/DirectXDevice.h"
#include "DirectX/DirectXGraphics.h"

// TODO:あとで別のクラスに分離する
// ヘルパー関数的なやつ
IDxcBlob* CompileLibrary(const WCHAR* filename, const WCHAR* targetString)
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

	IDxcBlob* p_blob;
	p_result->GetResult(&p_blob);

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
}

ID3D12Resource* DXRPipeLine::CreateTriangleVB()
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

	ID3D12Resource* p_buffer = CreateBuffer(sizeof(vertices), D3D12_RESOURCE_FLAG_NONE,
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

void DXRPipeLine::CreateAccelerationStructures()
{
	_vertex_buffer = CreateTriangleVB();

	AccelerationStructureBuffers bottom_level_buffers = CreateButtomLevelAS(_vertex_buffer);
	AccelerationStructureBuffers top_level_buffers = CreateTopLevelAS(bottom_level_buffers.pResult.Get(),_tlasSize);

	_bottomlevel_as = bottom_level_buffers;
	_toplevel_as = top_level_buffers;	
}

RootSignatureDesc DXRPipeLine::CreateRayGenRtooDesc()
{
	RootSignatureDesc desc;
	desc.range.resize(2);

	desc.range[0].BaseShaderRegister = 0;
	desc.range[0].NumDescriptors = 1;
	desc.range[0].RegisterSpace = 0;
	desc.range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	desc.range[0].OffsetInDescriptorsFromTableStart = 0;

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

DXilLibrary DXRPipeLine::CreateDxliLibrary(const std::wstring& shaderPath)
{
	auto p_blob = CompileLibrary(shaderPath.c_str(), L"lib_6_3");

	// 後で指定できるように変更
	std::vector<std::wstring> entryPoints = {L"rayGen",L"miss",L"chs" };

	return DXilLibrary(p_blob, entryPoints,entryPoints.size());
}