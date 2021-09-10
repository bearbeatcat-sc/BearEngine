#ifndef _GPU_PARTICLE_MANAGER_H_
#define _GPU_PARTICLE_MANAGER_H_

#include "../Singleton.h"
#include <vector>
#include <memory>
#include <d3d12.h>
#include <d3dx12.h>

#include "../DirectX/Core/PSOManager.h"

using Microsoft::WRL::ComPtr;

class ParticleEmitter;
class ParticleActionManager;
class ParticleAction;
class ParticleSequence;
class ParticleAction;

class ParticleManager
	: public Singleton<ParticleManager>
{
public:
	friend class Singleton<ParticleManager>;
	void Update();
	void DeleteParticles();
	void Draw();
	void AddParticleEmiiter(std::shared_ptr<ParticleEmitter> emiiter);
	void DeleteAll();
	void AddAction(std::shared_ptr<ParticleAction> action, const std::string& actionName);
	void AddSequence(std::shared_ptr<ParticleSequence> sequence);
	std::shared_ptr<ParticleAction> GetAction(const std::string& actionName);

	struct EmitterDescHandles
	{
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHandle;
		bool m_UseFlag;
	};

private:
	void Delete();
	void UpdateEmitter();
	void UpdateActions();
	void GenerateHeap();
	bool GenerateView(std::shared_ptr<ParticleEmitter> emiiter);
	void GenereteHandles();

protected:
	ParticleManager();
	~ParticleManager();

private:


	const int m_EmitterMaxCount = 120;
	std::vector<std::shared_ptr<ParticleEmitter>> m_ParticleEmitters;
	ParticleActionManager* m_ParticleActionManager;
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	std::vector<EmitterDescHandles*> m_Handles;
	ID3D12GraphicsCommandList* m_pCommandList;

};

#endif