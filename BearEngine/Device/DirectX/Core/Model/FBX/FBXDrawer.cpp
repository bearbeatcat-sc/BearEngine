#include "FBXDrawer.h"
#include "../../../../../Device/DirectX/DirectXDevice.h"
#include "../../../../../Device/DirectX/Core/Buffer.h"
#include "FBXMesh.h"

#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "zlib-md.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "libfbxsdk.lib")


FBXDrawer::FBXDrawer()
{
}

FBXDrawer::~FBXDrawer()
{
	models.clear();
}

bool FBXDrawer::Init()
{
	bool result = initPSO();
	result = generateConstantBuffer();

	return result;
}

bool FBXDrawer::Load(const char * filePath, std::string modelName)
{
	FBXModel* temp = new FBXModel();
	bool result = temp->load(filePath);
	models.emplace(modelName, temp);

	return result;
}

void FBXDrawer::Draw(std::string modelName, XMMATRIX mat)
{
	auto meshs = models.at(modelName)->getMeshs();

	ConstBufferData* constMap = nullptr;
	m_ConstantBuffer->getBuffer()->Map(0, nullptr, (void**)&constMap);
	constMap->mat = XMMatrixIdentity();
	constMap->mat = mat;
	m_ConstantBuffer->getBuffer()->Unmap(0, nullptr);


	for (auto mesh : meshs)
	{
		ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

		tempCommand->SetPipelineState(m_PSO.pso.Get());
		tempCommand->SetGraphicsRootSignature(m_PSO.rootSignature.Get());

		D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle;
		gpuDescHandle = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

		auto heap = basicDescHeap.Get();
		tempCommand->SetDescriptorHeaps(1, &heap);



		tempCommand->SetGraphicsRootDescriptorTable(0, gpuDescHandle);

		tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		tempCommand->IASetVertexBuffers(0, 1, &mesh->getVertexBufferView());
		tempCommand->IASetIndexBuffer(&mesh->getIndexBufferView());
		tempCommand->DrawIndexedInstanced(mesh->getIndexCount(), 1, 0, 0, 0);

	}

}

bool FBXDrawer::generateConstantBuffer()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1;

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&basicDescHeap))))
	{
		return false;
	}

	m_ConstantBuffer = std::shared_ptr<Buffer>(new Buffer());
	UINT buffSize = (sizeof(ConstBufferData) + 0xff) & ~0xff;
	ID3D12Resource* constBuff = m_ConstantBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = XMMatrixIdentity();
	constBuff->Unmap(0, nullptr);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;

	basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		basicHeapHandle);

	//basicHeapHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
	//	D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

bool FBXDrawer::initPSO()
{

	D3D12_DESCRIPTOR_RANGE descTblRange{};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.NumDescriptors = 1;
	descRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRangeSRV.BaseShaderRegister = 0;
	descRangeSRV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descRangeSRV;
	rootparam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;



	// PSO�̍쐬
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;


	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	D3D12_RASTERIZER_DESC rasterizeDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparam), rootparam, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	const wchar_t* basicShaderPath = L"Resources/ModelVertexShader.hlsl";
	const wchar_t* basicPixelPath = L"Resources/ModelPixelShader.hlsl";



	if (!PSOManager::GetInstance().CreatePSO(m_PSO, basicShaderPath, basicPixelPath,
		&inputLayout[0], _countof(inputLayout), rasterizeDesc, blendDesc, rootSignatureDesc))
	{
		return false;
	}

	return true;
}
