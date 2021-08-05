#include "NormalParticleAction.h"

#include "../../Utility/Random.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/Core/ShaderManager.h"

NormalParticleAction::NormalParticleAction(const std::string& particleUpdateShaderName, const std::string& particleInitShaderName)
	:ParticleAction(1.0f),m_ParticleUpdateComputeShader(particleUpdateShaderName), m_ParticleInitComputeShader(particleInitShaderName)
	, m_DestroyTime(1.0f)
{
	Init();
}

NormalParticleAction::~NormalParticleAction()
{
}

void NormalParticleAction::Update()
{
	if (!m_DummyFlag)return;

	UpdateConstanBuffer();
	m_DummyFlag = false;
}

void NormalParticleAction::SetDeathTime(float time)
{
	m_DestroyTime = time;
	m_DummyFlag = true;
}

void NormalParticleAction::SetRotateRange(const DirectX::SimpleMath::Vector3& rotate)
{
	m_RotateRange = SimpleMath::Vector4(rotate.x, rotate.y, rotate.z, 0.0f);
	m_DummyFlag = true;
}

void NormalParticleAction::SetPositionRange(const DirectX::SimpleMath::Vector3& position)
{
	m_PositionRange = SimpleMath::Vector4(position.x, position.y, position.z, 0.0f);
	m_DummyFlag = true;
}

void NormalParticleAction::SetVelocityRange(const DirectX::SimpleMath::Vector3& velocity)
{
	m_VelocityRange = SimpleMath::Vector4(velocity.x, velocity.y, velocity.z, 0.0f);
	m_DummyFlag = true;
}

void NormalParticleAction::SetScaleRange(const DirectX::SimpleMath::Vector3& scale)
{
	m_ScaleRange = SimpleMath::Vector4(scale.x, scale.y, scale.z, 0.0f);
	m_DummyFlag = true;
}

void NormalParticleAction::SetColorRange(const DirectX::SimpleMath::Color& color)
{
	m_ColorRange = color;
	m_DummyFlag = true;
}

void NormalParticleAction::SetAddColor(const DirectX::SimpleMath::Color& color)
{
	m_AddColor = color;
	m_DummyFlag = true;
}

void NormalParticleAction::SetAddRotate(const DirectX::SimpleMath::Vector3& rotate)
{
	m_AddRotate = SimpleMath::Vector4(rotate.x, rotate.y, rotate.z, 0.0f);
	m_DummyFlag = true;
}

void NormalParticleAction::SetAddVelocity(const DirectX::SimpleMath::Vector3& velocity)
{
	m_AddVelocity = SimpleMath::Vector4(velocity.x, velocity.y, velocity.z, 0.0f);
	m_DummyFlag = true;
}

void NormalParticleAction::SetAddScale(const DirectX::SimpleMath::Vector3& scale)
{
	m_AddScale = SimpleMath::Vector4(scale.x, scale.y, scale.z, 0.0f);
	m_DummyFlag = true;
}


PSO& NormalParticleAction::GetUpdatePSO()
{
	return m_ParticleUpdatePSO;
}

void NormalParticleAction::Init()
{
	CreatePSO();
	GenerateConstantBuffer();
}

void NormalParticleAction::UpdateConstanBuffer()
{
	NormalParticleActionData* constParticleParams = nullptr;
	m_UpdateParticleParamsBuffer->Map(0, nullptr, (void**)&constParticleParams);
	constParticleParams->AddColor = m_AddColor;
	constParticleParams->AddRotate = m_AddRotate;
	constParticleParams->AddVelocity = m_AddVelocity;
	constParticleParams->AddScale = m_AddScale;
	constParticleParams->LifeTimeRange = m_DestroyTime;
	constParticleParams->ColorRange = m_ColorRange;
	constParticleParams->PositionRange = m_PositionRange;
	constParticleParams->RotateRange = m_RotateRange;
	constParticleParams->ScaleRange = m_ScaleRange;
	constParticleParams->VecRange = m_VelocityRange;
	m_UpdateParticleParamsBuffer->Unmap(0, nullptr);


}

void NormalParticleAction::GenerateConstantBuffer()
{


	// パーティクルの情報更新用
	const UINT dataSize = (sizeof(NormalParticleActionData) + 0xff) & ~0xff;
	D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UpdateParticleParamsBuffer)
	);

	NormalParticleActionData* updateParamsMap = nullptr;
	m_UpdateParticleParamsBuffer->Map(0, nullptr, (void**)&updateParamsMap);
	updateParamsMap->AddColor = m_AddColor;
	updateParamsMap->AddRotate = m_AddRotate;
	updateParamsMap->AddVelocity = m_AddVelocity;
	updateParamsMap->AddScale = m_AddScale;
	updateParamsMap->LifeTimeRange = m_DestroyTime;
	updateParamsMap->ColorRange = m_ColorRange;
	updateParamsMap->PositionRange = m_PositionRange;
	updateParamsMap->RotateRange = m_RotateRange;
	updateParamsMap->ScaleRange = m_ScaleRange;
	updateParamsMap->VecRange = m_VelocityRange;

	m_UpdateParticleParamsBuffer->Unmap(0, nullptr);
}

void NormalParticleAction::CreatePSO()
{
	// ルートシグネチャーの作成
	CD3DX12_DESCRIPTOR_RANGE1 ranges[4];
	// パーティクルデータ
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読みのみ
	// パーティクルデータ
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き

	// モデル描画用頂点データ
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読み書き


	CD3DX12_ROOT_PARAMETER1 rootParameters[5];
	rootParameters[RootParam_ParticleUpdateParam].InitAsConstantBufferView(0);
	rootParameters[RootParam_SRV].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_UAV].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL); // UAV
	rootParameters[RootParam_VerticesPosition].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_EmiiterDataParam].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSigDesc;
	computeRootSigDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> errorBlob;

	D3D12SerializeVersionedRootSignature(&computeRootSigDesc, &signature, &errorBlob);
	DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0, signature->GetBufferPointer(), signature->GetBufferSize(),
		IID_PPV_ARGS(&m_ParticleUpdatePSO.rootSignature)
	);

	auto cs = ShaderManager::GetInstance().GetShader(m_ParticleUpdateComputeShader);

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = m_ParticleUpdatePSO.rootSignature.Get();
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	// パーティクル更新用のパイプライン
	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&m_ParticleUpdatePSO.pso)
	);

	// パーティクル初期化用のパイプライン
	cs = ShaderManager::GetInstance().GetShader(m_ParticleInitComputeShader);
	m_ParticleInitPSO.rootSignature = m_ParticleUpdatePSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&m_ParticleInitPSO.pso)
	);
}