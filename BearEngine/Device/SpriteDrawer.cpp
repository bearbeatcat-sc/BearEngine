#include "SpriteDrawer.h"
#include "./DirectX/DirectXDevice.h"
#include "./DirectX/DirectXGraphics.h"
#include "./DirectX/Core/PSOManager.h"
#include "./DirectX/DirectXInput.h"
#include "DirectX/Core/Buffer.h"
#include "Utility/Camera.h"
#include "DirectX/Core/EffectManager.h"

SpriteDrawer::SpriteDrawer()
{
}

SpriteDrawer::~SpriteDrawer()
{
	sprites.clear();
	m_ConstantHandles.clear();
}

bool SpriteDrawer::Init()
{
	if (!InitConstantHeaps())
	{
		return false;
	}
	if (!GenerateHandles())
	{
		return false;
	}

	return true;
}

void SpriteDrawer::Draw()
{
	DrawCall();
}

void SpriteDrawer::Update()
{
	CheckState();
}

void SpriteDrawer::AddSprite(std::shared_ptr<Sprite> sprite)
{
	if (sprites.size() >= m_ObjectCount - 1)
	{
		return;
	}

	if (!GenerateConstantView(sprite))
	{
		return;
	}

	int myOrder = sprite->GetDrawOrder();
	auto iter = sprites.begin();

	for (; iter != sprites.end(); ++iter)
	{
		if (myOrder < (*iter)->GetDrawOrder())
		{
			break;
		}
	}

	sprites.insert(iter,sprite);
}

void SpriteDrawer::CheckState()
{
	for (auto itr = sprites.begin(); itr != sprites.end();)
	{
		if ((*itr)->GetUpdateTextureFlag())
		{
			UpdateTexture((*itr));
		}

		if ((*itr)->m_DestroyFlag)
		{
			(*itr)->m_ConstantDescHandle->m_UseFlag = false;
			itr = sprites.erase(itr);
			continue;
		}

		itr++;
	}
}

bool SpriteDrawer::GenerateHandles()
{
	for (int i = 0; i < m_ObjectCount; i++)
	{

		UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_BasicDescHeap->GetCPUDescriptorHandleForHeapStart(),
			i * 2,
			descHandleInc
		);

		auto gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_BasicDescHeap->GetGPUDescriptorHandleForHeapStart(),
			i * 2,
			descHandleInc);

		m_ConstantHandles.push_back(MeshDrawer::CPU_GPU_Handles{ cpuHandle,gpuHandle,false });
	}

	return true;
}

bool SpriteDrawer::InitConstantHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 2 * m_ObjectCount;
	descHeapDesc.NodeMask = 0;

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&m_BasicDescHeap))))
	{
		return false;
	}

	return true;
}

bool SpriteDrawer::GenerateConstantView(std::shared_ptr<Sprite> sprite)
{
	auto constBuff = sprite->GetConstanBuffer()->getBuffer();

	MeshDrawer::CPU_GPU_Handles* handles = nullptr;
	bool m_isFind = false;

	for (int i = 0; i < m_ConstantHandles.size(); i++)
	{
		if (!m_ConstantHandles.at(i).m_UseFlag)
		{
			handles = &m_ConstantHandles.at(i);
			m_ConstantHandles.at(i).m_UseFlag = true;
			m_isFind = true;
			break;
		}
	}

	sprite->m_ConstantDescHandle = handles;


	// 定数ビューの生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		sprite->m_ConstantDescHandle->m_CPUDescHandle);


	// シェーダーリソースビューの生成
	UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto constDescHeapH = sprite->m_ConstantDescHandle->m_CPUDescHandle;
	constDescHeapH.ptr += descHandleInc;

	D3D12_SHADER_RESOURCE_VIEW_DESC srVDesc = {};
	srVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srVDesc.Texture2D.MipLevels = 1;


	// テクスチャバッファ
	srVDesc.Format = sprite->GetTextureBuffer().Get()->GetDesc().Format;
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		sprite->GetTextureBuffer().Get(),
		&srVDesc,
		constDescHeapH);

	return true;
}

bool SpriteDrawer::DrawCall()
{
	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	auto heap = m_BasicDescHeap.Get();

	for (auto sprite: sprites)
	{
		if (!sprite->GetDrawFlag()) continue;

		auto pso = EffectManager::GetInstance().GetEffect(sprite->GetEffectName())->GetPSO();
		tempCommand->SetPipelineState(pso->pso.Get());
		tempCommand->SetGraphicsRootSignature(pso->rootSignature.Get());


		// 行列用ディスクリプタの設定
		tempCommand->SetDescriptorHeaps(1, &heap);
		tempCommand->SetGraphicsRootDescriptorTable(0, sprite->m_ConstantDescHandle->m_GPUDescHandle);

		auto handle = sprite->m_ConstantDescHandle->m_GPUDescHandle;
		handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		tempCommand->SetGraphicsRootDescriptorTable(1, handle);

		sprite->Draw(tempCommand);
	}
	return true;
}

void SpriteDrawer::UpdateTexture(std::shared_ptr<Sprite> sprite)
{
	// シェーダーリソースビューの再生成
	UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto constDescHeapH = sprite->m_ConstantDescHandle->m_CPUDescHandle;
	constDescHeapH.ptr += descHandleInc;

	D3D12_SHADER_RESOURCE_VIEW_DESC srVDesc = {};
	srVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srVDesc.Texture2D.MipLevels = 1;


	// テクスチャバッファ
	srVDesc.Format = sprite->GetTextureBuffer().Get()->GetDesc().Format;
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		sprite->GetTextureBuffer().Get(),
		&srVDesc,
		constDescHeapH);

}
