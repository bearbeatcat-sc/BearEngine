#include "ParticleEmitter.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/DirectXGraphics.h"
#include "../Texture.h"
#include "../DirectX/Core/Buffer.h"
#include "../../Utility/Camera.h"
#include "../../Utility/Random.h"
#include "../../Utility/CameraManager.h"
#include "..//TextureManager.h"
#include "ParticleManager.h"
#include "NormalParticleAction.h"
#include "../DirectX/Core/Model/MeshManager.h"
#include "../../imgui/imgui.h"

ParticleEmitter::ParticleEmitter(const std::string& effectName, const std::string& actionName)
	:m_IsUpdate(true), m_DrawPrticleMode(DrawParticleMode_Normal),
	m_EffectName(effectName), m_IsMeshEmitter(false),
	m_IsVerticesMode(false), m_IsGenerted(false), m_ActionName(actionName),
	m_Pos(SimpleMath::Vector4::Zero), m_Rotate(SimpleMath::Vector4::Zero), m_Color(SimpleMath::Color(1, 1, 1, 1)),
	m_Scale(SimpleMath::Vector4::One), m_Velocity(SimpleMath::Vector4::Zero), m_DestroyFlag(false), m_BaseLifeTime(3.0f), m_TimeScale(1.0f)
	, m_IsMeshParticleMode(false), m_IsDraw(true),m_ObjectCount(120000)
{
	m_UavIndex = 0;
}

ParticleEmitter::ParticleEmitter(const std::string& effectName, const std::string& actionName, const std::string& modelName)
	:m_IsUpdate(true), m_DrawPrticleMode(DrawParticleMode_Normal),
	m_EffectName(effectName), m_IsMeshEmitter(true),
	m_IsVerticesMode(false), m_IsGenerted(false), m_ActionName(actionName),
	m_Pos(SimpleMath::Vector4::Zero), m_Rotate(SimpleMath::Vector4::Zero), m_Color(SimpleMath::Color(1, 1, 1, 1)),
	m_Scale(SimpleMath::Vector4::One), m_Velocity(SimpleMath::Vector4::Zero), m_DestroyFlag(false), m_BaseLifeTime(3.0f), m_TimeScale(1.0f),
	m_UseMeshName(modelName), m_IsMeshParticleMode(false), m_IsDraw(true), m_ObjectCount(120000)
{
	m_UavIndex = 0;

}


ParticleEmitter::~ParticleEmitter()
{
}


void ParticleEmitter::SetParticleMode(DrawParticleMode mode)
{
	m_DrawPrticleMode = mode;
}

void ParticleEmitter::SetTexture(const std::string& textureName)
{
	m_TextureName = textureName;
}



bool ParticleEmitter::Init()
{
	if (m_IsMeshEmitter)
	{
		return Init(m_UseMeshName);
	}

	if (m_IsMeshParticleMode)
	{
		return Init(MeshManager::GetInstance().GetMeshData(m_UseMeshName)->GetPositions());
	}

	m_Camera = CameraManager::GetInstance().GetMainCamera();

	GenerateVertexBuff();
	GenerateConBuff();
	GenerateTextureView();

	GenerateComputeHeap();
	GenerateComputeBuffer();



	return true;
}

bool ParticleEmitter::Init(const std::vector<XMFLOAT3>& vertices)
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();

	// 頂点情報を元にパーティクル数を指定するため
	GenerateComputeHeap();
	GenerateComputeBuffer();
	GenerateVerticesPositionBuffer(vertices);
	GenerateVertexBuff();
	GenerateConBuff();
	GenerateTextureView();



	m_IsVerticesMode = true;

	return true;
}

bool ParticleEmitter::Init(const std::string& modelName)
{
	m_Camera = CameraManager::GetInstance().GetMainCamera();

	// 使用するメッシュの名前
	m_UseMeshName = modelName;
	m_IsMeshEmitter = true;

	m_MeshData = MeshManager::GetInstance().GetMeshData(m_UseMeshName);
	m_ibView = m_MeshData->m_ibView;
	m_vbView = m_MeshData->m_vbView;

	// 頂点バッファは生成しない
	//GenerateVertexBuff();

	GenerateConBuff();
	GenerateTextureView();


	GenerateComputeHeap();
	GenerateComputeBuffer();


	return true;
}


void ParticleEmitter::Draw()
{
	DrawCall();
}



void ParticleEmitter::Destroy()
{
	m_DestroyFlag = true;
}

void ParticleEmitter::OnMeshParticleMode(bool flag, const std::string& meshName)
{
	m_IsMeshParticleMode = flag;
	m_UseMeshName = meshName;
}


bool ParticleEmitter::IsDestroyFlag()
{
	return m_DestroyFlag;
}

void ParticleEmitter::UpdateParticle(std::shared_ptr<NormalParticleAction> action, ID3D12GraphicsCommandList* commandList)
{
	if (!m_IsUpdate)return;
	if (m_DestroyFlag)return;
	if (m_DummyFlag)
	{
		UpdateEmitterBuffer();
		m_DummyFlag = false;
	}

	auto descriptorSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	int srvIndex;
	int uavIndex;


	// ここにリソースが入ってくる！
	ID3D12Resource* pUavResoruce = nullptr;

	// インデックスによって、セットするディスクリプターを変更
	if (m_UavIndex == 0)
	{
		srvIndex = SrvParticlePosVelo0;
		uavIndex = UavParticlePosVelo1;

		pUavResoruce = m_ParticleDataBuff1.Get();
	}
	else
	{
		srvIndex = SrvParticlePosVelo1;
		uavIndex = UavParticlePosVelo0;

		pUavResoruce = m_ParticleDataBuff0.Get();
	}

	auto trans = CD3DX12_RESOURCE_BARRIER::Transition(
		pUavResoruce,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);
	commandList->ResourceBarrier(1, &trans);

	auto pso = action->GetUpdatePSO();
	commandList->SetPipelineState(pso.pso.Get());
	commandList->SetComputeRootSignature(pso.rootSignature.Get());

	ID3D12DescriptorHeap* ppHaaps[] = { m_Heap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHaaps), ppHaaps);



	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), srvIndex, descriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), uavIndex, descriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), EmitterData, descriptorSize);

	commandList->SetComputeRootConstantBufferView(NormalParticleAction::RootParam_ParticleUpdateParam, action->GetBuffer()->GetGPUVirtualAddress());
	commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_SRV, srvHandle);
	commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_UAV, uavHandle);
	commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_EmiiterDataParam, cbvHandle);

	// 頂点モードならセットする（もしかしたらコスト高いかも）
	if (m_IsVerticesMode)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvVerticeDataHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), SrvVerticesPosition, descriptorSize);
		commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_VerticesPosition, srvVerticeDataHandle);
	}




	commandList->Dispatch(static_cast<int>(ceil(m_ObjectCount / 256.0f)), 1, 1);
	 trans = CD3DX12_RESOURCE_BARRIER::Transition(pUavResoruce, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &trans);

	//// リソースのインデックス切り替え
	//m_UavIndex = 1 - m_UavIndex;
}


void ParticleEmitter::UpdateNormalParticle(std::shared_ptr<NormalParticleAction> action,
	ID3D12GraphicsCommandList* commandList)
{
	InitParticle(action, commandList);
	UpdateParticle(action, commandList);
}

void ParticleEmitter::InitParticle(std::shared_ptr<ParticleAction> action, ID3D12GraphicsCommandList* commandList)
{
	if (!m_IsUpdate)return;

	auto descriptorSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	int srvIndex;
	int uavIndex;
	
	int pressuerWallUAVIndex;


	// ここにリソースが入ってくる！
	ID3D12Resource* pUavResoruce = nullptr;


	// インデックスによって、セットするディスクリプターを変更
	if (m_UavIndex == 0)
	{
		srvIndex = SrvParticlePosVelo0;
		uavIndex = UavParticlePosVelo1;

		pUavResoruce = m_ParticleDataBuff1.Get();
	}
	else
	{
		srvIndex = SrvParticlePosVelo1;
		uavIndex = UavParticlePosVelo0;

		pUavResoruce = m_ParticleDataBuff0.Get();
	}


	// 多分、読み書き可にする
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		pUavResoruce,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);
	commandList->ResourceBarrier(1, &barrier);


	auto pso = action->GetInitPSO();
	commandList->SetPipelineState(pso.pso.Get());
	commandList->SetComputeRootSignature(pso.rootSignature.Get());

	ID3D12DescriptorHeap* ppHaaps[] = { m_Heap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHaaps), ppHaaps);



	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), srvIndex, descriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), uavIndex, descriptorSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), EmitterData, descriptorSize);

	commandList->SetComputeRootConstantBufferView(NormalParticleAction::RootParam_ParticleUpdateParam, action->GetBuffer()->GetGPUVirtualAddress());
	commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_SRV, srvHandle);
	commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_UAV, uavHandle);
	commandList->SetComputeRootDescriptorTable(NormalParticleAction::RootParam_EmiiterDataParam, cbvHandle);
	commandList->Dispatch(m_ObjectCount / 256, 1, 1);

	auto trans = CD3DX12_RESOURCE_BARRIER::Transition(pUavResoruce, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &trans);

	// リソースのインデックス切り替え
	m_UavIndex = 1 - m_UavIndex;

	//m_IsGenerted = true;
}

void ParticleEmitter::SetActionName(const std::string& actionName)
{
	m_ActionName = actionName;
}

void ParticleEmitter::SetTimeScale(float timeScale)
{
	m_TimeScale = timeScale;
}

const std::string& ParticleEmitter::GetActionName()
{
	return m_ActionName;
}

int ParticleEmitter::GetParticleCount()
{
	return m_ObjectCount;
}

bool ParticleEmitter::GetDrawFlag()
{
	return m_IsDraw;
}

bool ParticleEmitter::GetUpdateFlag()
{
	return m_IsUpdate;
}

void ParticleEmitter::SetDrawFlag(bool flag)
{
	m_IsDraw = flag;
}

void ParticleEmitter::SetUpdateFlag(bool flag)
{
	m_IsUpdate = flag;
}

void ParticleEmitter::SetPosition(const SimpleMath::Vector3& pos)
{
	m_Pos = SimpleMath::Vector4(pos.x, pos.y, pos.z, 0.0f);
	m_DummyFlag = true;
}

void ParticleEmitter::SetRotate(const SimpleMath::Vector3& rotate)
{
	m_Rotate = SimpleMath::Vector4(rotate.x, rotate.y, rotate.z, 0.0f);
	m_DummyFlag = true;
}

void ParticleEmitter::SetColor(const SimpleMath::Color& color)
{
	m_Color = color;
	m_DummyFlag = true;
}

void ParticleEmitter::SetScale(const SimpleMath::Vector3& scale)
{
	m_Scale = SimpleMath::Vector4(scale.x, scale.y, scale.z, 0.0f);
	m_DummyFlag = true;
}

void ParticleEmitter::SetVelocity(const SimpleMath::Vector3& velocity)
{
	m_Velocity = SimpleMath::Vector4(velocity.x, velocity.y, velocity.z, 0.0f);
	m_DummyFlag = true;
}

void ParticleEmitter::SetLifeTime(float lifeTime)
{
	m_BaseLifeTime = lifeTime;
	m_DummyFlag = true;
}

void ParticleEmitter::SetParticleCount(int count)
{
	m_ObjectCount = count;
	m_DummyFlag = true;
}

void ParticleEmitter::SetHandle(ParticleManager::EmitterDescHandles* handle)
{
	m_pHandle = handle;
}

void ParticleEmitter::SetEffectName(const std::string& effectName)
{
	m_EffectName = effectName;
}

std::shared_ptr<Buffer> ParticleEmitter::GetConstantBuffer()
{
	return m_ConstantBuffer;
}

ComPtr<ID3D12Resource> ParticleEmitter::GetTextureBuffer()
{
	return m_TextureBuffer;
}

ParticleManager::EmitterDescHandles* ParticleEmitter::GetHandle()
{
	return m_pHandle;
}

const std::string& ParticleEmitter::GetEffectName()
{
	return m_EffectName;
}


bool ParticleEmitter::DrawCall()
{

	UpdateConstantBuffer();

	auto descriptorSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	//auto heap = m_BasicDescHeap.Get();
	ID3D12DescriptorHeap* ppHeaps[] = { m_Heap.Get() };

	int srvIndex = SrvParticlePosVelo1;

	if (m_UavIndex == 0)
	{
		srvIndex = SrvParticlePosVelo0;
	}

	// 仮
	//auto pso = EffectManager::GetInstance().GetEffect(m_EffectName)->GetPSO();


	tempCommand->IASetVertexBuffers(0, 1, &m_vbView);

	if (m_IsMeshEmitter)
	{
		tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		tempCommand->IASetIndexBuffer(&m_ibView);
	}
	else
	{
		tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	}



	// 行列のセット
	tempCommand->SetGraphicsRootConstantBufferView(0, m_ConstantBuffer->getBuffer()->GetGPUVirtualAddress());
	// テクスチャのセット
	tempCommand->SetGraphicsRootDescriptorTable(2, m_pHandle->m_GPUDescHandle);
	// コンピュートシェーダーのディスクリプタ
	tempCommand->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_Heap->GetGPUDescriptorHandleForHeapStart(), srvIndex, descriptorSize);
	tempCommand->SetGraphicsRootDescriptorTable(NormalParticleAction::RootParam_SRV, srvHandle);

	if (m_IsMeshEmitter)
	{
		unsigned int offset = 0;

		for (auto& material : m_MeshData->GetDrawMaterialDatas())
		{
			tempCommand->DrawIndexedInstanced(material.indexCount, m_ObjectCount, offset, 0, 0);
			offset += material.indexCount;
		}

		return true;
	}

	tempCommand->DrawInstanced(m_ObjectCount, 1, 0, 0);
	return true;
}

void ParticleEmitter::UpdateConstantBuffer()
{
	XMMATRIX mat = XMMatrixIdentity();
	auto vpMat = m_Camera->GetViewMat() * m_Camera->GetProjectMat();
	auto billmat = XMMatrixIdentity();
	auto viewMat = m_Camera->GetViewMat();
	auto projMat = m_Camera->GetProjectMat();

	switch (m_DrawPrticleMode)
	{
	case DrawParticleMode_Normal:
		mat = vpMat;
		break;

	case DrawParticleMode_BillBoard:
		mat = vpMat;
		billmat = m_Camera->GetBillBoardMat();
		break;

	case DrawParticleMode_BillBoard_Y:
		mat = vpMat;
		billmat = m_Camera->GetYAxisBillBoardMat();
		break;

	}




	ConstBufferData* constMap = nullptr;
	m_ConstantBuffer->getBuffer()->Map(0, nullptr, (void**)&constMap);
	constMap->billboard_matrix = billmat;
	constMap->v_matrix = viewMat;
	constMap->p_matrix = projMat;
	constMap->vp_matrix = vpMat;
	constMap->nearFar = SimpleMath::Vector3(m_Camera->GetNear(), m_Camera->GetFar(), 0.0f);
	m_ConstantBuffer->getBuffer()->Unmap(0, nullptr);
}

void ParticleEmitter::UpdateEmitterBuffer()
{
	// パーティクルの更新情報
	ParticleEmiiterData* emitterParamsMap = nullptr;
	m_EmmiterParamsBuffer->Map(0, nullptr, (void**)&emitterParamsMap);
	emitterParamsMap->RandomSeed = Random::GetRandom(0.0f, 1.0f);
	emitterParamsMap->BasePosition = m_Pos;
	emitterParamsMap->BaseRotate = m_Rotate;
	emitterParamsMap->BaseColor = m_Color;
	emitterParamsMap->BaseScale = m_Scale;
	emitterParamsMap->BaseVelocity = m_Velocity;
	emitterParamsMap->MaxParticleCount = m_ObjectCount;
	emitterParamsMap->BaseLifeTime = m_BaseLifeTime;
	emitterParamsMap->TimeScale = m_TimeScale;
	m_EmmiterParamsBuffer->Unmap(0, nullptr);
}

void ParticleEmitter::GenerateVertexBuff()
{
	std::vector<Vertex> vertices;
	vertices.resize(m_ObjectCount);

	//for (UINT i = 0; i < m_ObjectCount; i++)
	//{
	//	vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	//}

	m_VertexBuffer = std::make_shared<Buffer>();
	UINT buffSize = sizeof(Vertex) * m_ObjectCount;
	ID3D12Resource* vertBuff = m_VertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	Vertex* vertMap = nullptr;

	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return;
	}

	for (int i = 0; i < m_ObjectCount; i++)
	{
		vertMap[i] = vertices[i];
	}

	vertBuff->Unmap(0, nullptr);

	m_vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = static_cast<UINT>(buffSize);
	m_vbView.StrideInBytes = sizeof(Vertex);
}

void ParticleEmitter::GenerateConBuff()
{
	// 行列用
	m_ConstantBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(ConstBufferData) + 0xff) & ~0xff;
	ID3D12Resource* constBuff = m_ConstantBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constBuff->Unmap(0, nullptr);


}

void ParticleEmitter::GenerateComputeBuffer()
{
	GenerateCBParticleData();
}


void ParticleEmitter::GenerateCBParticleData()
{

	// エミッター用のデータを生成
	UINT dataSize = (sizeof(ParticleEmiiterData) + 0xff) & ~0xff;
	D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_EmmiterParamsBuffer)
	);

	ParticleEmiiterData* emitterParamsMap = nullptr;
	m_EmmiterParamsBuffer->Map(0, nullptr, (void**)&emitterParamsMap);
	emitterParamsMap->RandomSeed = Random::GetRandom(0.0f, 1.0f);
	emitterParamsMap->BasePosition = m_Pos;
	emitterParamsMap->BaseRotate = m_Rotate;
	emitterParamsMap->BaseColor = m_Color;
	emitterParamsMap->BaseScale = m_Scale;
	emitterParamsMap->BaseVelocity = m_Velocity;
	emitterParamsMap->MaxParticleCount = m_ObjectCount;
	emitterParamsMap->BaseLifeTime = m_BaseLifeTime;
	emitterParamsMap->TimeScale = m_TimeScale;
	m_EmmiterParamsBuffer->Unmap(0, nullptr);


	
	std::vector<ParticleData> particleDatas;
	particleDatas.resize(m_ObjectCount);

	// コンピュートシェーダー用のバッファの生成
	dataSize = (sizeof(ParticleData) * m_ObjectCount + 0xff) & ~0xff;
	D3D12_HEAP_PROPERTIES defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

	// 転送用と定数バッファを2つずつ作成する
	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_ParticleDataBuff0)
	);

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_ParticleDataBuff1)
	);

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadConstBuff0)
	);

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadConstBuff1)
	);

	D3D12_SUBRESOURCE_DATA particleData = {};
	particleData.pData = reinterpret_cast<UINT8*>(&particleDatas[0]);
	particleData.RowPitch = dataSize;
	particleData.SlicePitch = particleData.RowPitch;

	auto commandList = DirectXGraphics::GetInstance().GetCommandList();
	// リソースの転送
	UpdateSubresources<1>(commandList, m_ParticleDataBuff0.Get(), m_UploadConstBuff0.Get(), 0, 0, 1, &particleData);
	UpdateSubresources<1>(commandList, m_ParticleDataBuff1.Get(), m_UploadConstBuff1.Get(), 0, 0, 1, &particleData);

	auto trans = CD3DX12_RESOURCE_BARRIER::Transition(m_ParticleDataBuff0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// リソースをコピー状態から、ピクセルシェーダー以外のシェーダーで扱うに変更
	commandList->ResourceBarrier(1, &trans);

	trans = CD3DX12_RESOURCE_BARRIER::Transition(m_ParticleDataBuff1.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &trans);

	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_ObjectCount;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleData);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// SRVを生成
	auto descriptorSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle0(m_Heap->GetCPUDescriptorHandleForHeapStart(), SrvParticlePosVelo0, descriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle1(m_Heap->GetCPUDescriptorHandleForHeapStart(), SrvParticlePosVelo1, descriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(m_Heap->GetCPUDescriptorHandleForHeapStart(), EmitterData, descriptorSize);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(m_ParticleDataBuff0.Get(), &srvDesc, srvHandle0);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(m_ParticleDataBuff1.Get(), &srvDesc, srvHandle1);

	// エミッターデータ用のビューを生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = m_EmmiterParamsBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)m_EmmiterParamsBuffer->GetDesc().Width;

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		cbvHandle);


	// UAV（読み書き可のやつ）
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_ObjectCount; // 要素数
	uavDesc.Buffer.StructureByteStride = sizeof(ParticleData);
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle0(m_Heap->GetCPUDescriptorHandleForHeapStart(), UavParticlePosVelo0, descriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle1(m_Heap->GetCPUDescriptorHandleForHeapStart(), UavParticlePosVelo1, descriptorSize);
	DirectXDevice::GetInstance().GetDevice()->CreateUnorderedAccessView(m_ParticleDataBuff0.Get(), nullptr, &uavDesc, uavHandle0);
	DirectXDevice::GetInstance().GetDevice()->CreateUnorderedAccessView(m_ParticleDataBuff1.Get(), nullptr, &uavDesc, uavHandle1);

	particleDatas.clear();
	particleDatas.shrink_to_fit();
}


void ParticleEmitter::GenerateComputeHeap()
{
	// 2つのバッファ用のヒープを作る
	D3D12_DESCRIPTOR_HEAP_DESC srvUavHeapDesc = {};
	srvUavHeapDesc.NumDescriptors = DescriptorCount;
	srvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&srvUavHeapDesc,
		IID_PPV_ARGS(&m_Heap)
	);


}


void ParticleEmitter::GenerateTexture()
{
	//auto handle = m_BasicDescHeap->GetCPUDescriptorHandleForHeapStart();
	//handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	std::shared_ptr<Texture> texuture;

	// テクスチャバッファの作成
	D3D12_HEAP_PROPERTIES texHeapProp{};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	return;
}

ParticleEmitter::DrawParticleMode ParticleEmitter::GetDrawParticleMode()
{
	return m_DrawPrticleMode;
}

void ParticleEmitter::GenerateTextureView()
{
	// シェーダーリソースビューの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srVDesc = {};
	srVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srVDesc.Texture2D.MipLevels = 1;


	// メッシュエミッターの場合、メッシュのテクスチャデータを使う
	if (m_IsMeshEmitter)
	{
		m_TextureBuffer = m_MeshData->GetFirstTextureBuffer();
	}
	else if (m_TextureName != "")
	{
		m_TextureBuffer = TextureManager::GetInstance().GetTexture(m_TextureName);
	}
	else
	{
		m_TextureBuffer = TextureManager::GetInstance().GetWhiteTex();
	}

	srVDesc.Format = m_TextureBuffer->GetDesc().Format;
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_TextureBuffer.Get(),
		&srVDesc,
		m_pHandle->m_CPUDescHandle);
}

void ParticleEmitter::GenerateVerticesPositionBuffer(const std::vector<XMFLOAT3>& vertices)
{
	// ここあとで、モデル頂点を読み込む
	std::vector<XMFLOAT3> verticesDatas;


	// 頂点数 * つなぐポイント数
	m_VerticesCount = vertices.size();
	int count = m_VerticesCount;
	m_ObjectCount = count;

	verticesDatas.resize(count);
	std::copy(vertices.begin(), vertices.end(), verticesDatas.begin());

	//int verticesCount = verticesDatas.size();
	int verticesCount = m_ObjectCount; // 頂点数よりもパーティクル数が多い場合を想定して

	// コンピュートシェーダー用のバッファの生成
	const UINT dataSize = (sizeof(XMFLOAT3) * verticesCount + 0xff) & ~0xff;
	D3D12_HEAP_PROPERTIES defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

	// 転送用と定数バッファを作成
	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_VerticesDataBuff)
	);

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadVerticesDataBuff)
	);


	D3D12_SUBRESOURCE_DATA verticesData = {};
	verticesData.pData = reinterpret_cast<UINT8*>(&verticesDatas[0]);
	verticesData.RowPitch = dataSize;
	verticesData.SlicePitch = verticesData.RowPitch;

	auto commandList = DirectXGraphics::GetInstance().GetCommandList();
	// リソースの転送
	UpdateSubresources<1>(commandList, m_VerticesDataBuff.Get(), m_UploadVerticesDataBuff.Get(), 0, 0, 1, &verticesData);

	auto trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VerticesDataBuff.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// リソースをコピー状態から、ピクセルシェーダー以外のシェーダーで扱うに変更
	commandList->ResourceBarrier(1, &trans);

	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = m_ObjectCount;
	srvDesc.Buffer.StructureByteStride = sizeof(XMFLOAT3);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// SRVを生成
	auto descriptorSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_Heap->GetCPUDescriptorHandleForHeapStart(), SrvVerticesPosition, descriptorSize);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(m_VerticesDataBuff.Get(), &srvDesc, srvHandle);


}

