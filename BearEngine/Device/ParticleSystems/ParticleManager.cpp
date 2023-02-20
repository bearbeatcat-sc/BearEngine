#include "ParticleManager.h"

#include "ParticleEmitter.h"
#include "ParticleActionManager.h"
#include "../DirectX/DirectXGraphics.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/Core/EffectManager.h"
#include "../DirectX/Core/ShaderManager.h"
#include "NormalParticleAction.h"

#include <algorithm>

ParticleManager::ParticleManager()
{
	m_ParticleActionManager = new ParticleActionManager();
	GenerateHeap();
	GenereteHandles();
	m_pCommandList = DirectXGraphics::GetInstance().GetCommandList();
}

ParticleManager::~ParticleManager()
{
	for (auto itr = m_ParticleEmitters.begin(); itr != m_ParticleEmitters.end();)
	{
		itr = m_ParticleEmitters.erase(itr);
	}

	for (auto itr = m_Handles.begin(); itr != m_Handles.end(); ++itr)
	{
		delete (*itr);
	}

	m_Handles.clear();
	m_Handles.shrink_to_fit();

	delete m_ParticleActionManager;
}


void ParticleManager::Update()
{
	UpdateActions();
	UpdateEmitter();
}

void ParticleManager::DeleteParticles()
{
	Delete();
}

void ParticleManager::Draw()
{
	std::string psoName = "";
	PSO* pso;
	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();
	auto heap = m_Heap.Get();

	for (auto& emmit : m_ParticleEmitters)
	{
		if (!emmit->GetDrawFlag()) continue;

		auto tempEffectName = emmit->GetEffectName();

		if (psoName != tempEffectName)
		{
			psoName = tempEffectName;
			pso = EffectManager::GetInstance().GetEffect(psoName)->GetPSO();
			tempCommand->SetPipelineState(pso->pso.Get());
			tempCommand->SetGraphicsRootSignature(pso->rootSignature.Get());
		}

		tempCommand->SetDescriptorHeaps(1, &heap);


		emmit->Draw();
	}
}


void ParticleManager::AddParticleEmiiter(std::shared_ptr<ParticleEmitter> emiiter)
{
	if (m_ParticleEmitters.size() >= m_EmitterMaxCount)
	{
		return;
	}

	emiiter->GenerateTexture();

	if (!GenerateView(emiiter))
	{
		return;
	}

	emiiter->Init();

	m_ParticleEmitters.push_back(emiiter);

	std::sort(m_ParticleEmitters.begin(), m_ParticleEmitters.end(), [](const std::shared_ptr<ParticleEmitter> hoge1, const std::shared_ptr<ParticleEmitter> hoge2)
	{
		return hoge1->GetEffectName() > hoge2->GetEffectName();
	});
}

void ParticleManager::DeleteAll()
{
	for (auto itr = m_ParticleEmitters.begin(); itr != m_ParticleEmitters.end(); ++itr)
	{
		(*itr)->Destroy();
	}

}

void ParticleManager::AddAction(std::shared_ptr<ParticleAction> action, const std::string& actionName)
{
	m_ParticleActionManager->AddAction(action, actionName);
}

void ParticleManager::AddSequence(std::shared_ptr<ParticleSequence> sequence)
{
	m_ParticleActionManager->AddSequence(sequence);
}

std::shared_ptr<ParticleAction> ParticleManager::GetAction(const std::string& actionName)
{
	return m_ParticleActionManager->GetAction(actionName);
}

void ParticleManager::Delete()
{
	for (auto itr = m_ParticleEmitters.begin(); itr != m_ParticleEmitters.end();)
	{
		if ((*itr)->IsDestroyFlag())
		{
			(*itr)->GetHandle()->m_UseFlag = false;
			//delete (*itr);
			itr = m_ParticleEmitters.erase(itr);
			continue;
		}

		++itr;
	}
}

void ParticleManager::UpdateEmitter()
{

	for (auto& emmit : m_ParticleEmitters)
	{
		if (!emmit->GetUpdateFlag()) continue;

		auto actionName = emmit->GetActionName();
		auto action = m_ParticleActionManager->GetAction(actionName);

		const std::shared_ptr<NormalParticleAction> normalAction = std::static_pointer_cast<NormalParticleAction>(action);

		// 通常のパーティクル描画		
		emmit->UpdateNormalParticle(normalAction, m_pCommandList);
	}

}

void ParticleManager::UpdateActions()
{
	m_ParticleActionManager->Update();
}

void ParticleManager::GenerateHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = m_EmitterMaxCount; //テクスチャ
	descHeapDesc.NodeMask = 0;

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&m_Heap))))
	{
		return;
	}

	return;
}

bool ParticleManager::GenerateView(std::shared_ptr<ParticleEmitter> emiiter)
{
	EmitterDescHandles* handles = nullptr;
	bool m_isFind = false;

	for (int i = 0; i < m_EmitterMaxCount; i++)
	{
		if (!m_Handles.at(i)->m_UseFlag)
		{
			handles = m_Handles.at(i);
			m_Handles.at(i)->m_UseFlag = true;
			m_isFind = true;
			break;
		}
	}

	// 定数バッファのハンドルが余っていないので生成不可
	if (!m_isFind)return false;

	auto constBuff = emiiter->GetTextureBuffer();

	emiiter->SetHandle(handles);

	return true;
}

void ParticleManager::GenereteHandles()
{
	UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_Handles.reserve(m_EmitterMaxCount);

	for (int i = 0; i < m_EmitterMaxCount; i++)
	{

		auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_Heap->GetCPUDescriptorHandleForHeapStart(),
			i,
			descHandleInc
		);

		auto gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_Heap->GetGPUDescriptorHandleForHeapStart(),
			i,
			descHandleInc);

		m_Handles.push_back(new ParticleManager::EmitterDescHandles{ cpuHandle,gpuHandle,false });
	}
}

