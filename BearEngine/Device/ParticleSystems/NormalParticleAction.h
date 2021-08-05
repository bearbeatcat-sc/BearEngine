#ifndef _GPUPARTICLE_ACTION_H_
#define _GPUPARTICLE_ACTION_H_

#include "ParticleActionManager.h"
#include "ParticleAction.h"

class Random;

class NormalParticleAction
	:public ParticleAction
{
public:


	
	NormalParticleAction(const std::string& particleUpdateShaderName = "ParticleUpdateComputeShader", const std::string& particleInitShaderName = "ParticleInitComputeShader");
	~NormalParticleAction();
	void Update();

	struct NormalParticleActionData
	{
		XMFLOAT4 AddVelocity;
		XMFLOAT4 AddRotate;
		XMFLOAT4 AddColor;
		XMFLOAT4 AddScale;

		XMFLOAT4 ColorRange;
		XMFLOAT4 PositionRange;
		XMFLOAT4 VecRange;
		XMFLOAT4 ScaleRange;
		XMFLOAT4 RotateRange;

		float LifeTimeRange;
	};

	void SetDeathTime(float time);

	void SetRotateRange(const DirectX::SimpleMath::Vector3& rotate);
	void SetPositionRange(const DirectX::SimpleMath::Vector3& position);
	void SetVelocityRange(const DirectX::SimpleMath::Vector3& velocity);
	void SetScaleRange(const DirectX::SimpleMath::Vector3& scale);
	void SetColorRange(const DirectX::SimpleMath::Color& color);

	void SetAddColor(const DirectX::SimpleMath::Color& color);
	void SetAddRotate(const DirectX::SimpleMath::Vector3& rotate);
	void SetAddVelocity(const DirectX::SimpleMath::Vector3& velocity);
	void SetAddScale(const DirectX::SimpleMath::Vector3& scale);

	enum RootParameterIndex
	{
		RootParam_ParticleUpdateParam = 0,
		RootParam_SRV = 1,
		RootParam_UAV = 2,
		RootParam_VerticesPosition = 3,
		RootParam_EmiiterDataParam = 4,
	};

	PSO& GetUpdatePSO();

private:
	 void Init() override;
	void UpdateConstanBuffer() override;
	void GenerateConstantBuffer() override;
	void CreatePSO() override;

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


	PSO m_ParticleUpdatePSO;

	float m_DestroyTime;
	bool m_DummyFlag;
};

#endif