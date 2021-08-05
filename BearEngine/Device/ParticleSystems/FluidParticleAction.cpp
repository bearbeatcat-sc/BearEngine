#include "FluidParticleAction.h"

#include "../../imgui/imgui.h"
#include "../../Utility/Random.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/Core/ShaderManager.h"
#include "../../Utility/Math/MathUtility.h"
#include "../../Utility/Time.h"
#include "../../Utility/Timer.h"
#include "../DirectX/DirectXInput.h"


FluidParticleAction::FluidParticleAction(const std::string& particleUpdateShaderName, const std::string& particleInitShaderName)
	:ParticleAction(1.0f),m_ParticleUpdateComputeShader(particleUpdateShaderName), m_ParticleInitComputeShader(particleInitShaderName)
{
	Init();


}

FluidParticleAction::~FluidParticleAction()
{

}

void FluidParticleAction::Update()
{
	//if (!m_DummyFlag)return;



	//float _gravity[3]{ Gravity.x,Gravity.y,Gravity.z };

	//if (mIsControllMode)
	//{
	//	float xValue = DirectXInput::GetInstance().GetGamePadValue(GAMEPAD_ThubStick_LX);
	//	float yValue = DirectXInput::GetInstance().GetGamePadValue(GAMEPAD_ThubStick_LY);
	//	float zValue = DirectXInput::GetInstance().GetGamePadValue(GAMEPAD_ThubStick_RY);

	//	// デッドゾーン
	//	if (xValue <= 0.1f && xValue >= -0.1f) { xValue = 0.0f; }
	//	if (yValue <= 0.1f && yValue >= -0.1f) { yValue = 0.0f; }
	//	if (zValue <= 0.1f && zValue >= -0.1f) { zValue = 0.0f; }

	//	_gravity[0] = xValue * ValuePower;
	//	_gravity[1] = yValue * ValuePower;
	//	_gravity[2] = zValue * ValuePower;

	//	Gravity = SimpleMath::Vector4(_gravity[0], _gravity[1], _gravity[2], 0);
	//}

	//ImGui::Begin("FluidComputeParameter", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	////ImGui::Checkbox("IsControllMode", &mIsControllMode);
	////ImGui::DragFloat("Smoothlen", &Smoothlen, 0.01f);
	////ImGui::DragFloat("MaxAllowableTimeStep", &MaxAllowableTimeStep, 0.01f);
	////ImGui::DragFloat("RestDensity", &RestDensity, 0.01f);
	//ImGui::DragFloat3("Gravity", _gravity, 0.01f);
	//Gravity = SimpleMath::Vector4(_gravity[0], _gravity[1], _gravity[2], 0);
	//ImGui::End();


	UpdateConstanBuffer();
	m_DummyFlag = false;
}

void FluidParticleAction::SetRotateRange(const DirectX::SimpleMath::Vector3& rotate)
{
	m_RotateRange = SimpleMath::Vector4(rotate.x, rotate.y, rotate.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetPositionRange(const DirectX::SimpleMath::Vector3& position)
{
	m_PositionRange = SimpleMath::Vector4(position.x, position.y, position.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetVelocityRange(const DirectX::SimpleMath::Vector3& velocity)
{
	m_VelocityRange = SimpleMath::Vector4(velocity.x, velocity.y, velocity.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetScaleRange(const DirectX::SimpleMath::Vector3& scale)
{
	m_ScaleRange = SimpleMath::Vector4(scale.x, scale.y, scale.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetColorRange(const DirectX::SimpleMath::Color& color)
{
	m_ColorRange = color;
	m_DummyFlag = true;
}

void FluidParticleAction::SetAddColor(const DirectX::SimpleMath::Color& color)
{
	m_AddColor = color;
	m_DummyFlag = true;
}

void FluidParticleAction::SetAddRotate(const DirectX::SimpleMath::Vector3& rotate)
{
	m_AddRotate = SimpleMath::Vector4(rotate.x, rotate.y, rotate.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetAddVelocity(const DirectX::SimpleMath::Vector3& velocity)
{
	m_AddVelocity = SimpleMath::Vector4(velocity.x, velocity.y, velocity.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetAddScale(const DirectX::SimpleMath::Vector3& scale)
{
	m_AddScale = SimpleMath::Vector4(scale.x, scale.y, scale.z, 0.0f);
	m_DummyFlag = true;
}

void FluidParticleAction::SetWallCount(int count)
{
	wallCount = count;
	m_DummyFlag = true;
}

PSO& FluidParticleAction::GetForcePSO()
{
	return mParticleForcePSO;
}

PSO& FluidParticleAction::GetPressurePSO()
{
	return mParticlePressurePSO;
}

PSO& FluidParticleAction::GetCollsionPSO()
{
	return mParticleCollisionPSO;
}

PSO& FluidParticleAction::GetPressureGradientPSO()
{
	return mParticlePressureGradientPSO;
}


PSO& FluidParticleAction::GetIntegratePSO()
{
	return mParticleIntegratePSO;
}

PSO& FluidParticleAction::GetMoveParticlePSO()
{
	return mParticleMoveParticlePSO;
}


//PSO& FluidParticleAction::GetInitPSO()
//{
//	return m_ParticleInitPSO;
//}
//
//PSO& FluidParticleAction::GetUpdatePSO()
//{
//	return m_ParticleUpdatePSO;
//}

ID3D12Resource* FluidParticleAction::GetBuffer()
{
	return m_UpdateParticleParamsBuffer.Get();
}

void FluidParticleAction::Init()
{
	CreatePSO();
	GenerateConstantBuffer();
}

float FluidParticleAction::Distance2(SimpleMath::Vector3 v1, SimpleMath::Vector3 v2)
{
	SimpleMath::Vector3 v = v2 - v1;
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

float FluidParticleAction::Weight(float dist, float re)
{
	return dist >= re ? 0.0f : (re / dist) - 1.0f;
}

void FluidParticleAction::CalcNZeroAndLambda()
{
	n0ForNumberDensity = 0.0f;
	n0ForGradient = 0.0f;
	n0ForLaplacian = 0.0f;
	lambda = 0.0f;

	for (int iX = -4; iX < 5; ++iX)
	{
		for (int iY = -4; iY < 5; ++iY)
		{
			for (int iZ = -4; iZ < 5; ++iZ)
			{
				if (((iX == 0) && (iY == 0)) && (iZ == 0))
					continue;
				float distance2 = Distance2(SimpleMath::Vector3(0, 0, 0), SimpleMath::Vector3(iX, iY, iZ) * PARTICLE_DISTANCE);
				float dist = sqrt(distance2);
				n0ForNumberDensity += Weight(dist, radiusForNumberDensity);
				n0ForGradient += Weight(dist, radiusForGradient);
				n0ForLaplacian += Weight(dist, radiusForLaplacian);
				lambda += distance2 * Weight(dist, radiusForLaplacian);
			}
		}
	}
	lambda = lambda / n0ForLaplacian;
}

void FluidParticleAction::CalcConstantParameter()
{
	radiusForNumberDensity = RADIUS_FOR_NUMBER_DENSITY;
	radiusForGradient = RADIUS_FOR_GRADIENT;
	radiusForLaplacian = RADIUS_FOR_LAPLACIAN;
	radiusForNumberDensity2 = radiusForNumberDensity * radiusForNumberDensity;
	radiusForGradient2 = radiusForGradient * radiusForGradient;
	radiusForLaplacian2 = radiusForLaplacian * radiusForLaplacian;
	CalcNZeroAndLambda();
	fluidDensity = FLUID_DENSITY;
	collisionDistance = COLLISION_DISTANCE;
	collisionDistance2 = collisionDistance * collisionDistance;
}



void FluidParticleAction::UpdateConstanBuffer()
{
	
	GPUParticleActionData* constParticleParams = nullptr;
	m_UpdateParticleParamsBuffer->Map(0, nullptr, (void**)&constParticleParams);
	//constParticleParams->timeStep = MathUtility::Min(MaxAllowableTimeStep, Time::DeltaTime);
	constParticleParams->timeStep = 0.04f;
	//constParticleParams->domainSphereRadius = 3.0f;
	//constParticleParams->fluidDensity = fluidDensity;
	//constParticleParams->collisionDistance2 = collisionDistance2;
	//constParticleParams->collisionDistance = collisionDistance;
	//constParticleParams->lambda = lambda;
	//constParticleParams->n0ForGradient = n0ForGradient;
	//constParticleParams->n0ForLaplacian = n0ForLaplacian;
	//constParticleParams->n0ForNumberDensity = n0ForNumberDensity;
	//constParticleParams->radiusForGradient = radiusForGradient;
	//constParticleParams->radiusForGradient2 = radiusForGradient2;
	//constParticleParams->radiusForLaplacian = radiusForLaplacian;
	//constParticleParams->radiusForLaplacian2 = radiusForLaplacian2;
	//constParticleParams->radiusForNumberDensity = radiusForNumberDensity;
	//constParticleParams->radiusForNumberDensity2 = radiusForNumberDensity2;
	//constParticleParams->maxVelocity = 0.50f;
	constParticleParams->gravity = Gravity;
	constParticleParams->wallCount = wallCount;

	m_UpdateParticleParamsBuffer->Unmap(0, nullptr);


}

void FluidParticleAction::GenerateConstantBuffer()
{


	// パーティクルの情報更新用
	const UINT dataSize = (sizeof(GPUParticleActionData) + 0xff) & ~0xff;
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

	CalcConstantParameter();
	
	GPUParticleActionData* updateParamsMap = nullptr;
	m_UpdateParticleParamsBuffer->Map(0, nullptr, (void**)&updateParamsMap);
	updateParamsMap->lambda = lambda;
	updateParamsMap->n0ForGradient = n0ForGradient;
	updateParamsMap->n0ForLaplacian = n0ForLaplacian;
	updateParamsMap->n0ForNumberDensity = n0ForNumberDensity;
	updateParamsMap->radiusForGradient = radiusForGradient;
	updateParamsMap->radiusForLaplacian = radiusForLaplacian;
	updateParamsMap->radiusForNumberDensity = radiusForNumberDensity;
	updateParamsMap->maxVelocity = 0.50f;
	updateParamsMap->DIM = DIM;
	updateParamsMap->PARTICLE_DISTANCE = PARTICLE_DISTANCE;
	updateParamsMap->DT = DT;
	updateParamsMap->COEFFICIENT_OF_RESTITUTION = COEFFICIENT_OF_RESTITUTION;
	updateParamsMap->KINEMATIC_VISCOSITY = KINEMATIC_VISCOSITY;
	updateParamsMap->FLUID_DENSITY = FLUID_DENSITY;
	updateParamsMap->wallCount = wallCount;
	m_UpdateParticleParamsBuffer->Unmap(0, nullptr);
}

void FluidParticleAction::CreatePSO()
{
	// ルートシグネチャーの作成
	CD3DX12_DESCRIPTOR_RANGE1 ranges[10];
	// パーティクルデータ
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読みのみ
	// パーティクルデータ
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き

	// モデル描画用頂点データ
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読み書き

	// 流体用Density
	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き
	ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読み書き

	// 流体用Forces
	ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き
	ranges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読み書き

		// 流体用Wall
	ranges[8].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE); // 読み書き
	ranges[9].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE); // 読み書き


	CD3DX12_ROOT_PARAMETER1 rootParameters[11];
	rootParameters[RootParam_ParticleUpdateParam].InitAsConstantBufferView(0);
	rootParameters[RootParam_SRV].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_UAV].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL); // UAV
	rootParameters[RootParam_DensitySRV].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_DensityUAV].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_ALL); // UAV
	rootParameters[RootParam_ForcesSRV].InitAsDescriptorTable(1, &ranges[6], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_ForcesUAV].InitAsDescriptorTable(1, &ranges[7], D3D12_SHADER_VISIBILITY_ALL); // UAV
	rootParameters[RootParam_WallPalamSRV].InitAsDescriptorTable(1, &ranges[8], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_WallPalamUAV].InitAsDescriptorTable(1, &ranges[9], D3D12_SHADER_VISIBILITY_ALL); // UAV
	rootParameters[RootParam_VerticesPosition].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL); // SRV
	rootParameters[RootParam_EmiiterDataParam].InitAsDescriptorTable(1, &ranges[3], D3D12_SHADER_VISIBILITY_ALL);



	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSigDesc;
	computeRootSigDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> errorBlob;

	D3D12SerializeVersionedRootSignature(&computeRootSigDesc, &signature, &errorBlob);
	DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0, signature->GetBufferPointer(), signature->GetBufferSize(),
		IID_PPV_ARGS(&mParticleCollisionPSO.rootSignature)
	);


	// 現状 Fluid用に書き換えなので…
	auto cs = ShaderManager::GetInstance().GetShader("FluidParticleCollisionComputeShader");

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = mParticleCollisionPSO.rootSignature.Get();
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	// Collisionのパイプライン
	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&mParticleCollisionPSO.pso)
	);


	// Forceのパイプライン
	cs = ShaderManager::GetInstance().GetShader("FluidParticleForceComputeShader");
	mParticleForcePSO.rootSignature = mParticleCollisionPSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&mParticleForcePSO.pso)
	);

	// Integrateのパイプライン
	cs = ShaderManager::GetInstance().GetShader("FluidParticleIntegrateComputeShader");
	mParticleIntegratePSO.rootSignature = mParticleCollisionPSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&mParticleIntegratePSO.pso)
	);

	// Initのパイプライン
	cs = ShaderManager::GetInstance().GetShader("FluidParticleInitComputeShader");
	m_ParticleInitPSO.rootSignature = mParticleCollisionPSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&m_ParticleInitPSO.pso)
	);

	// Pressureのパイプライン
	cs = ShaderManager::GetInstance().GetShader("FluidParticlePressureComputeShader");
	mParticlePressurePSO.rootSignature = mParticleCollisionPSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&mParticlePressurePSO.pso)
	);

	// PressureGradientのパイプライン
	cs = ShaderManager::GetInstance().GetShader("FluidParticlePressureGradientComputeShader");
	mParticlePressureGradientPSO.rootSignature = mParticleCollisionPSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&mParticlePressureGradientPSO.pso)
	);

	// MoveParticleのパイプライン
	cs = ShaderManager::GetInstance().GetShader("FluidParticleMoveParticleComputeShader");
	mParticleMoveParticlePSO.rootSignature = mParticleCollisionPSO.rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(cs);

	DirectXDevice::GetInstance().GetDevice()->CreateComputePipelineState(
		&computePsoDesc,
		IID_PPV_ARGS(&mParticleMoveParticlePSO.pso)
	);
}

