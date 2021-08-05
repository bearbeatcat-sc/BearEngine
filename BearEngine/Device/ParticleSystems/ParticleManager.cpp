#include "ParticleManager.h"

#include "ParticleEmitter.h"
#include "ParticleActionManager.h"
#include "../DirectX/DirectXGraphics.h"
#include "../DirectX/DirectXDevice.h"
#include "FluidParticleAction.h"
#include "../DirectX/Core/Buffer.h"
#include "../DirectX/Core/EffectManager.h"
#include "../DirectX/Core/ShaderManager.h"

ParticleManager::ParticleManager()
{
	m_ParticleActionManager = new ParticleActionManager();
	GenerateHeap();
	GenereteHandles();
	InitFluidDepthPipeLine();
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

		if (emmit->GetDrawParticleMode() == ParticleEmitter::DrawParticleMode::DrawParticleMode_FluidDepth)
		{
			emmit->DrawWall();
			continue;
		}
		
		emmit->Draw();
	}
}

void ParticleManager::ZbufferDraw()
{
	std::string psoName = "";
	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();
	auto heap = m_Heap.Get();


	tempCommand->SetPipelineState(m_DepthPSO.pso.Get());
	tempCommand->SetGraphicsRootSignature(m_DepthPSO.rootSignature.Get());

	for (auto& emmit : m_ParticleEmitters)
	{
		if (!emmit->GetDrawFlag()) continue;

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
	m_ParticleActionManager->AddAction(action,actionName);
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

		if(emmit->GetDrawParticleMode() == ParticleEmitter::DrawParticleMode_FluidDepth)
		{
			auto fluidAction = std::dynamic_pointer_cast<FluidParticleAction>(action);
			emmit->UpdateFluidParticle(fluidAction, m_pCommandList);

		}
		
		// 通常のパーティクル描画		
		//emmit->UpdateNormalParticle(action, m_pCommandList);
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

bool ParticleManager::InitFluidDepthPipeLine()
{
	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	// 行列用
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	// コンピュートシェーダーでの演算結果
	rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
	// テクスチャ
	rootParameters[2].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);


	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = true;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = true;

	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_DEST_ALPHA;

	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE;


	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc2 = {};
	renderTargetBlendDesc2.BlendEnable = false;

	renderTargetBlendDesc2.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc2.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc2.DestBlendAlpha = D3D12_BLEND_DEST_ALPHA;

	renderTargetBlendDesc2.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc2.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc2.SrcBlend = D3D12_BLEND_ONE;
	renderTargetBlendDesc2.DestBlend = D3D12_BLEND_ONE;
	
	blendDesc.RenderTarget[1] = renderTargetBlendDesc2;

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	D3D12_RASTERIZER_DESC rasterizeDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// テスト用
	ID3DBlob* vertexShaderBlob = ShaderManager::GetInstance().GetShader("FluidDepthVertexShader");
	ID3DBlob* pixelShaderBlob = ShaderManager::GetInstance().GetShader("FluidDepthPixelShader");
	ID3DBlob* geomtryShaderBlob = ShaderManager::GetInstance().GetShader("FluidDepthGeometryShader");


	//PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);
	gpipeline.GS = CD3DX12_SHADER_BYTECODE(geomtryShaderBlob);

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizeDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	gpipeline.NumRenderTargets = 2;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;
	gpipeline.RTVFormats[1] = DXGI_FORMAT_R32_FLOAT;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	ID3DBlob* rootSigBlob;
	ID3DBlob* errorBlob;

	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_DepthPSO.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = m_DepthPSO.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&m_DepthPSO.pso))))
	{
		return false;
	}

	return true;
}
