#include "SkyBox.h"

#include "Device/Texture.h"
#include "Device/DirectX/Core/Buffer.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "Utility/Camera.h"
#include "../Utility/StringUtil.h"
#include "DirectX/Core/Model/GeomtryGenerater.h"


SkyBox::SkyBox(std::string filePath,std::shared_ptr<Camera> camera)
	:m_TextureName(filePath), m_Color(SimpleMath::Color(1,1,1,1)),m_Camera(camera),
	basicVertexShaderPath(L"BasicResources/SkyBoxVertexShader.hlsl"), basicPixelShaderPath(L"BasicResources/SkyBoxPixelShader.hlsl")
{	
	 GeometryGenerator::GenerateCubeMeshData(m_MeshDatas,SimpleMath::Vector3(10, 10, 10));
	 m_Scale = SimpleMath::Vector3(1, 1, 1);
}

SkyBox::SkyBox(std::string filePath, std::shared_ptr<Camera> camera, SimpleMath::Vector3 cubeScale)
{
	GeometryGenerator::GenerateCubeMeshData(m_MeshDatas, cubeScale);
	m_Scale = SimpleMath::Vector3(1, 1, 1);
}

SkyBox::~SkyBox()
{
}

void SkyBox::Init()
{
	generateVertexBuffer();
	generateIndexBuffer();
	generateConstantBuffer();
	generateTextureBuffer();
	CreatePSO();
}

void SkyBox::Update()
{
	//updateConstBuff();
}

void SkyBox::Draw()
{
	updateConstBuff();

	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	tempCommand->SetPipelineState(m_PSO.pso.Get());
	tempCommand->SetGraphicsRootSignature(m_PSO.rootSignature.Get());

	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle;
	gpuDescHandle = basicDescHeap->GetGPUDescriptorHandleForHeapStart();

	auto heap = basicDescHeap.Get();
	tempCommand->SetDescriptorHeaps(1, &heap);
	tempCommand->SetGraphicsRootDescriptorTable(0, gpuDescHandle);

	gpuDescHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	tempCommand->SetGraphicsRootDescriptorTable(1, gpuDescHandle);


	tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	tempCommand->IASetVertexBuffers(0, 1, &m_vbView);
	tempCommand->IASetIndexBuffer(&m_ibView);
	tempCommand->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);

}

void SkyBox::SetAdjustmentPosition(SimpleMath::Vector3 pos)
{
	m_AdjustmentPosition = pos;
}

void SkyBox::SetScale(SimpleMath::Vector3 scale)
{
	m_Scale = scale;
}

void SkyBox::SetColor(SimpleMath::Color color)
{
	m_Color = color;
}

ID3D12Resource* SkyBox::GetCubeResouce()
{
	return cubeMap.Get();
}

ID3D12DescriptorHeap* SkyBox::GETDescHeap()
{
	return basicDescHeap.Get();
}

bool SkyBox::generateVertexBuffer()
{
	auto vertices = m_MeshDatas.vertices;

	m_VertexBuffer = std::make_shared<Buffer>();
	int size = sizeof(MeshData::Vertex) * vertices.size();
	m_VertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto vertBuff = m_VertexBuffer->getBuffer();

	MeshData::Vertex* vertMap = nullptr;

	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return false;

	}

	//for (int i = 0; i < vertices.size(); i++)
	//{
	//	vertMap[i] = vertices[i];
	//}

	std::copy(vertices.begin(), vertices.end(), vertMap);


	vertBuff->Unmap(0, nullptr);

	m_vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	m_vbView.StrideInBytes = sizeof(vertices[0]);

	return true;
}

bool SkyBox::generateIndexBuffer()
{


	auto _indices = m_MeshDatas.m_Indices;

	m_IndexCount = _indices.size();

	// indexを設定
	m_IndexBuffer = std::make_shared<Buffer>();
	auto size = sizeof(int) * m_IndexCount;
	m_IndexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto indexBuff = m_IndexBuffer->getBuffer();

	unsigned short* indexMap = nullptr;

	indexBuff->Map(0, nullptr, (void**)&indexMap);

	std::copy(_indices.begin(), _indices.end(), indexMap);

	//for (int i = 0; i < _indices.size(); i++)
	//{
	//	indexMap[i] = _indices[i];
	//}

	indexBuff->Unmap(0, nullptr);


	m_ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = sizeof(_indices[0]) * _indices.size();

	return true;
}

bool SkyBox::generateConstantBuffer()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.NodeMask = 0;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 2;

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&basicDescHeap))))
	{
		return false;
	}

	m_ConstantBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(ConstBufferData) + 0xff) & ~0xff;
	ID3D12Resource* constBuff = m_ConstantBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = XMMatrixIdentity();
	constMap->color = m_Color;
	constBuff->Unmap(0, nullptr);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;

	HeapHandle = basicDescHeap.Get()->GetCPUDescriptorHandleForHeapStart();

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		HeapHandle);

	HeapHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

bool SkyBox::generateTextureBuffer()
{
	//Texture* tex = new Texture(StringUtil::GetWideStringFromString(m_MaterialData.texFilePath));



	ScratchImage image;
	DirectX::TexMetadata metadata;

	LoadFromDDSFile(StringUtil::GetWideStringFromString(m_TextureName).c_str(),
		DirectX::DDS_FLAGS_NONE, &metadata, image);

	// DDSファイルのテクスチャからリソースを作成
	CreateTexture(DirectXDevice::GetInstance().GetDevice(),
		metadata, &cubeMap);

	ComPtr<ID3D12Resource> srcBuffer;

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	PrepareUpload(DirectXDevice::GetInstance().GetDevice(),
		image.GetImages(), image.GetImageCount(), metadata, subresources);

	auto totalBytes = GetRequiredIntermediateSize(cubeMap.Get(), 0, UINT(subresources.size()));
	auto staging = DirectXDevice::GetInstance().CreateResource(CD3DX12_RESOURCE_DESC::Buffer(totalBytes),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, D3D12_HEAP_TYPE_UPLOAD);

	auto command = DirectXGraphics::GetInstance().GetCommandList();

	UpdateSubresources(
		command, cubeMap.Get(), staging.Get(), 0, 0,
		subresources.size(), subresources.data());

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		cubeMap.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	command->ResourceBarrier(1, &barrier);

	DirectXGraphics::GetInstance().EndCommand();



	// シェーダーリソースビュー
	D3D12_SHADER_RESOURCE_VIEW_DESC srVDesc = {};

	srVDesc.Format = metadata.format;
	srVDesc.TextureCube.MipLevels = UINT(metadata.mipLevels);
	srVDesc.TextureCube.MostDetailedMip = 0;
	srVDesc.Texture1D.ResourceMinLODClamp = 0;
	srVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		cubeMap.Get(),
		&srVDesc,
		HeapHandle);



	return true;
}

void SkyBox::updateConstBuff()
{
	auto VPMat = m_Camera->GetViewMat() * m_Camera->GetProjectMat(); 
	m_WorldMat = SimpleMath::Matrix::Identity;
	m_WorldMat = SimpleMath::Matrix::CreateScale(m_Scale) * SimpleMath::Matrix::CreateTranslation(m_Camera->GetPosition() + m_AdjustmentPosition);
	//m_WorldMat = SimpleMath::Matrix::CreateScale(SimpleMath::Vector3(30, 30, 30)) * SimpleMath::Matrix::CreateTranslation(SimpleMath::Vector3(0,0,6));

	ConstBufferData* constMap = nullptr;
	if (m_ConstantBuffer->getBuffer()->Map(0, nullptr, (void**)&constMap) == S_OK)
	{
		constMap->mat = XMMatrixIdentity();
		constMap->mat = m_WorldMat * VPMat;
		constMap->color = m_Color;
		constMap->eyePos = m_Camera->GetPosition();
		m_ConstantBuffer->getBuffer()->Unmap(0, nullptr);
	}
}

bool SkyBox::CreatePSO()
{
	D3D12_DESCRIPTOR_RANGE descTblRange{};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descTblRangeSRV{};
	descTblRangeSRV.NumDescriptors = 1;
	descTblRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRangeSRV.BaseShaderRegister = 0;
	descTblRangeSRV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRangeSRV;
	rootparam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



	// PSOの作成
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
	rasterizeDesc.CullMode = D3D12_CULL_MODE_NONE;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	D3D12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparam), rootparam, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	if (!PSOManager::GetInstance().CreatePSO(m_PSO, basicVertexShaderPath, basicPixelShaderPath,
		&inputLayout[0], _countof(inputLayout), rasterizeDesc, blendDesc, rootSignatureDesc))
	{
		return false;
	}

	return true;
}
