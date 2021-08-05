#include "MeshDrawer.h"

#include "Device/DirectX/Core/Buffer.h"
#include "Device/DirectX/DirectXDevice.h"
#include "Device/DirectX/DirectXGraphics.h"
#include "Mesh.h"
#include "Utility/Camera.h"
#include "../../../../Utility/CameraManager.h"
#include "../EffectManager.h"
#include "../../../Lights/Light.h"
#include "../../../Lights/DirectionalLight.h"
#include "../../../Lights/LightManager.h"
#include "../ShaderManager.h"
#include "../../../../Utility/LogSystem.h"
#include "../../../../Utility/Math/Plane.h"
#include "../../../../imgui/imgui.h"

#include <assert.h>
#include <algorithm>
#include <vector>
#include <chrono>


MeshDrawer::MeshDrawer()
{
}

MeshDrawer::~MeshDrawer()
{
	m_meshs.clear();

	for (auto itr = m_MatHandles.begin(); itr != m_MatHandles.end(); ++itr)
	{
		delete (*itr);
	}

	m_MatHandles.clear();
	m_MatHandles.shrink_to_fit();


	for (auto itr = m_ConstantHandles.begin(); itr != m_ConstantHandles.end(); ++itr)
	{
		delete (*itr);
	}
	m_ConstantHandles.clear();	
	m_ConstantHandles.shrink_to_fit();

}

bool MeshDrawer::Init()
{
	if (!GenerateShadowPSO())
	{
		assert(0);
		return false;
	}

	if (!InitMateralHeaps())
	{
		assert(0);
		return false;
	}
	if (!InitConstantHeaps())
	{
		assert(0);
		return false;
	}
	if (!GenerateHandles())
	{
		assert(0);
		return false;
	}

	m_Camera = CameraManager::GetInstance().GetMainCamera();
	m_DirLight = LightManager::GetInstance().GetDirectionalLight();

	return true;
}

void MeshDrawer::Update()
{
	Delete();
}

void MeshDrawer::Draw()
{
	std::chrono::system_clock::time_point start, end;

	start = std::chrono::system_clock::now();




	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	auto heap = m_BasicDescHeap.Get();
	auto matHeap = m_MaterialDescHeap.Get();
	auto depthHeap = DirectXGraphics::GetInstance().GetDepthSRVHeap();

	// 視錐台カリング処理
	auto visibleMeshs = FrustumCulling();

	//size_t size = m_meshs.size();
	size_t size = visibleMeshs.size();



	PSO* pso = nullptr;
	std::string effectName = "";

	for (size_t i = 0; i < size; ++i)
	{
		//if (!m_meshs[i]->IsVisible()) continue;
		if (!visibleMeshs[i]->IsVisible()) continue;

		//auto temp_EffectName = m_meshs[i]->GetEffectName();
		auto temp_EffectName = visibleMeshs[i]->GetEffectName();

		// 設定されているPSO名が違うなら、パイプラインをセットし直す
		if (temp_EffectName != effectName)
		{
			effectName = temp_EffectName;

			pso = EffectManager::GetInstance().GetEffect(effectName)->GetPSO();

			tempCommand->SetPipelineState(pso->pso.Get());
			tempCommand->SetGraphicsRootSignature(pso->rootSignature.Get());

			// 影用のディスクリプタ設定
			tempCommand->SetDescriptorHeaps(1, &depthHeap);
			auto handle = depthHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_ShadowTex, handle);
		}


		// 行列用ディスクリプタの設定
		tempCommand->SetDescriptorHeaps(1, &heap);
		//auto constHandle = m_meshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		auto constHandle = visibleMeshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Matrix, constHandle);

		tempCommand->SetGraphicsRootConstantBufferView(RootParamterIndex_Light, m_DirLight->GetBuffer()->getBuffer()->GetGPUVirtualAddress());

		// マテリアルの適用
		tempCommand->SetDescriptorHeaps(1, &matHeap);

		//auto matHandle = m_meshs[i]->GetMatDescHandle()->m_GPUDescHandle;
		auto matHandle = visibleMeshs[i]->GetMatDescHandle()->m_GPUDescHandle;
		unsigned int offset = 0;


		visibleMeshs[i]->SetBuffer(tempCommand);
		//m_meshs[i]->SetBuffer(tempCommand);

		//for (auto& material : m_meshs[i]->GetDrawMaterialDatas())
		for (auto& material : visibleMeshs[i]->GetDrawMaterialDatas())
		{
			tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Material, matHandle);
			tempCommand->DrawIndexedInstanced(material.indexCount, 1, offset, 0, 0);

			offset += material.indexCount;
			matHandle.ptr += m_MatIncSize;
		}

	}
	
	end = std::chrono::system_clock::now();

	wchar_t str[256];
	m_CurrentDrawTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);

	m_CurrentVisibleMeshCount = visibleMeshs.size();
}

void MeshDrawer::CubeMapDraw(int index)
{
	std::chrono::system_clock::time_point start, end;

	start = std::chrono::system_clock::now();




	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	auto heap = m_BasicDescHeap.Get();
	auto matHeap = m_MaterialDescHeap.Get();
	auto depthHeap = DirectXGraphics::GetInstance().GetDepthSRVHeap();

	// 視錐台カリング処理
	//auto visibleMeshs = FrustumCulling();

	// TODO:後で最適化
	size_t size = m_meshs.size();
	//size_t size = visibleMeshs.size();



	PSO* pso = nullptr;
	std::string effectName = "";

	pso = EffectManager::GetInstance().GetEffect("CubeMapMeshEffect")->GetPSO();

	tempCommand->SetPipelineState(pso->pso.Get());
	tempCommand->SetGraphicsRootSignature(pso->rootSignature.Get());

	// 影用のディスクリプタ設定
	tempCommand->SetDescriptorHeaps(1, &depthHeap);
	auto handle = depthHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_ShadowTex, handle);

	for (size_t i = 0; i < size; ++i)
	{
		//if (!m_meshs[i]->IsVisible()) continue;
		if (!m_meshs[i]->IsVisible()) continue;

		//auto temp_EffectName = m_meshs[i]->GetEffectName();
		//auto temp_EffectName = m_meshs[i]->GetEffectName();

		// 行列用ディスクリプタの設定
		tempCommand->SetDescriptorHeaps(1, &heap);
		//auto constHandle = m_meshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		//auto constHandle = visibleMeshs[i]->GetConstantDescHandle()->m_GPUDescHandle;

		auto cubemapConstantBuffers = m_meshs[i]->GetCubeMapConstantBuffers();

		auto constHandle = m_meshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Matrix, constHandle);
		tempCommand->SetGraphicsRootConstantBufferView(RootParamterIndex_Light, m_DirLight->GetBuffer()->getBuffer()->GetGPUVirtualAddress());

		// 直接Bufferを指定する
		tempCommand->SetGraphicsRootConstantBufferView(RootParamterIndex_CubeMapMatrix, cubemapConstantBuffers.at(index)->getBuffer()->GetGPUVirtualAddress());

		
		// マテリアルの適用
		tempCommand->SetDescriptorHeaps(1, &matHeap);

		//auto matHandle = m_meshs[i]->GetMatDescHandle()->m_GPUDescHandle;
		auto matHandle = m_meshs[i]->GetMatDescHandle()->m_GPUDescHandle;
		unsigned int offset = 0;


		m_meshs[i]->SetBuffer(tempCommand,index);
		//m_meshs[i]->SetBuffer(tempCommand);

		//for (auto& material : m_meshs[i]->GetDrawMaterialDatas())
		for (auto& material : m_meshs[i]->GetDrawMaterialDatas())
		{
			tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Material, matHandle);
			tempCommand->DrawIndexedInstanced(material.indexCount, 1, offset, 0, 0);

			offset += material.indexCount;
			matHandle.ptr += m_MatIncSize;
		}

	}



	end = std::chrono::system_clock::now();

	wchar_t str[256];
	m_CurrentDrawTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);

	//m_CurrentVisibleMeshCount = visibleMeshs.size();
}

void MeshDrawer::WallDraw()
{
	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	auto heap = m_BasicDescHeap.Get();
	auto matHeap = m_MaterialDescHeap.Get();
	auto depthHeap = DirectXGraphics::GetInstance().GetDepthSRVHeap();

	// 視錐台カリング処理
	auto visibleMeshs = FrustumCulling();

	//size_t size = m_meshs.size();
	size_t size = visibleMeshs.size();



	PSO* pso = nullptr;
	std::string effectName = "";

	for (size_t i = 0; i < size; ++i)
	{
		//if (!m_meshs[i]->IsVisible()) continue;
		if (!visibleMeshs[i]->IsVisible()) continue;
		if (m_meshs[i]->GetDrawType() != DrawType::Wall) continue;

		//auto temp_EffectName = m_meshs[i]->GetEffectName();
		auto temp_EffectName = visibleMeshs[i]->GetEffectName();

		// 設定されているPSO名が違うなら、パイプラインをセットし直す
		if (temp_EffectName != effectName)
		{
			effectName = temp_EffectName;

			pso = EffectManager::GetInstance().GetEffect(effectName)->GetPSO();

			tempCommand->SetPipelineState(pso->pso.Get());
			tempCommand->SetGraphicsRootSignature(pso->rootSignature.Get());

			// 影用のディスクリプタ設定
			tempCommand->SetDescriptorHeaps(1, &depthHeap);
			auto handle = depthHeap->GetGPUDescriptorHandleForHeapStart();
			handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_ShadowTex, handle);
		}


		// 行列用ディスクリプタの設定
		tempCommand->SetDescriptorHeaps(1, &heap);
		//auto constHandle = m_meshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		auto constHandle = visibleMeshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Matrix, constHandle);

		tempCommand->SetGraphicsRootConstantBufferView(RootParamterIndex_Light, m_DirLight->GetBuffer()->getBuffer()->GetGPUVirtualAddress());

		// マテリアルの適用
		tempCommand->SetDescriptorHeaps(1, &matHeap);

		//auto matHandle = m_meshs[i]->GetMatDescHandle()->m_GPUDescHandle;
		auto matHandle = visibleMeshs[i]->GetMatDescHandle()->m_GPUDescHandle;
		unsigned int offset = 0;


		visibleMeshs[i]->SetBuffer(tempCommand);
		//m_meshs[i]->SetBuffer(tempCommand);

		//for (auto& material : m_meshs[i]->GetDrawMaterialDatas())
		for (auto& material : visibleMeshs[i]->GetDrawMaterialDatas())
		{
			tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Material, matHandle);
			tempCommand->DrawIndexedInstanced(material.indexCount, 1, offset, 0, 0);

			offset += material.indexCount;
			matHandle.ptr += m_MatIncSize;
		}

	}

}

void MeshDrawer::DrawDebug()
{
#ifdef _DEBUG

	if (ImGui::BeginTabItem("MeshDrawProperties"))
	{
		ImGui::Text("MeshDrawTime:%d", m_CurrentDrawTime);
		ImGui::Text("CullingObject:%i", m_meshs.size() - m_CurrentVisibleMeshCount);
		ImGui::EndTabItem();
	}

#endif

}

void MeshDrawer::ShadowDraw()
{
	ID3D12GraphicsCommandList* tempCommand = DirectXGraphics::GetInstance().GetCommandList();

	auto heap = m_BasicDescHeap.Get();


	size_t size = m_meshs.size();


	for (size_t i = 0; i < size; ++i)
	{
		tempCommand->SetPipelineState(m_ShadowPSO.pso.Get());
		tempCommand->SetGraphicsRootSignature(m_ShadowPSO.rootSignature.Get());
		tempCommand->SetDescriptorHeaps(1, &heap);

		// 定数バッファ
		auto constHandle = m_meshs[i]->GetConstantDescHandle()->m_GPUDescHandle;
		tempCommand->SetGraphicsRootDescriptorTable(RootParamterIndex_Matrix, constHandle);
		//tempCommand->SetGraphicsRootConstantBufferView(1, m_DirLight->GetBuffer()->getBuffer()->GetGPUVirtualAddress());


		unsigned int offset = 0;

		for (auto& material : m_meshs[i]->GetDrawMaterialDatas())
		{
			m_meshs[i]->SetBuffer(tempCommand);
			tempCommand->DrawIndexedInstanced(material.indexCount, 1, offset, 0, 0);

			offset += material.indexCount;
		}

	}
}

void MeshDrawer::AddObjMesh(std::shared_ptr<Mesh> mesh)
{
	if (m_meshs.size() >= m_ObjectCount - 1)
	{
		
		assert(0);
		return;
	}

	if (!GenerateViews(mesh))
	{
		assert(0);
		return;
	}

	m_meshs.push_back(mesh);

	std::sort(m_meshs.begin(), m_meshs.end(), [](const std::shared_ptr<Mesh> hoge1, const std::shared_ptr<Mesh> hoge2)
	{
		return hoge1->GetEffectName() > hoge2->GetEffectName();
	});
	
}

void MeshDrawer::Delete()
{
	for (auto itr = m_meshs.begin(); itr != m_meshs.end();)
	{
		if ((*itr)->GetDestroyFlag())
		{
			(*itr)->GetConstantDescHandle()->m_UseFlag = false;
			(*itr)->GetMatDescHandle()->m_UseFlag = false;
			itr = m_meshs.erase(itr);
			continue;
		}

		++itr;
	}
}

const std::vector<std::shared_ptr<Mesh>> MeshDrawer::FrustumCulling()
{
	std::vector<std::shared_ptr<Mesh>> visibleMeshs;
	
	for (auto mesh : m_meshs)
	{
		if (IsInSideFrustum(mesh->GetMeshAABB()))
		{
			visibleMeshs.push_back(mesh);
		}
	}

	return visibleMeshs;
}

bool MeshDrawer::IsInSideFrustum(const MeshData::MeshAABB& meshAABB)
{
	// 現状、メインカメラでの想定のみ
	auto frustum = CameraManager::GetInstance().GetMainCamera()->GetFrustum();
	
	size_t size = frustum.size();

	bool result = true;

	SimpleMath::Vector3 scale = (meshAABB.m_Min - meshAABB.m_Max) * 0.5f;
	auto center = meshAABB.m_Center;

	for(int i = 0; i < size; i++)
	{
		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x - scale.x, center.y - scale.y, center.z - scale.z)) 
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x + scale.x, center.y - scale.y, center.z - scale.z))
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x - scale.x, center.y + scale.y, center.z - scale.z))
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x + scale.x, center.y + scale.y, center.z - scale.z))
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x - scale.x, center.y - scale.y, center.z + scale.z))
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x + scale.x, center.y - scale.y, center.z + scale.z))
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x - scale.x, center.y + scale.y, center.z + scale.z))
			>= 0.0f)
		{
			continue;
		}

		if (frustum[i]->DotCoordinate(
			SimpleMath::Vector3(center.x + scale.x, center.y + scale.y, center.z + scale.z))
			>= 0.0f)
		{
			continue;
		}

		return false;
	}

	return result;
}



bool MeshDrawer::InitMateralHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC matDecHeapDesc = {};
	matDecHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matDecHeapDesc.NodeMask = 0;
	matDecHeapDesc.NumDescriptors = m_MaterialCount * m_CBVAndSRV * m_ObjectCount; // マテリアル数 * 2 * オブジェクト数
	matDecHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&matDecHeapDesc,
		IID_PPV_ARGS(&m_MaterialDescHeap)
	))
	{
		return false;
	}

	return true;
}

bool MeshDrawer::InitConstantHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = m_ObjectCount * m_ContanstBuffCount; // 定数バッファ2つ分なので

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&m_BasicDescHeap))))
	{
		return false;
	}

	return true;
}

bool MeshDrawer::GenerateHandles()
{
	UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_MatHandles.reserve(m_ObjectCount);
	m_ConstantHandles.reserve(m_ObjectCount);

	for (int i = 0; i < m_ObjectCount; i++)
	{

		auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_MaterialDescHeap->GetCPUDescriptorHandleForHeapStart(),
			i * m_CBVAndSRV * m_MaterialCount,
			descHandleInc
		);

		auto gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_MaterialDescHeap->GetGPUDescriptorHandleForHeapStart(),
			i * m_CBVAndSRV * m_MaterialCount,
			descHandleInc);

		m_MatHandles.push_back(new MeshDrawer::CPU_GPU_Handles{ cpuHandle,gpuHandle,false });
	}

	for (int i = 0; i < m_ObjectCount; i++)
	{

		auto cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_BasicDescHeap->GetCPUDescriptorHandleForHeapStart(),
			i * m_ContanstBuffCount,
			descHandleInc
		);

		auto gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_BasicDescHeap->GetGPUDescriptorHandleForHeapStart(),
			i * m_ContanstBuffCount,
			descHandleInc);

		m_ConstantHandles.push_back(new MeshDrawer::CPU_GPU_Handles{ cpuHandle,gpuHandle,false });
	}

	m_DescHandleCount = m_ObjectCount;
	m_MatIncSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	) * (m_ContanstBuffCount + 1); // 定数バッファの数分、ずらす

	return true;
}

bool MeshDrawer::GenerateShadowPSO()
{

	D3D12_DESCRIPTOR_RANGE descTblRange[4] = {};

	// 行列用
	descTblRange[0].NumDescriptors = 1;
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[0].BaseShaderRegister = 0;
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// マテリアル定数バッファ
	descTblRange[1].NumDescriptors = 1;
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange[1].BaseShaderRegister = 2;
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// テクスチャ用バッファ
	descTblRange[2].NumDescriptors = 1;
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTblRange[2].BaseShaderRegister = 0;
	descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// シャドウマップ用のテクスチャ
	descTblRange[3] = CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);



	// ライトと行列関連
	CD3DX12_ROOT_PARAMETER rootparam[4] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 多分、ディスクリプタと連動してるので、マテリアル情報とテクスチャは同じディスクリプタで
	// 管理しているから、こうなってる

	// テクスチャとマテリアル
	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];
	rootparam[1].DescriptorTable.NumDescriptorRanges = 2;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootparam[2].InitAsDescriptorTable(1, &descTblRange[3]);
	rootparam[3].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);

	// PSOの作成
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;


	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	D3D12_RASTERIZER_DESC rasterizeDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};
	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0] = CD3DX12_STATIC_SAMPLER_DESC(0);
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	samplerDesc[1] = CD3DX12_STATIC_SAMPLER_DESC(0);
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //<=であればtrue(1.0)そうでなければ(0.0)
	samplerDesc[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT; //比較結果をバイリニア補間
	samplerDesc[1].MaxAnisotropy = 1; //深度傾斜を有効に
	samplerDesc[1].ShaderRegister = 1;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparam), rootparam, _countof(samplerDesc), samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* vertexShaderBlob = ShaderManager::GetInstance().GetShader("ShadowVertexShader");

	//PSOの作成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS.BytecodeLength = 0;
	gpipeline.PS.pShaderBytecode = nullptr;

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizeDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = _countof(inputLayout);

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 0;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	ID3DBlob* rootSigBlob;
	ID3DBlob* errorBlob;

	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc,D3D_ROOT_SIGNATURE_VERSION_1_0,&rootSigBlob,&errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&m_ShadowPSO.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = m_ShadowPSO.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&m_ShadowPSO.pso))))
	{
		return false;
	}

	return true;
}

bool MeshDrawer::GenerateViews(std::shared_ptr<Mesh> mesh)
{
	if (!GenerateConstantView(mesh))
	{
		return false;
	}
	if (!GenerateMaterialView(mesh))
	{
		return false;
	}

	return true;
}

bool MeshDrawer::GenerateConstantView(std::shared_ptr<Mesh> mesh)
{
	auto constBuff = mesh->GetConstantBuffer()->getBuffer();

	UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	CPU_GPU_Handles* handles = nullptr;
	bool m_isFind = false;

	for (int i = 0; i < m_DescHandleCount; i++)
	{
		if (!m_ConstantHandles.at(i)->m_UseFlag)
		{
			handles = m_ConstantHandles.at(i);
			m_ConstantHandles.at(i)->m_UseFlag = true;
			m_isFind = true;
			break;
		}
	}

	// 定数バッファのハンドルが余っていないので生成不可
	if (!m_isFind)return false;

	mesh->SetConstantDescHandle(handles);
	auto handle = mesh->GetConstantDescHandle()->m_CPUDescHandle;

	// 行列などの定数データのビューを生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;
	
	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		handle);


	return true;
}

bool MeshDrawer::GenerateMaterialView(std::shared_ptr<Mesh> mesh)
{
	// シェーダーリソースビューの配置の分も計算に考慮しなくてはいけない

	auto materialBuffers = mesh->GetMaterialBuffer();
	auto textures = mesh->GetTextureBuff();
	auto materials = mesh->GetMaterialDatas();

	// ハンドル
	CPU_GPU_Handles* handles = nullptr;
	bool m_isFind = false;

	

	for (int i = 0; i < m_DescHandleCount; i++)
	{
		if (!m_MatHandles.at(i)->m_UseFlag)
		{
			handles = m_MatHandles.at(i);
			m_MatHandles.at(i)->m_UseFlag = true;
			m_isFind = true;
			break;
		}
	}

	// 定数バッファのハンドルが余っていないので生成不可
	if (!m_isFind)return false;

	mesh->SetMatDescHandle(handles);
	auto matDescHeapH = mesh->GetMatDescHandle()->m_CPUDescHandle;

	D3D12_SHADER_RESOURCE_VIEW_DESC srVDesc = {};
	srVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srVDesc.Texture2D.MipLevels = 1;

	for (const auto& mat : materials)
	{
		UINT descHandleInc = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		auto materialBuff = materialBuffers.at(mat.first)->getBuffer();



		D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
		matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
		matCBVDesc.SizeInBytes = materialBuff->GetDesc().Width;

		// マテリアル用のビューを生成
		DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
			&matCBVDesc,
			matDescHeapH
		);

		matDescHeapH.ptr += descHandleInc;

		if (textures.find(mat.first) != textures.end())
		{
			srVDesc.Format = textures.at(mat.first)->GetDesc().Format;
			// テクスチャバッファ用
			DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
				textures.at(mat.first).Get(),
				&srVDesc,
				matDescHeapH);
		}

		matDescHeapH.ptr += descHandleInc;
	}

}

