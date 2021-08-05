#include "FluidRenderingPipeLine.h"

#include "../WindowApp.h"
#include "../DirectX/DirectXGraphics.h"
#include "../DirectX/DirectXDevice.h"
#include "../DirectX/Core/ShaderManager.h"
#include "../DirectX/Core/Buffer.h"
#include "../../Utility/CameraManager.h"
#include "../../Utility/Camera.h"
#include "../../Utility/Math/MathUtility.h"
#include "../../imgui/imgui.h"
#include "../Lights/LightManager.h"
#include "../Lights/DirectionalLight.h"
#include "../../Utility/CSVParser.h"
#include "ParticleManager.h"

FluidRendringPipeLine::FluidRendringPipeLine()
{
	m_default_fluidprop = new  FluidProperty();
	m_opend_fluidprop = m_default_fluidprop;
}

FluidRendringPipeLine::~FluidRendringPipeLine()
{
	if (m_default_fluidprop) {
		delete m_default_fluidprop;
	}

	for (auto itr = m_LoadProps.begin(); itr != m_LoadProps.end(); ++itr)
	{
		delete (*itr);
	}

	m_pLoadData.clear();
}


HRESULT FluidRendringPipeLine::Init()
{
	// シェーダー読み込み

	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidRenderVertexShader.hlsl", "vs_5_0", "PE_FluidVS");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidRenderPixelShader.hlsl", "ps_5_0", "PE_FluidPS");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidBluerVertexShader.hlsl", "vs_5_0", "PE_Fluid_BluerVS");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidBluerPixelShader.hlsl", "ps_5_0", "PE_FluidPS_HorizontalBlur", "HorizontalBlur_Main");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidBluerPixelShader.hlsl", "ps_5_0", "PE_FluidPS_VerticalBlur", "VerticalBlur_Main");

	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidBluerPixelShader.hlsl", "ps_5_0", "PE_FluidPS_HorizontalGausBlur", "HorizontalGausBlur_Main");
	ShaderManager::GetInstance().LoadShader(L"BasicResources/FluidBluerPixelShader.hlsl", "ps_5_0", "PE_FluidPS_VerticalGausBlur", "VerticalGausBlur_Main");



	m_pCommandList = DirectXGraphics::GetInstance().GetCommandList();

	auto result = CreateDepthHeap();
	if (result != S_OK)	return result;

	//result = CreateDepthResources();
	//if (result != S_OK) return result;

	result = CreateRenderResource();
	if (result != S_OK) return result;

	result = CreateRenderHeaps();
	if (result != S_OK) return result;

	result = CreatePeraPolygon();
	if (result != S_OK) return result;

	result = CreatePSO();
	if (result != S_OK) return result;

	result = CreateBluerPSO();
	if (result != S_OK) return result;

	result = CreateDrawPolygonPSO();
	if (result != S_OK) return result;

	result = CreateConstantDeschHeap();
	if (result != S_OK)return result;

	result = CreateConstantBufferAndView();
	if (result != S_OK)return result;

	result = CreateResultBuffer();
	if (result != S_OK)return result;

	return S_OK;
}

//void FluidRendringPipeLine::DrawDepthBegin()
//{
//	auto handle = m_DepthHeaps->GetCPUDescriptorHandleForHeapStart();
//
//	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthBuffer.Get(),
//		D3D12_RESOURCE_STATE_DEPTH_READ,
//		D3D12_RESOURCE_STATE_DEPTH_WRITE));
//
//	// レンダーターゲットセット
//	m_pCommandList->OMSetRenderTargets(0, nullptr, false, &handle);
//	m_pCommandList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//
//	DirectXGraphics::GetInstance().SetScissorRect();
//	DirectXGraphics::GetInstance().SetViewPort();
//}
//
//void FluidRendringPipeLine::DrawDepthEnd()
//{
//	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthBuffer.Get(),
//		D3D12_RESOURCE_STATE_DEPTH_WRITE,
//		D3D12_RESOURCE_STATE_DEPTH_READ));
//}

void FluidRendringPipeLine::ConstBufferUpdate()
{

	auto constBuff = m_ConstDataBuffer->getBuffer();
	auto camera = CameraManager::GetInstance().GetMainCamera();
	SimpleMath::Matrix vpMat = camera->GetViewMat() * camera->GetProjectMat();
	SimpleMath::Matrix v = camera->GetProjectMat();

	XMVECTOR det;

	// ここらへん最適化が必要
	//float _far = camera->GetFar();
	//float _near = camera->GetNear();
	//float fov = camera->GetFov();
	//float aspect = camera->GetAspect();
	//float deg2rad = (MathUtility::TwoPI / 360.0f);

	SimpleMath::Vector3 cameraPos = camera->GetPosition();
	SimpleMath::Vector3 lightDir = LightManager::GetInstance().GetDirectionalLight()->GetDirection();

	//SimpleMath::Vector4 frustumParams;
	//frustumParams.x = -_far * tanf(fov * 0.5f * deg2rad) * aspect; // left
	//frustumParams.y = _far * tanf(fov * 0.5f * deg2rad) * aspect; // right
	//frustumParams.z = -_far * tanf(fov * 0.5f * deg2rad);  //bottom
	//frustumParams.w = _far * tanf(fov * 0.5f * deg2rad); //  top

	ConstDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->Inverse_ProjectMat = XMMatrixInverse(&det, camera->GetProjectMat());
	constMap->Inverse_ViewMat = XMMatrixInverse(&det, camera->GetViewMat());
	constMap->Proj = camera->GetProjectMat();
	constMap->View = camera->GetViewMat();
	//constMap->LightDirection = SimpleMath::Vector4(LightManager::GetInstance().GetDirectionalLight()->GetDirection());
	constBuff->Unmap(0, nullptr);

	auto weights = MathUtility::GetGaussianWeights(8, m_opend_fluidprop->m_gaus_radius);

	// リソースの転送
	float* mappedWeight = nullptr;
	m_bkWeightsResource->Map(0, nullptr, (void**)&mappedWeight);
	std::copy(weights.begin(), weights.end(), mappedWeight);
	m_bkWeightsResource->Unmap(0, nullptr);

	auto blurParamBuffer = m_BlurParamBuffer->getBuffer();
	//auto gaussRadius = GetGaussRadius();
	//auto gaussWeight = GetGausWeights();

	//float MAP_WIDTH = 1920.0f;
	//float MAP_HEIGHT = 1080.0f;
	//
	BlurParams* blurParamsMap = nullptr;
	blurParamBuffer->Map(0, nullptr, (void**)&blurParamsMap);
	blurParamsMap->BluerDepthFallOff = m_opend_fluidprop->m_bilateral_bluerdepth_falloff;
	blurParamsMap->BluerScale = m_opend_fluidprop->m_bilateral_bluer_scale;
	blurParamsMap->FilterRadius = m_opend_fluidprop->m_bilateral_filtter_radius;
	blurParamBuffer->Unmap(0, nullptr);
}

void FluidRendringPipeLine::DrawFluidBegin()
{
	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_FluidRenderResource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	m_pCommandList->OMSetRenderTargets(1, &handle, false, nullptr);
	DirectXGraphics::GetInstance().ClearRenderTargetView(handle);

	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();
}


void FluidRendringPipeLine::DrawFluid(ID3D12DescriptorHeap* pBackTextureDescHeap, ID3D12DescriptorHeap* pCubemapDescHeap, ID3D12DescriptorHeap* pBackDepthTextureDescHeap)
{

	DrawFluidBegin();

	UpdateRenderParams();
	ConstBufferUpdate();


	auto renderSrvHeap = m_RenderSRVHeap.Get();

	m_pCommandList->SetPipelineState(m_FluidRenderingPSO.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(m_FluidRenderingPSO.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	auto csvHandle = m_ConstantDeschHeap->GetGPUDescriptorHandleForHeapStart();
	m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_Matrix, csvHandle);
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 3;

	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_RenderParams, csvHandle);


	m_pCommandList->SetDescriptorHeaps(1, &renderSrvHeap);

	auto srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_DepthMap, srvHandle);

	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_ThickNessMap, srvHandle);

	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_WallTexture, srvHandle);


	m_pCommandList->SetDescriptorHeaps(1, &pBackTextureDescHeap);
	const auto backTextureHandle = pBackTextureDescHeap->GetGPUDescriptorHandleForHeapStart();

	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_BackTexture, backTextureHandle);

	// 静的キューブマップ
	auto cubeMapHandle = pCubemapDescHeap->GetGPUDescriptorHandleForHeapStart();
	cubeMapHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	m_pCommandList->SetDescriptorHeaps(1, &pCubemapDescHeap);
	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_CubeMap, cubeMapHandle);


	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_ResultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_SOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	));
	
	// リザルト用のUAV
	auto resultHandle = m_ResultViewHeap->GetGPUDescriptorHandleForHeapStart();
	m_pCommandList->SetDescriptorHeaps(1, m_ResultViewHeap.GetAddressOf());
	resultHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_ResultBuffer, resultHandle);

	m_pCommandList->DrawInstanced(4, 1, 0, 0);

	DrawFluidEnd();
}

void FluidRendringPipeLine::DrawFluidEnd()
{
	// シェーダー用に変更
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_FluidRenderResource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

bool FluidRendringPipeLine::UpdateGUI()
{
	bool changeFlag = false;

	ImGui::Begin("FluidParameter", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O"))
			{
				OpenFile();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveFile();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	float fluidColor[4] = { m_opend_fluidprop->m_fluid_color_.x,m_opend_fluidprop->m_fluid_color_.y,m_opend_fluidprop->m_fluid_color_.z,m_opend_fluidprop->m_fluid_color_.w };
	float diffuseColor[4] = { m_opend_fluidprop->diffuse_light_.x,m_opend_fluidprop->diffuse_light_.y,m_opend_fluidprop->diffuse_light_.z,m_opend_fluidprop->diffuse_light_.w };
	float specularColor[4] = { m_opend_fluidprop->specular_light_.x, m_opend_fluidprop->specular_light_.y, m_opend_fluidprop->specular_light_.z, m_opend_fluidprop->specular_light_.w };


	if (ImGui::ColorEdit4("FluidColor", fluidColor)) { changeFlag = true; }
	if (ImGui::ColorEdit4("DiffuseLight", diffuseColor)) { changeFlag = true; }
	if (ImGui::ColorEdit4("SpecularLight", specularColor)) { changeFlag = true; }
	if (ImGui::DragFloat("GausRadius", &m_opend_fluidprop->m_gaus_radius, 0.001f)) { changeFlag = true; }
	if (ImGui::DragFloat("Bilateral_BluerScale", &m_opend_fluidprop->m_bilateral_bluer_scale, 0.001f)) { changeFlag = true; }
	if (ImGui::DragFloat("Bilateral_BluerDepthFallOff", &m_opend_fluidprop->m_bilateral_bluerdepth_falloff, 0.001f)) { changeFlag = true; }
	if (ImGui::DragFloat("Bilateral_FillterRadius", &m_opend_fluidprop->m_bilateral_filtter_radius, 0.001f)) { changeFlag = true; }
	if (ImGui::DragFloat("ClampDepth", &m_ClampDepth, 0.0000001f, 0.000001f)) { changeFlag = true; }
	ImGui::End();

	m_opend_fluidprop->m_fluid_color_ = SimpleMath::Vector4(fluidColor);
	m_opend_fluidprop->specular_light_ = SimpleMath::Vector4(specularColor);
	m_opend_fluidprop->diffuse_light_ = SimpleMath::Vector4(diffuseColor);

	return changeFlag;
}

void FluidRendringPipeLine::OpenFile()
{
	auto filePath = WindowApp::GetInstance().FileOpen();

	if (filePath == "")
	{
		return;
	}

	std::vector<std::vector<std::string>> loadData;

	CSVParser::LoadCSV(loadData, filePath);


	// LoadColor
	float fluidColor_r = std::stof(loadData[1][1]);
	float fluidColor_g = std::stof(loadData[1][2]);
	float fluidColor_b = std::stof(loadData[1][3]);
	float fluidColor_a = std::stof(loadData[1][4]);

	float specular_r = std::stof(loadData[2][1]);
	float specular_g = std::stof(loadData[2][2]);
	float specular_b = std::stof(loadData[2][3]);
	float specular_a = std::stof(loadData[2][4]);

	float ambient_r = std::stof(loadData[3][1]);
	float ambient_g = std::stof(loadData[3][2]);
	float ambient_b = std::stof(loadData[3][3]);
	float ambient_a = std::stof(loadData[3][4]);

	float diffuse_r = std::stof(loadData[4][1]);
	float  diffuse_g = std::stof(loadData[4][2]);
	float  diffuse_b = std::stof(loadData[4][3]);
	float  diffuse_a = std::stof(loadData[4][4]);

	m_opend_fluidprop->m_fluid_color_ = SimpleMath::Vector4(fluidColor_r, fluidColor_g, fluidColor_b, fluidColor_a);
	m_opend_fluidprop->specular_light_ = SimpleMath::Vector4(specular_r, specular_g, specular_b, specular_a);
	m_opend_fluidprop->diffuse_light_ = SimpleMath::Vector4(diffuse_r, diffuse_g, diffuse_b, diffuse_a);
	m_opend_fluidprop->ambinet_light_ = SimpleMath::Vector4(ambient_r, ambient_g, ambient_b, ambient_a);

	m_opend_fluidprop->m_gaus_radius = std::stof(loadData[7][1]);
	m_opend_fluidprop->m_bilateral_bluer_scale = std::stof(loadData[7][2]);
	m_opend_fluidprop->m_bilateral_bluerdepth_falloff = std::stof(loadData[7][3]);
	m_opend_fluidprop->m_bilateral_filtter_radius = std::stof(loadData[7][4]);

	// 書き込みフォーマットデータとして保存。
	m_pLoadData = loadData;
}

void FluidRendringPipeLine::SaveFile()
{
	auto filePath = WindowApp::GetInstance().FileOpen();

	// FluidColor
	m_pLoadData[1][1] = std::to_string(m_opend_fluidprop->m_fluid_color_.x);
	m_pLoadData[1][2] = std::to_string(m_opend_fluidprop->m_fluid_color_.y);
	m_pLoadData[1][3] = std::to_string(m_opend_fluidprop->m_fluid_color_.z);
	m_pLoadData[1][4] = std::to_string(m_opend_fluidprop->m_fluid_color_.w);

	// SpecularColor
	m_pLoadData[2][1] = std::to_string(m_opend_fluidprop->specular_light_.x);
	m_pLoadData[2][2] = std::to_string(m_opend_fluidprop->specular_light_.y);
	m_pLoadData[2][3] = std::to_string(m_opend_fluidprop->specular_light_.z);
	m_pLoadData[2][4] = std::to_string(m_opend_fluidprop->specular_light_.w);

	// AmbientColor
	m_pLoadData[3][1] = std::to_string(m_opend_fluidprop->ambinet_light_.x);
	m_pLoadData[3][2] = std::to_string(m_opend_fluidprop->ambinet_light_.y);
	m_pLoadData[3][3] = std::to_string(m_opend_fluidprop->ambinet_light_.z);
	m_pLoadData[3][4] = std::to_string(m_opend_fluidprop->ambinet_light_.w);

	// DiffuseColor
	m_pLoadData[4][1] = std::to_string(m_opend_fluidprop->diffuse_light_.x);
	m_pLoadData[4][2] = std::to_string(m_opend_fluidprop->diffuse_light_.y);
	m_pLoadData[4][3] = std::to_string(m_opend_fluidprop->diffuse_light_.z);
	m_pLoadData[4][4] = std::to_string(m_opend_fluidprop->diffuse_light_.w);

	m_pLoadData[7][1] = std::to_string(m_opend_fluidprop->m_gaus_radius);
	m_pLoadData[7][2] = std::to_string(m_opend_fluidprop->m_bilateral_bluer_scale);
	m_pLoadData[7][3] = std::to_string(m_opend_fluidprop->m_bilateral_bluerdepth_falloff);
	m_pLoadData[7][4] = std::to_string(m_opend_fluidprop->m_bilateral_filtter_radius);

	CSVParser::SaveCSV(m_pLoadData, filePath);
}

bool FluidRendringPipeLine::CreateResultBuffer()
{
	// ヒープの作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; // リザルト用のSRVとUAV もしかしたら、UAVだけでいいかも？
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = DirectXDevice::GetInstance().GetDevice()
		->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_ResultViewHeap));


	ResultBuffer resultbuff;
	resultbuff.wallOnInkCount = 0;
	resultbuff.wallPixelCount = 0;

	UINT dataSize = (sizeof(ResultBuffer) + 0xff) & ~0xff;

	D3D12_HEAP_PROPERTIES defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_HEAP_PROPERTIES uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);

	if (DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&defaultHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_ResultBuffer)
	) != S_OK)
	{
		return false;
	}

	if (DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadResultBuffer)
	) != S_OK)
	{
		return false;
	}


	D3D12_SUBRESOURCE_DATA particleData = {};
	particleData.pData = reinterpret_cast<UINT8*>(&resultbuff);
	particleData.RowPitch = dataSize;
	particleData.SlicePitch = particleData.RowPitch;

	auto commandList = DirectXGraphics::GetInstance().GetCommandList();

	// リソースの転送
	UpdateSubresources<1>(commandList, m_ResultBuffer.Get(), m_UploadResultBuffer.Get(), 0, 0, 1, &particleData);
	// リソースをコピー状態から、ピクセルシェーダー以外のシェーダーで扱うに変更
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ResultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));


	// シェーダーリソースビューの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = 1;
	srvDesc.Buffer.StructureByteStride = sizeof(ResultBuffer);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// SRVを生成
	auto descriptorSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle0(m_ResultViewHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorSize);
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(m_ResultBuffer.Get(), &srvDesc, srvHandle0);

	// UAV（読み書き可のやつ）
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 1; // 要素数
	uavDesc.Buffer.StructureByteStride = sizeof(ResultBuffer);
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle0(m_ResultViewHeap->GetCPUDescriptorHandleForHeapStart(), 1, descriptorSize);
	DirectXDevice::GetInstance().GetDevice()->CreateUnorderedAccessView(m_ResultBuffer.Get(), nullptr, &uavDesc, uavHandle0);


	// 読み戻し用のバッファ
	D3D12_HEAP_PROPERTIES readbackHeapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK) };
	D3D12_RESOURCE_DESC readbackBufferDesc{ CD3DX12_RESOURCE_DESC::Buffer(dataSize) };

	DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&readbackHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&readbackBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_ResultreadbackBuffer));
	
	return true;
}

void FluidRendringPipeLine::DrawDepthMapBegin()
{
	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthMapResource0.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_tickness_resource0.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto depthHandle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	depthHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto ticknessHandle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	ticknessHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * 4;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[2] =
	{
		ticknessHandle,depthHandle
	};

	auto dsvHeap = DirectXGraphics::GetInstance().GetDSVHeap();
	m_pCommandList->OMSetRenderTargets(2, rtvs, false, &dsvHeap->GetCPUDescriptorHandleForHeapStart());

	std::array<float,4> clearColor;
	clearColor[0] = 0.0f;
	clearColor[1] = 0.0f;
	clearColor[2] = 0.0f;
	clearColor[3] = 0.0f;
	
	for (auto& rt : rtvs)
	{
		m_pCommandList->ClearRenderTargetView(rt, clearColor.data(), 0, nullptr);
	}

	//DirectXGraphics::GetInstance().SetRenderTarget(handle);
	//DirectXGraphics::GetInstance().SetRenderTarget(handles);




	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();
}

void FluidRendringPipeLine::DrawDepthMap()
{
	DrawDepthMapBegin();

	ConstBufferUpdate();

	//auto depthHeap = m_DepthSRVHeaps.Get();

	//m_pCommandList->SetPipelineState(m_DepthRenderingPSO.pso.Get());
	//m_pCommandList->SetGraphicsRootSignature(m_FluidRenderingPSO.rootSignature.Get());

	//m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	//auto csvHandle = m_ConstantDeschHeap->GetGPUDescriptorHandleForHeapStart();
	//m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
	//m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_Matrix, csvHandle);

	//auto srvHandle = m_DepthSRVHeaps->GetGPUDescriptorHandleForHeapStart();
	//m_pCommandList->SetDescriptorHeaps(1, &depthHeap);
	//m_pCommandList->SetGraphicsRootDescriptorTable(RootParamterIndex_DepthTex, srvHandle);

	//m_pCommandList->DrawInstanced(4, 1, 0, 0);

	ParticleManager::GetInstance().ZbufferDraw();


	DrawDepthMapEnd();
}

void FluidRendringPipeLine::DrawDepthMapEnd()
{
	// シェーダー用に変更
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthMapResource0.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_tickness_resource0.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void FluidRendringPipeLine::DrawPolygon()
{
	m_pCommandList->SetGraphicsRootSignature(m_PeraPolygonRenderingPSO.rootSignature.Get());
	m_pCommandList->SetPipelineState(m_PeraPolygonRenderingPSO.pso.Get());
	m_pCommandList->SetDescriptorHeaps(1, m_RenderSRVHeap.GetAddressOf());

	auto srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);


	m_pCommandList->IASetPrimitiveTopology(
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);
	m_pCommandList->DrawInstanced(4, 1, 0, 0);
}

void FluidRendringPipeLine::DrawWallMapBegin()
{
	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_WallRenderResource.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//DirectXGraphics::GetInstance().SetRenderTarget(handle, false);

	auto dsvHeaps = DirectXGraphics::GetInstance().GetDSVHeap();
	auto dsvH = dsvHeaps->GetCPUDescriptorHandleForHeapStart();
	dsvH.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvH.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	
	m_pCommandList->OMSetRenderTargets(1, &handle, false, &dsvH);

	DirectXGraphics::GetInstance().ClearRenderTargetView(handle);
	m_pCommandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();
}

void FluidRendringPipeLine::DrawWallMapEnd()
{
	// シェーダー用に変更
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_WallRenderResource.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

void FluidRendringPipeLine::UpdateResultData()
{
	UINT dataSize = (sizeof(ResultBuffer) + 0xff) & ~0xff;
	
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_ResultBuffer.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COPY_SOURCE
	));


	m_pCommandList->CopyResource(m_ResultreadbackBuffer.Get(), m_ResultBuffer.Get());

	D3D12_RANGE readbackBufferRange{ 0,dataSize };
	ResultBuffer* pReadBackBufferData{};

	if(m_ResultreadbackBuffer->Map(
		0,
		&readbackBufferRange,
		reinterpret_cast<void**>(&pReadBackBufferData)
	) == S_OK)
	{
		D3D12_RANGE emptyRange{ 0,0 };
		m_ResultreadbackBuffer->Unmap(
			0,
			&emptyRange
		);

		m_ResultBufferData.wallOnInkCount = pReadBackBufferData->wallOnInkCount;
		m_ResultBufferData.wallPixelCount = pReadBackBufferData->wallPixelCount;


		ResultBuffer defBuffer;
		defBuffer.wallOnInkCount = 0;
		defBuffer.wallPixelCount = 0;

		D3D12_SUBRESOURCE_DATA particleData = {};
		particleData.pData = reinterpret_cast<UINT8*>(&defBuffer);
		particleData.RowPitch = dataSize;
		particleData.SlicePitch = particleData.RowPitch;

		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ResultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
		// リソースの転送
		UpdateSubresources<1>(m_pCommandList, m_ResultBuffer.Get(), m_UploadResultBuffer.Get(), 0, 0, 1, &particleData);
		// リソースをコピー状態から、ピクセルシェーダー以外のシェーダーで扱うに変更
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ResultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE));
		
	}

}


void FluidRendringPipeLine::UpdateRenderParams()
{
	const bool changeFlag = UpdateGUI();

	//if (!changeFlag)return;

	auto cameraPos = CameraManager::GetInstance().GetMainCamera()->GetPosition();
	auto lightDir = LightManager::GetInstance().GetDirectionalLight()->GetDirection();

	auto constBuff = m_RenderingParamsBuffer->getBuffer();
	RenderingParams* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->DiffuseLight = m_opend_fluidprop->diffuse_light_;
	constMap->SpecularLight = m_opend_fluidprop->specular_light_;
	constMap->FluidColor = m_opend_fluidprop->m_fluid_color_;
	constMap->EyePos = SimpleMath::Vector4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
	constMap->LightDirection = SimpleMath::Vector4(lightDir.x, lightDir.y, lightDir.z, 1.0f);
	constMap->ClampDepth = m_ClampDepth;
	//constMap->LightDirection = SimpleMath::Vector4(LightManager::GetInstance().GetDirectionalLight()->GetDirection());
	constBuff->Unmap(0, nullptr);
}

HRESULT FluidRendringPipeLine::CreateDepthHeap()
{

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1; //通常深度バッファ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	auto result = DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(&m_DepthHeaps)
	);


	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;


	result = DirectXDevice::GetInstance().GetDevice()
		->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DepthSRVHeaps));


	return result;
}

HRESULT FluidRendringPipeLine::CreateConstantDeschHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 4; // 行列/ガウスブラー用/バイラテラルパラメータ/レンダリングパラメータ

	auto result = DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(&descHeapDesc,
		IID_PPV_ARGS(&m_ConstantDeschHeap));

	return result;
}

//HRESULT FluidRendringPipeLine::CreateDepthResources()
//{
//	// 深度バッファ作成
//	D3D12_RESOURCE_DESC depthResDesc{};
//	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	depthResDesc.Width = WindowApp::GetInstance().GetWindowSize().window_Width;
//	depthResDesc.Height = WindowApp::GetInstance().GetWindowSize().window_Height;
//	depthResDesc.DepthOrArraySize = 1;
//	depthResDesc.Format = DXGI_FORMAT_R32_TYPELESS;
//	depthResDesc.SampleDesc.Count = 1;
//	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//
//	D3D12_HEAP_PROPERTIES depthHeapProp{};
//	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
//	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//
//	D3D12_CLEAR_VALUE depthClearValue{};
//	depthClearValue.DepthStencil.Depth = 1.0f;
//	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
//
//	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
//		&depthHeapProp,
//		D3D12_HEAP_FLAG_NONE,
//		&depthResDesc,
//		D3D12_RESOURCE_STATE_DEPTH_READ,
//		&depthClearValue,
//		IID_PPV_ARGS(&m_DepthBuffer)
//	);
//
//	m_DepthBuffer->SetName(L"FluidDepthResource");
//
//	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
//	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
//	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
//	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
//
//	// DepthStencilView
//	DirectXDevice::GetInstance().GetDevice()->CreateDepthStencilView(
//		m_DepthBuffer.Get(),
//		&dsvDesc,
//		m_DepthHeaps->GetCPUDescriptorHandleForHeapStart()
//	);
//
//	// ビューの生成
//	D3D12_SHADER_RESOURCE_VIEW_DESC resDesc = {};
//	resDesc.Format = DXGI_FORMAT_R32_FLOAT;
//	resDesc.Texture2D.MipLevels = 1;
//	resDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	resDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//
//	auto handle = m_DepthSRVHeaps->GetCPUDescriptorHandleForHeapStart();
//	m_DepthSRVHeaps->SetName(L"DepthSRVHeaps");
//
//	// シェーダーリソース用
//	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
//		m_DepthBuffer.Get(),
//		&resDesc,
//		handle);
//
//	return result;
//}

HRESULT FluidRendringPipeLine::CreateRenderResource()
{
	auto heapDesc = DirectXGraphics::GetInstance().GetRTVHeap()->GetDesc();

	auto& backbuff = DirectXGraphics::GetInstance().GetBackBuffers()[0];
	auto resDesc = backbuff->GetDesc();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	auto clearColor = DirectXGraphics::GetInstance().GetClearColor();

	std::array<float, 4> clearColorArray;
	clearColorArray[0] = clearColor.R();
	clearColorArray[1] = clearColor.G();
	clearColorArray[2] = clearColor.B();
	clearColorArray[3] = clearColor.A();

	D3D12_CLEAR_VALUE claeValue = CD3DX12_CLEAR_VALUE(
		DXGI_FORMAT_R8G8B8A8_UNORM, clearColorArray.data());

	D3D12_HEAP_PROPERTIES heapProp =
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	// リソースの生成
	auto result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_FluidRenderResource.ReleaseAndGetAddressOf()));

	// 壁だけを描画したリソース
	 result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_WallRenderResource.ReleaseAndGetAddressOf()));
	
	resDesc.Format = DXGI_FORMAT_R32_FLOAT;

	std::array<float, 1> clearColorArray1;
	clearColorArray[0] = clearColor.R();


	claeValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R32_FLOAT, clearColorArray1.data());

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_DepthMapResource0.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_DepthMapResource1.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_DepthMapResource2.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_tickness_resource0.ReleaseAndGetAddressOf()));

	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_tickness_resource1.ReleaseAndGetAddressOf()));


	result = DirectXDevice::GetInstance().GetDevice()
		->CreateCommittedResource(&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			&claeValue,
			IID_PPV_ARGS(m_tickness_resource2.ReleaseAndGetAddressOf()));


	m_WallRenderResource->SetName(L"WallRenderResource");
	m_FluidRenderResource->SetName(L"FluidRenderResource");
	m_DepthMapResource0->SetName(L"DepthMapResource0");
	m_DepthMapResource1->SetName(L"DepthMapResource1");
	m_DepthMapResource2->SetName(L"DepthMapResource2");
	m_tickness_resource0->SetName(L"ThicknessResource0");
	m_tickness_resource1->SetName(L"ThicknessResource1");
	m_tickness_resource2->SetName(L"ThicknessResource2");

	return result;
}

HRESULT FluidRendringPipeLine::CreateRenderHeaps()
{
	auto heapDesc = DirectXGraphics::GetInstance().GetRTVHeap()->GetDesc();
	heapDesc.NumDescriptors = 8;

	// デスクリプタの生成
	auto result = DirectXDevice::GetInstance().GetDevice()
		->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_RenderRTVHeap.ReleaseAndGetAddressOf()));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	auto handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_FluidRenderResource.Get(),
		&rtvDesc,
		handle
	);

	rtvDesc.Format = DXGI_FORMAT_R32_FLOAT;

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_DepthMapResource0.Get(),
		&rtvDesc,
		handle
	);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_DepthMapResource1.Get(),
		&rtvDesc,
		handle
	);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_DepthMapResource2.Get(),
		&rtvDesc,
		handle
	);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_tickness_resource0.Get(),
		&rtvDesc,
		handle
	);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_tickness_resource1.Get(),
		&rtvDesc,
		handle
	);


	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// レンダーターゲットビューの作成
	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_tickness_resource2.Get(),
		&rtvDesc,
		handle
	);



	// 壁描画用のリソース
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	DirectXDevice::GetInstance().GetDevice()->CreateRenderTargetView(
		m_WallRenderResource.Get(),
		&rtvDesc,
		handle
	);


	// SRV用ヒープの作成
	heapDesc.NumDescriptors = 8;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = DirectXDevice::GetInstance().GetDevice()->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(m_RenderSRVHeap.ReleaseAndGetAddressOf()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


	handle = m_RenderSRVHeap->GetCPUDescriptorHandleForHeapStart();

	//　SRVの作成
	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_FluidRenderResource.Get(),
		&srvDesc,
		handle);

	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_DepthMapResource0.Get(),
		&srvDesc,
		handle);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_DepthMapResource1.Get(),
		&srvDesc,
		handle);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_DepthMapResource2.Get(),
		&srvDesc,
		handle);


	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_tickness_resource0.Get(),
		&srvDesc,
		handle);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_tickness_resource1.Get(),
		&srvDesc,
		handle);

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_tickness_resource2.Get(),
		&srvDesc,
		handle);


	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		m_WallRenderResource.Get(),
		&srvDesc,
		handle);

	return result;
}

HRESULT FluidRendringPipeLine::CreatePSO()
{
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;

	D3D12_DESCRIPTOR_RANGE range[8] = {};

	// DepthMapTexture
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;

	// MatrixConstBuffer
	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[1].BaseShaderRegister = 0;
	range[1].NumDescriptors = 1;

	// thicknessTexture
	range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[2].BaseShaderRegister = 1;
	range[2].NumDescriptors = 1;

	// backTexture
	range[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[3].BaseShaderRegister = 2;
	range[3].NumDescriptors = 1;

	// cubeMap
	range[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[4].BaseShaderRegister = 3;
	range[4].NumDescriptors = 1;

	// renderParams
	range[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[5].BaseShaderRegister = 1;
	range[5].NumDescriptors = 1;

	// renderParams
	range[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[6].BaseShaderRegister = 4;
	range[6].NumDescriptors = 1;

	// 壁とインクのリザルト
	range[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	range[7].BaseShaderRegister = 1;
	range[7].NumDescriptors = 1;

	D3D12_ROOT_PARAMETER rp[8] = {};

	rp[RootParamterIndex_DepthMap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_DepthMap].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_DepthMap].DescriptorTable.pDescriptorRanges = &range[0];
	rp[RootParamterIndex_DepthMap].DescriptorTable.NumDescriptorRanges = 1;

	rp[RootParamterIndex_Matrix].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_Matrix].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_Matrix].DescriptorTable.pDescriptorRanges = &range[1];
	rp[RootParamterIndex_Matrix].DescriptorTable.NumDescriptorRanges = 1;


	rp[RootParamterIndex_ThickNessMap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_ThickNessMap].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_ThickNessMap].DescriptorTable.pDescriptorRanges = &range[2];
	rp[RootParamterIndex_ThickNessMap].DescriptorTable.NumDescriptorRanges = 1;

	rp[RootParamterIndex_BackTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_BackTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_BackTexture].DescriptorTable.pDescriptorRanges = &range[3];
	rp[RootParamterIndex_BackTexture].DescriptorTable.NumDescriptorRanges = 1;

	rp[RootParamterIndex_CubeMap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_CubeMap].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_CubeMap].DescriptorTable.pDescriptorRanges = &range[4];
	rp[RootParamterIndex_CubeMap].DescriptorTable.NumDescriptorRanges = 1;

	rp[RootParamterIndex_RenderParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_RenderParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_RenderParams].DescriptorTable.pDescriptorRanges = &range[5];
	rp[RootParamterIndex_RenderParams].DescriptorTable.NumDescriptorRanges = 1;

	rp[RootParamterIndex_WallTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_WallTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_WallTexture].DescriptorTable.pDescriptorRanges = &range[6];
	rp[RootParamterIndex_WallTexture].DescriptorTable.NumDescriptorRanges = 1;

	rp[RootParamterIndex_ResultBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[RootParamterIndex_ResultBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[RootParamterIndex_ResultBuffer].DescriptorTable.pDescriptorRanges = &range[7];
	rp[RootParamterIndex_ResultBuffer].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;


	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(rp);
	rsDesc.pParameters = rp;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rsBlob;
	ComPtr<ID3DBlob> errBlob;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());

	result = DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(m_FluidRenderingPSO.rootSignature.ReleaseAndGetAddressOf()));


	// 最終レンダリング結果用のパイプライン
	auto vs = ShaderManager::GetInstance().GetShader("PE_FluidVS");
	auto ps = ShaderManager::GetInstance().GetShader("PE_FluidPS");


	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;

	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.BlendState = blendDesc;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	gpsDesc.pRootSignature = m_FluidRenderingPSO.rootSignature.Get();

	// パイプライン生成
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(m_FluidRenderingPSO.pso.ReleaseAndGetAddressOf()));

	return result;
}

HRESULT FluidRendringPipeLine::CreateBluerPSO()
{
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;

	D3D12_DESCRIPTOR_RANGE range[3] = {};

	// tex
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;

	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[1].BaseShaderRegister = 0;
	range[1].NumDescriptors = 1;

	range[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[2].BaseShaderRegister = 1;
	range[2].NumDescriptors = 1;


	D3D12_ROOT_PARAMETER rp[2] = {};

	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[0].DescriptorTable.pDescriptorRanges = &range[0];
	rp[0].DescriptorTable.NumDescriptorRanges = 1;

	rp[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[1].DescriptorTable.pDescriptorRanges = &range[1];
	rp[1].DescriptorTable.NumDescriptorRanges = 2;

	//rp[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//rp[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	//rp[2].DescriptorTable.pDescriptorRanges = &range[2];
	//rp[2].DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;


	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(rp);
	rsDesc.pParameters = rp;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rsBlob;
	ComPtr<ID3DBlob> errBlob;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());

	result = DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(m_DepthHorizontalBluarPSO.rootSignature.ReleaseAndGetAddressOf()));

	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	renderTargetBlendDesc.BlendEnable = false;

	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;

	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	auto vs = ShaderManager::GetInstance().GetShader("PE_Fluid_BluerVS");


	blendDesc.RenderTarget[0] = renderTargetBlendDesc;
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.BlendState = blendDesc;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT; // 通常カラー
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	gpsDesc.pRootSignature = m_DepthHorizontalBluarPSO.rootSignature.Get();





	// ブラー用
	auto ps = ShaderManager::GetInstance().GetShader("PE_FluidPS_HorizontalBlur");
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

	// パイプライン生成
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(m_DepthHorizontalBluarPSO.pso.ReleaseAndGetAddressOf()));


	ps = ShaderManager::GetInstance().GetShader("PE_FluidPS_VerticalBlur");
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

	// パイプライン生成
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(m_DepthVerticalBluarPSO.pso.ReleaseAndGetAddressOf()));


	// ここからthickness用ガウスブラー
	ps = ShaderManager::GetInstance().GetShader("PE_FluidPS_HorizontalGausBlur");
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

	// パイプライン生成
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(m_DepthHorizontalGausBluarPSO.pso.ReleaseAndGetAddressOf()));

	ps = ShaderManager::GetInstance().GetShader("PE_FluidPS_VerticalGausBlur");
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

	// パイプライン生成
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(m_DepthVerticalGausBluarPSO.pso.ReleaseAndGetAddressOf()));


	return result;
}

HRESULT FluidRendringPipeLine::CreatePeraPolygon()
{
	struct PeraVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	PeraVertex pv[4] = { {{-1,-1,0.1},{0,1}},
						{{-1,1,0.1},{0,0}},
						{{1,-1,0.1},{1,1}},
						{{1,1,0.1},{1,0}} };


	// 頂点バッファの準備
	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(pv)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_peraVB.ReleaseAndGetAddressOf()));



	PeraVertex* mappedPera = nullptr;
	m_peraVB->Map(0, nullptr, (void**)&mappedPera);
	std::copy(std::begin(pv), std::end(pv), mappedPera);

	m_peraVB->Unmap(0, nullptr);

	m_PeraVBV.BufferLocation = m_peraVB->GetGPUVirtualAddress();
	m_PeraVBV.SizeInBytes = sizeof(pv);
	m_PeraVBV.StrideInBytes = sizeof(PeraVertex);


	return result;
}

HRESULT FluidRendringPipeLine::CreateDrawPolygonPSO()
{
	D3D12_INPUT_ELEMENT_DESC layout[2] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.InputLayout.NumElements = _countof(layout);
	gpsDesc.InputLayout.pInputElementDescs = layout;

	D3D12_DESCRIPTOR_RANGE range[1] = {};

	// 描画テクスチャ
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].NumDescriptors = 1;


	D3D12_ROOT_PARAMETER rp[1] = {};

	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rp[0].DescriptorTable.pDescriptorRanges = &range[0];
	rp[0].DescriptorTable.NumDescriptorRanges = 1;



	D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(0);
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;


	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(rp);
	rsDesc.pParameters = rp;
	rsDesc.pStaticSamplers = &sampler;
	rsDesc.NumStaticSamplers = 1;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rsBlob;
	ComPtr<ID3DBlob> errBlob;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf());

	result = DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(
		0,
		rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(m_PeraPolygonRenderingPSO.rootSignature.ReleaseAndGetAddressOf()));


	// シェーダー
	auto vs = ShaderManager::GetInstance().GetShader("PE_PeraPolygonVS");
	auto ps = ShaderManager::GetInstance().GetShader("PE_PeraPolygonPS");

	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vs);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(ps);
	gpsDesc.DepthStencilState.DepthEnable = false;
	gpsDesc.DepthStencilState.StencilEnable = false;
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 通常カラー
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleDesc.Quality = 0;
	gpsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	gpsDesc.pRootSignature = m_PeraPolygonRenderingPSO.rootSignature.Get();

	// パイプライン生成
	result = DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(
		&gpsDesc, IID_PPV_ARGS(m_PeraPolygonRenderingPSO.pso.ReleaseAndGetAddressOf()));


	return result;
}

HRESULT FluidRendringPipeLine::CreateConstantBufferAndView()
{
#pragma region ガウス
	auto weights = MathUtility::GetGaussianWeights(8, 4.0f);
	int size = (sizeof(weights[0]) * weights.size() + 0xff) & ~0xff;
	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_bkWeightsResource.ReleaseAndGetAddressOf()));


	// リソースの転送
	float* mappedWeight = nullptr;
	m_bkWeightsResource->Map(0, nullptr, (void**)&mappedWeight);
	std::copy(weights.begin(), weights.end(), mappedWeight);
	m_bkWeightsResource->Unmap(0, nullptr);

#pragma endregion

	//float MAP_WIDTH = 1920;
	//float MAP_HEIGHT = 1080;

	m_BlurParamBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(m_BlurParamBuffer) + 0xff) & ~0xff;
	m_BlurParamBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	auto blurParamBuffer = m_BlurParamBuffer->getBuffer();
	auto gaussWeight = MathUtility::GetGaussianWeights(8, 5.0f);

	BlurParams* blurParamsMap = nullptr;
	blurParamBuffer->Map(0, nullptr, (void**)&blurParamsMap);
	blurParamsMap->BluerScale = m_opend_fluidprop->m_bilateral_bluer_scale;
	blurParamsMap->BluerDepthFallOff = m_opend_fluidprop->m_bilateral_bluerdepth_falloff;
	blurParamsMap->FilterRadius = m_opend_fluidprop->m_bilateral_filtter_radius;
	blurParamBuffer->Unmap(0, nullptr);


#pragma region 定数バッファの生成
	m_ConstDataBuffer = std::make_shared<Buffer>();
	buffSize = (sizeof(m_ConstDataBuffer) + 0xff) & ~0xff;
	m_ConstDataBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto constBuff = m_ConstDataBuffer->getBuffer();

	auto camera = CameraManager::GetInstance().GetMainCamera();
	SimpleMath::Matrix vpMat = camera->GetViewMat() * camera->GetProjectMat();

	ConstDatas* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	//constMap->Proj = camera->GetProjectMat();
	//constMap->View = camera->GetViewMat();
	constBuff->Unmap(0, nullptr);


	// 行列などの定数データのビューを生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;

	auto handle = m_ConstantDeschHeap.Get()->GetCPUDescriptorHandleForHeapStart();

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		handle);


	m_RenderingParamsBuffer = std::make_shared<Buffer>();
	buffSize = (sizeof(RenderingParams) + 0xff) & ~0xff;
	m_RenderingParamsBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);
	auto renderingBuff = m_RenderingParamsBuffer->getBuffer();

	// 初期値の代入は後で

#pragma endregion 


	// ガウスブラー
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	cbvDesc.BufferLocation = m_bkWeightsResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)m_bkWeightsResource->GetDesc().Width;

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		handle);

	// バイラテラルパラメータ
	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	cbvDesc.BufferLocation = blurParamBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)blurParamBuffer->GetDesc().Width;

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		handle);


	// レンダリングパラメータのView生成
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc_rpb{};
	cbvDesc_rpb.BufferLocation = renderingBuff->GetGPUVirtualAddress();
	cbvDesc_rpb.SizeInBytes = (UINT)renderingBuff->GetDesc().Width;

	handle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DirectXDevice::GetInstance().GetDevice()->CreateConstantBufferView(
		&cbvDesc_rpb,
		handle);


	return S_OK;
}

//float FluidRendringPipeLine::GetGaussRadius()
//{
//	//return (-0.5f / (m_fRadiusL * m_fRadiusL));
//}
//
//// 参考
//// https://t-pot.com/program/132_BilateralFilter/index.html
//SimpleMath::Vector4 FluidRendringPipeLine::GetGausWeights()
//{
//	float radius = m_gaus_radius;
//	float dispersion = radius * radius;
//
//	const unsigned int WEIGHT_MUN = 4;
//
//	float tbl[WEIGHT_MUN];
//
//	for (int i = 0; i < WEIGHT_MUN; ++i)
//	{
//		tbl[i] = 2.0f * expf(-0.5f * ((2.0f * (float)i + 1.5f) * (2.0f * (float)i + 1.5f)) / dispersion);
//	}
//
//
//	return SimpleMath::Vector4(tbl[0], tbl[1], tbl[2], tbl[3]);
//}

HRESULT FluidRendringPipeLine::SmoothDepthMap()
{
	ImGui::Begin("Blur");
	ImGui::DragInt("BlurCount", &m_BlurCount);
	ImGui::End();

#pragma region 一回目のブラー

#pragma region HorizontalBlur
	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthMapResource1.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//DirectXGraphics::GetInstance().SetRenderTarget(handle, false);

	auto dsvH = DirectXGraphics::GetInstance().GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_pCommandList->OMSetRenderTargets(1, &handle, false, nullptr);
	DirectXGraphics::GetInstance().ClearRenderTargetView(handle);




	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();




	auto srvHeap = m_RenderSRVHeap.Get();

	m_pCommandList->SetPipelineState(m_DepthHorizontalBluarPSO.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(m_DepthHorizontalBluarPSO.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	// ブラーWeightをセット
	auto csvHandle = m_ConstantDeschHeap->GetGPUDescriptorHandleForHeapStart();
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
	m_pCommandList->SetGraphicsRootDescriptorTable(1, csvHandle);

	// DepthMap0をリソースとして
	auto srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetDescriptorHeaps(1, &srvHeap);
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	m_pCommandList->DrawInstanced(4, 1, 0, 0);
#pragma endregion 

#pragma region VerticalBlur
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthMapResource1.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthMapResource2.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_pCommandList->OMSetRenderTargets(1, &handle, false, &dsvH);
	DirectXGraphics::GetInstance().ClearRenderTargetView(handle);

	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();


	m_pCommandList->SetPipelineState(m_DepthVerticalBluarPSO.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(m_DepthHorizontalBluarPSO.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	// ブラーWeightをセット
	csvHandle = m_ConstantDeschHeap->GetGPUDescriptorHandleForHeapStart();
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
	m_pCommandList->SetGraphicsRootDescriptorTable(1, csvHandle);

	// DepthMap1をリソースとして
	srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetDescriptorHeaps(1, &srvHeap);
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	m_pCommandList->DrawInstanced(4, 1, 0, 0);



	// シェーダー用に変更
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthMapResource2.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

#pragma endregion

#pragma endregion

	for (int i = 0; i < m_BlurCount; ++i)
	{
#pragma region HorizontalBlur

		// 前回の結果を使って書き込む。
		m_pCommandList->ResourceBarrier(
			1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_DepthMapResource1.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += DirectXDevice::GetInstance().GetDevice()
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		handle.ptr += DirectXDevice::GetInstance().GetDevice()
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		m_pCommandList->OMSetRenderTargets(1, &handle, false, nullptr);
		DirectXGraphics::GetInstance().ClearRenderTargetView(handle);

		DirectXGraphics::GetInstance().SetScissorRect();
		DirectXGraphics::GetInstance().SetViewPort();

		m_pCommandList->SetPipelineState(m_DepthHorizontalBluarPSO.pso.Get());
		m_pCommandList->SetGraphicsRootSignature(m_DepthHorizontalBluarPSO.rootSignature.Get());

		m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

		// ブラーWeightをセット
		m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
		m_pCommandList->SetGraphicsRootDescriptorTable(1, csvHandle);

		// DepthMap2をリソースとして
		srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		m_pCommandList->SetDescriptorHeaps(1, &srvHeap);
		m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

		m_pCommandList->DrawInstanced(4, 1, 0, 0);
#pragma endregion 

#pragma region VerticalBlur
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthMapResource1.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		m_pCommandList->ResourceBarrier(
			1, &CD3DX12_RESOURCE_BARRIER::Transition(
				m_DepthMapResource2.Get(),
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET));

		handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += DirectXDevice::GetInstance().GetDevice()
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		handle.ptr += DirectXDevice::GetInstance().GetDevice()
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		handle.ptr += DirectXDevice::GetInstance().GetDevice()
			->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		m_pCommandList->OMSetRenderTargets(1, &handle, false, &dsvH);
		DirectXGraphics::GetInstance().ClearRenderTargetView(handle);

		DirectXGraphics::GetInstance().SetScissorRect();
		DirectXGraphics::GetInstance().SetViewPort();


		m_pCommandList->SetPipelineState(m_DepthVerticalBluarPSO.pso.Get());
		m_pCommandList->SetGraphicsRootSignature(m_DepthHorizontalBluarPSO.rootSignature.Get());

		m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

		// ブラーWeightをセット
		m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
		m_pCommandList->SetGraphicsRootDescriptorTable(1, csvHandle);

		// DepthMap1をリソースとして
		srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		m_pCommandList->SetDescriptorHeaps(1, &srvHeap);
		m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

		m_pCommandList->DrawInstanced(4, 1, 0, 0);



		// シェーダー用に変更
		m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DepthMapResource2.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

#pragma endregion

	}


	return S_OK;

}

HRESULT FluidRendringPipeLine::SmoothThicknesshMap()
{
	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_tickness_resource1.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	// ひどいかきかたなのであとでなおす
	auto handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	auto dsvH = DirectXGraphics::GetInstance().GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
	m_pCommandList->OMSetRenderTargets(1, &handle, false, &dsvH);
	DirectXGraphics::GetInstance().ClearRenderTargetView(handle);

	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();




	auto srvHeap = m_RenderSRVHeap.Get();

	m_pCommandList->SetPipelineState(m_DepthHorizontalGausBluarPSO.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(m_DepthHorizontalBluarPSO.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	// ブラーWeightをセット
	auto csvHandle = m_ConstantDeschHeap->GetGPUDescriptorHandleForHeapStart();
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
	m_pCommandList->SetGraphicsRootDescriptorTable(1, csvHandle);

	// thickness0をリソースとして
	auto srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	m_pCommandList->SetDescriptorHeaps(1, &srvHeap);
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	m_pCommandList->DrawInstanced(4, 1, 0, 0);


#pragma region VerticalBlur
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_tickness_resource1.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	m_pCommandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_tickness_resource2.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	handle = m_RenderRTVHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	handle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_pCommandList->OMSetRenderTargets(1, &handle, false, &dsvH);
	DirectXGraphics::GetInstance().ClearRenderTargetView(handle);
	DirectXGraphics::GetInstance().SetScissorRect();
	DirectXGraphics::GetInstance().SetViewPort();


	m_pCommandList->SetPipelineState(m_DepthVerticalGausBluarPSO.pso.Get());
	m_pCommandList->SetGraphicsRootSignature(m_DepthHorizontalBluarPSO.rootSignature.Get());

	m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pCommandList->IASetVertexBuffers(0, 1, &m_PeraVBV);

	// ブラーWeightをセット
	csvHandle = m_ConstantDeschHeap->GetGPUDescriptorHandleForHeapStart();
	csvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pCommandList->SetDescriptorHeaps(1, m_ConstantDeschHeap.GetAddressOf());
	m_pCommandList->SetGraphicsRootDescriptorTable(1, csvHandle);

	// 1をリソースとして
	srvHandle = m_RenderSRVHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	srvHandle.ptr += DirectXDevice::GetInstance().GetDevice()
		->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_pCommandList->SetDescriptorHeaps(1, &srvHeap);
	m_pCommandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	m_pCommandList->DrawInstanced(4, 1, 0, 0);



	// シェーダー用に変更
	m_pCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_tickness_resource2.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

#pragma endregion

	return S_OK;
}

ID3D12DescriptorHeap* FluidRendringPipeLine::GetFluidRenderDescriptorHeaps()
{
	return m_RenderSRVHeap.Get();
}

const FluidRendringPipeLine::ResultBuffer& FluidRendringPipeLine::GetResultBufferData()
{
	return m_ResultBufferData;
}

