#ifndef _PARTICLE_ACTION_H_
#define _PARTICLE_ACTION_H_

#include <d3d12.h>
#include <wrl/client.h>
#include "../DirectX/Core/PSOManager.h"

class ParticleAction
{
public:
	ParticleAction(float destroyTime)
		:m_DestroyTime(destroyTime)
	{}
	
	virtual ~ParticleAction() {};

	virtual void Update() = 0;
	ID3D12Resource* GetBuffer() { return m_UpdateParticleParamsBuffer.Get(); }
	const PSO& GetInitPSO() { return m_ParticleInitPSO; }

	void SetDeathTime(float time) { m_DestroyTime = time; }

protected:
	virtual void Init() = 0;
	virtual void UpdateConstanBuffer() = 0;
	virtual void GenerateConstantBuffer() = 0;
	virtual void CreatePSO() = 0;

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UpdateParticleParamsBuffer;
	PSO m_ParticleInitPSO;
	
	float m_DestroyTime;
	bool m_DummyFlag;

};

#endif