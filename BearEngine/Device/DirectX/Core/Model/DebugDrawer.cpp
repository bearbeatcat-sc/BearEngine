#include "DebugDrawer.h"

#include "Utility/CameraManager.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "GeomtryGenerater.h"
#include "../Buffer.h"
#include "Utility/Camera.h"

DebugDrawer::DebugDrawer()
{
}

DebugDrawer::~DebugDrawer()
{
	m_ConstantBuffers.clear();
}

bool DebugDrawer::Init(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath)
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();

	D3D12_DESCRIPTOR_RANGE descTblRange{};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam.DescriptorTable.NumDescriptorRanges = 1;
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
	rasterizeDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;





	if (!PSOManager::GetInstance().CreatePSO(m_PSO, vertexShaderPath, pixelShaderPath, &inputLayout[0], _countof(inputLayout),
		rasterizeDesc, blendDesc, rootSignatureDesc))
	{
		return false;
	}

	GenerateCubeData();


	return true;
}

bool DebugDrawer::GenerateVertexBuffer(std::vector<XMFLOAT3>& vertices)
{

	m_VertexBuffer = std::make_shared<Buffer>();
	int size = sizeof(XMFLOAT3) * vertices.size();
	m_VertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto vertBuff = m_VertexBuffer->getBuffer();

	// バッファ生成失敗
	if (vertBuff == nullptr)
	{
		return false;
	}

	XMFLOAT3* vertMap = nullptr;

	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return false;
	}

	std::copy(vertices.begin(), vertices.end(), vertMap);


	vertBuff->Unmap(0, nullptr);


	m_vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	m_vbView.StrideInBytes = sizeof(vertices[0]);

	return true;
}

bool DebugDrawer::GenerateIndexBuffer(const std::vector<unsigned short>& indices)
{
	m_IndexCount = indices.size();

	// indexを設定
	m_IndexBuffer = std::make_shared<Buffer>();
	int size = sizeof(int) * m_IndexCount;
	m_IndexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto indexBuff = m_IndexBuffer->getBuffer();

	unsigned short* indexMap = nullptr;

	indexBuff->Map(0, nullptr, (void**)&indexMap);

	std::copy(indices.begin(), indices.end(), indexMap);


	indexBuff->Unmap(0, nullptr);


	m_ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = sizeof(indices[0]) * indices.size();

	return true;
}

void DebugDrawer::GenerateCubeData()
{
	std::vector<XMFLOAT3> vertices;
	std::vector<unsigned short> indicies;

	GeometryGenerator::GenerateCubeDatas(vertices, indicies, DirectX::SimpleMath::Vector3(1, 1, 1));

	if (!GenerateVertexBuffer(vertices))
	{
		return;
	}

	if (!GenerateIndexBuffer(indicies))
	{
		return;
	}

	if (!GenerateConstantBuffers())
	{
		return;
	}

	if (!InitConstantHeaps())
	{
		return;
	}

	if (!GenerateConstantView())
	{
		return;
	}
}

bool DebugDrawer::GenerateConstantView()
{
	UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto handle = m_ConstDescHeap->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < m_ConstantBuffers.size(); i++)
	{
		auto constBuff = m_ConstantBuffers[i]->getBuffer();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
		cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;

		DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
			&cbvDesc,
			handle);

		handle.ptr += descHandleInc;
	}

	return true;
}

bool DebugDrawer::InitConstantHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 1 * m_DrawObjectCount;

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&m_ConstDescHeap))))
	{
		return false;
	}

	return true;
}

bool DebugDrawer::GenerateConstantBuffers()
{
	for (int i = 0; i < m_DrawObjectCount; i++)
	{
		auto constBuff_ = std::make_shared<Buffer>();

		UINT buffSize = (sizeof(ConstBufferData) + 0xff) & ~0xff;
		constBuff_->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
		auto constBuff = constBuff_->getBuffer();

		ConstBufferData* constMap = nullptr;
		constBuff->Map(0, nullptr, (void**)&constMap);
		constBuff->Unmap(0, nullptr);

		m_ConstantBuffers.push_back(constBuff_);
	}

	return true;
}

void DebugDrawer::SetConstantBuff(int index, DebugDrawer::DrawCubeCommand cube)
{
	auto vpMat = m_Camera->GetViewMat() * m_Camera->GetProjectMat();
	auto worldMat = DirectX::SimpleMath::Matrix::CreateScale(cube.size) * DirectX::SimpleMath::Matrix::CreateTranslation(cube.pos);
	auto constBuff = m_ConstantBuffers[index]->getBuffer();

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->worldMat = worldMat;
	constMap->vpMat = vpMat;
	constBuff->Unmap(0, nullptr);
}

void DebugDrawer::DrawCube(SimpleMath::Vector3 size, SimpleMath::Vector3 pos)
{
	if (m_CubeCommand.size() >= m_DrawObjectCount)return;

	DebugDrawer::DrawCubeCommand command{};
	command.size = size;
	command.pos = pos;

	m_CubeCommand.push_back(command);
}

void DebugDrawer::Draw()
{
	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	tempCommand->SetPipelineState(m_PSO.pso.Get());
	tempCommand->SetGraphicsRootSignature(m_PSO.rootSignature.Get());
	auto matIncSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	auto handle = m_ConstDescHeap->GetGPUDescriptorHandleForHeapStart();
	auto heap = m_ConstDescHeap.Get();
	tempCommand->SetDescriptorHeaps(1, &heap);

	for (int i = 0; i < m_CubeCommand.size(); i++)
	{
		SetConstantBuff(i, m_CubeCommand[i]);
		tempCommand->SetGraphicsRootDescriptorTable(0, handle);

		tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		tempCommand->IASetVertexBuffers(0, 1, &m_vbView);
		tempCommand->IASetIndexBuffer(&m_ibView);
		tempCommand->DrawIndexedInstanced(m_IndexCount, 1, 0, 0, 0);

		handle.ptr += matIncSize;
	}

	m_CubeCommand.clear();
}

