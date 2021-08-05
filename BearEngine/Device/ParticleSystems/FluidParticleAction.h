#ifndef _FLUIDPARTICLE_ACTION_H_
#define _FLUIDPARTICLE_ACTION_H_

#include "ParticleActionManager.h"
#include "../DirectX/Core/PSOManager.h"
#include "ParticleAction.h"

class Random;
class Timer;

class FluidParticleAction
	:public ParticleAction
{
public:
	FluidParticleAction(const std::string& particleUpdateShaderName = "ParticleUpdateComputeShader", const std::string& particleInitShaderName = "ParticleInitComputeShader");
	~FluidParticleAction();

	struct GPUParticleActionData
	{
		XMFLOAT4 gravity;
		float timeStep;
		float maxVelocity;
		float radiusForNumberDensity;
		float radiusForGradient;
		float radiusForLaplacian;
		float n0ForNumberDensity;
		float n0ForGradient;
		float n0ForLaplacian;
		float lambda;
		int DIM;
		float PARTICLE_DISTANCE;
		float DT;
		float COEFFICIENT_OF_RESTITUTION;
		float KINEMATIC_VISCOSITY;
		float FLUID_DENSITY;
		float wallCount;
	};

	
	void Update() override;

	//void SetDeathTime(float time);

	void SetRotateRange(const DirectX::SimpleMath::Vector3& rotate);
	void SetPositionRange(const DirectX::SimpleMath::Vector3& position);
	void SetVelocityRange(const DirectX::SimpleMath::Vector3& velocity);
	void SetScaleRange(const DirectX::SimpleMath::Vector3& scale);
	void SetColorRange(const DirectX::SimpleMath::Color& color);

	void SetAddColor(const DirectX::SimpleMath::Color& color);
	void SetAddRotate(const DirectX::SimpleMath::Vector3& rotate);
	void SetAddVelocity(const DirectX::SimpleMath::Vector3& velocity);
	void SetAddScale(const DirectX::SimpleMath::Vector3& scale);
	void SetWallCount(int count);

	enum RootParameterIndex
	{
		RootParam_ParticleUpdateParam = 0,
		RootParam_SRV = 1,
		RootParam_UAV = 2,
		RootParam_DensitySRV = 3,
		RootParam_DensityUAV = 4,
		RootParam_ForcesSRV = 5,
		RootParam_ForcesUAV = 6,
		RootParam_WallPalamSRV = 7,
		RootParam_WallPalamUAV = 8,
		RootParam_VerticesPosition = 9,
		RootParam_EmiiterDataParam = 10,
	};


	PSO& GetForcePSO();
	PSO& GetPressurePSO();
	PSO& GetCollsionPSO();
	PSO& GetPressureGradientPSO();
	PSO& GetIntegratePSO();
	PSO& GetMoveParticlePSO();

	ID3D12Resource* GetBuffer();

private:
	void Init() override;
	void UpdateConstanBuffer() override;
	void GenerateConstantBuffer() override;
	void CreatePSO() override;
	
	float Distance2(SimpleMath::Vector3 v1, SimpleMath::Vector3 v2);
	float Weight(float dist, float re);
	void CalcNZeroAndLambda();
	void CalcConstantParameter();



private:
	//ComPtr<ID3D12Resource> m_UpdateParticleParamsBuffer;

	DirectX::SimpleMath::Vector4 m_PositionRange;
	DirectX::SimpleMath::Vector4 m_VelocityRange;
	DirectX::SimpleMath::Vector4 m_ScaleRange;
	DirectX::SimpleMath::Color m_ColorRange;
	DirectX::SimpleMath::Vector4 m_RotateRange;

	DirectX::SimpleMath::Color m_AddColor;
	DirectX::SimpleMath::Vector4 m_AddRotate;
	DirectX::SimpleMath::Vector4 m_AddVelocity;
	DirectX::SimpleMath::Vector4 m_AddScale;

	std::string m_ParticleUpdateComputeShader;
	std::string m_ParticleInitComputeShader;

	//float Smoothlen = 0.012f;
	//float PressureStiffness = 200.0f;
	//float RestDensity = 1000.0f;
	//float ParticleMass = 0.0002f;
	//float Viscosity = 0.1f;
	//float MaxAllowableTimeStep = 0.005f;
	float MaxAllowableTimeStep = 32.0f;
	//float Restitution = 1.0f;
	//float MaxVelocity = 0.50f;

	int DIM = 3;
	float PARTICLE_DISTANCE = 0.22f;
	float DT = 0.001f;
	float KINEMATIC_VISCOSITY = (1.0E-6);
	float FLUID_DENSITY = 1000.0f;
	float RADIUS_FOR_NUMBER_DENSITY = (2.1f * PARTICLE_DISTANCE);
	float RADIUS_FOR_GRADIENT = (2.1f * PARTICLE_DISTANCE);
	float RADIUS_FOR_LAPLACIAN = (3.1f * PARTICLE_DISTANCE);
	float COLLISION_DISTANCE = (1.0f * PARTICLE_DISTANCE);
	float COEFFICIENT_OF_RESTITUTION = 0.2f;

	float radiusForNumberDensity;
	float radiusForNumberDensity2;
	float radiusForGradient;
	float radiusForGradient2;
	float radiusForLaplacian;
	float radiusForLaplacian2;
	float n0ForNumberDensity;
	float n0ForGradient;
	float n0ForLaplacian;
	float lambda;
	float collisionDistance;
	float collisionDistance2;
	float fluidDensity;
	int wallCount;
	
	SimpleMath::Vector4 Gravity = SimpleMath::Vector4(0.0f, -0.6f, 0.0f, 0.0f);


	PSO mParticleMoveParticlePSO;
	PSO mParticleForcePSO;
	PSO mParticlePressurePSO;
	PSO mParticleCollisionPSO;
	PSO mParticlePressureGradientPSO;
	PSO mParticleIntegratePSO;

	//float m_DestroyTime;
	//bool m_DummyFlag;


};

#endif