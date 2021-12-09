#include "DebugDrawer.h"

#include "Utility/CameraManager.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "GeomtryGenerater.h"
#include "MeshManager.h"
#include "../Buffer.h"
#include "Utility/Camera.h"

DebugDrawer::DebugDrawer()
{
}

DebugDrawer::~DebugDrawer()
{
	_constantBuffers.clear();
}

bool DebugDrawer::Init(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath)
{



	GenerateCubeData();
	GenerateSphereData();
	GeneratePipeline(vertexShaderPath, pixelShaderPath);

	return true;
}

bool DebugDrawer::GenerateVertexBuffer(std::vector<XMFLOAT3>& vertices, ComPtr<ID3D12Resource> vertexBuffer, D3D12_VERTEX_BUFFER_VIEW& vertex_buffer_view)
{


	auto vertBuff = vertexBuffer;

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


	vertex_buffer_view.BufferLocation = vertBuff->GetGPUVirtualAddress();
	vertex_buffer_view.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertex_buffer_view.StrideInBytes = sizeof(vertices[0]);

	return true;
}

bool DebugDrawer::GenerateIndexBuffer(const std::vector<UINT>& indices, ComPtr<ID3D12Resource> indexBuffer, const int indexCount, D3D12_INDEX_BUFFER_VIEW& index_buffer_view)
{

	auto indexBuff = indexBuffer;

	unsigned short* indexMap = nullptr;

	indexBuff->Map(0, nullptr, (void**)&indexMap);

	std::copy(indices.begin(), indices.end(), indexMap);


	indexBuff->Unmap(0, nullptr);


	index_buffer_view.BufferLocation = indexBuff->GetGPUVirtualAddress();
	index_buffer_view.Format = DXGI_FORMAT_R16_UINT;
	index_buffer_view.SizeInBytes = sizeof(indices[0]) * indices.size();

	return true;
}

void DebugDrawer::GenerateCubeData()
{
	std::vector<XMFLOAT3> vertices;
	std::vector<UINT> indicies;

	GeometryGenerator::GenerateCubeDatas(vertices, indicies, DirectX::SimpleMath::Vector3(1, 1, 1));
	_cubeIndexCount = indicies.size();

	// VertexBuffer
	_cubeVertexBuffer = std::make_shared<Buffer>();
	int size = sizeof(XMFLOAT3) * vertices.size();
	_cubeVertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);


	// IndexBuffer
	_cubeIndexBuffer = std::make_shared<Buffer>();
	size = sizeof(int) * _cubeIndexCount;
	_cubeIndexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, size, D3D12_RESOURCE_STATE_GENERIC_READ);

	if (!GenerateVertexBuffer(vertices, _cubeVertexBuffer->getBuffer(), _cubeVertexBufferView))
	{
		return;
	}

	if (!GenerateIndexBuffer(indicies, _cubeIndexBuffer->getBuffer(), _cubeIndexCount, _cubeIndexBufferView))
	{
		return;
	}

}

void DebugDrawer::GenerateSphereData()
{
	auto meshData = MeshManager::GetInstance().GetSpehereMeshData(12);

	_sphereVertexBuffer = meshData->GetVertexBuffer();
	_sphereIndexBuffer = meshData->GetIndexBuffer();

	meshData->CopyIndexBufferView(_sphereIndexBufferView);
	meshData->CopyVertexBufferView(_sphereVertexBufferView);

	_sphereIndexCount = _sphereIndexBufferView.SizeInBytes / sizeof(UINT);
}

void DebugDrawer::GeneratePipeline(const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath)
{
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

	if (!PSOManager::GetInstance().CreatePSO(_pso, vertexShaderPath, pixelShaderPath, &inputLayout[0], _countof(inputLayout),
		rasterizeDesc, blendDesc, rootSignatureDesc))
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

	auto handle = _constDescHeap->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < _constantBuffers.size(); i++)
	{
		auto constBuff = _constantBuffers[i]->getBuffer();

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
		IID_PPV_ARGS(&_constDescHeap))))
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

		_constantBuffers.push_back(constBuff_);
	}

	return true;
}

void DebugDrawer::SetCubeConstantBuffer(int index, DebugDrawer::DrawCubeCommand cube)
{
	auto vpMat = _camera->GetViewMat() * _camera->GetProjectMat();
	auto worldMat = DirectX::SimpleMath::Matrix::CreateScale(cube.size) * cube.rotation * DirectX::SimpleMath::Matrix::CreateTranslation(cube.pos);
	auto constBuff = _constantBuffers[index]->getBuffer();

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->worldMat = worldMat;
	constMap->vpMat = vpMat;
	constBuff->Unmap(0, nullptr);
}

void DebugDrawer::SetSphereConstantBuffer(int index, DebugDrawer::DrawSpehereCommand sphere)
{
	auto vpMat = _camera->GetViewMat() * _camera->GetProjectMat();
	auto worldMat = DirectX::SimpleMath::Matrix::CreateScale(sphere.radius) * DirectX::SimpleMath::Matrix::CreateTranslation(sphere.pos);
	auto constBuff = _constantBuffers[index]->getBuffer();

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->worldMat = worldMat;
	constMap->vpMat = vpMat;
	constBuff->Unmap(0, nullptr);
}


void DebugDrawer::DrawCube(const SimpleMath::Vector3 size, const SimpleMath::Vector3 pos, const SimpleMath::Matrix rotateMat)
{
	if (!IsAddCommand())return;

	DebugDrawer::DrawCubeCommand command{};
	command.size = size;
	command.pos = pos;
	command.rotation = rotateMat;

	_draw_cube_commands.push_back(command);
}

bool DebugDrawer::IsAddCommand()
{
	return _draw_cube_commands.size() + _draw_sphere_commands.size() <= m_DrawObjectCount;
}

void DebugDrawer::DrawCube(const SimpleMath::Vector3 size, const SimpleMath::Vector3 pos, const SimpleMath::Quaternion rotate_qu)
{
	if (!IsAddCommand())return;

	DebugDrawer::DrawCubeCommand command{};
	command.size = size;
	command.pos = pos;
	command.rotation = SimpleMath::Matrix::CreateFromQuaternion(rotate_qu);

	_draw_cube_commands.push_back(command);
}

void DebugDrawer::DrawSphere(const float radius, const SimpleMath::Vector3 pos)
{
	if (!IsAddCommand())return;

	DrawSpehereCommand command{};
	command.radius = radius;
	command.pos = pos;

	_draw_sphere_commands.push_back(command);
}

const int DebugDrawer::RenderCube(ID3D12GraphicsCommandList* tempCommand, UINT matIncSize, D3D12_GPU_DESCRIPTOR_HANDLE& handle, const int offset)
{
	for (int i = 0; i < _draw_cube_commands.size(); i++)
	{
		SetCubeConstantBuffer(i + offset, _draw_cube_commands[i]);
		tempCommand->SetGraphicsRootDescriptorTable(0, handle);

		tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		tempCommand->IASetVertexBuffers(0, 1, &_cubeVertexBufferView);
		tempCommand->IASetIndexBuffer(&_cubeIndexBufferView);
		tempCommand->DrawIndexedInstanced(_cubeIndexCount, 1, 0, 0, 0);

		handle.ptr += matIncSize;
	}

	return _draw_cube_commands.size();
}

const int DebugDrawer::RenderSphere(ID3D12GraphicsCommandList* tempCommand, UINT matIncSize, D3D12_GPU_DESCRIPTOR_HANDLE& handle, const int offset)
{
	for (int i = 0; i < _draw_sphere_commands.size(); i++)
	{
		SetSphereConstantBuffer(i + offset, _draw_sphere_commands[i]);
		tempCommand->SetGraphicsRootDescriptorTable(0, handle);

		tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		tempCommand->IASetVertexBuffers(0, 1, &_sphereVertexBufferView);
		tempCommand->IASetIndexBuffer(&_sphereIndexBufferView);
		tempCommand->DrawIndexedInstanced(_sphereIndexCount, 1, 0, 0, 0);

		handle.ptr += matIncSize;
	}

	return _draw_sphere_commands.size();
}


void DebugDrawer::Draw()
{
	_camera = CameraManager::GetInstance().GetMainCamera();

	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	tempCommand->SetPipelineState(_pso.pso.Get());
	tempCommand->SetGraphicsRootSignature(_pso.rootSignature.Get());
	auto matIncSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	auto handle = _constDescHeap->GetGPUDescriptorHandleForHeapStart();
	auto heap = _constDescHeap.Get();
	tempCommand->SetDescriptorHeaps(1, &heap);

	int offset = RenderCube(tempCommand, matIncSize, handle, 0);
	RenderSphere(tempCommand, matIncSize, handle, offset);

	_draw_cube_commands.clear();
	_draw_sphere_commands.clear();
}

