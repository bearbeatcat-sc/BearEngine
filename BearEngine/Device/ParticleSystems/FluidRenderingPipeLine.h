#ifndef _FLUID_RENDRING_PIPELINE_H_
#define _FLUID_RENDRING_PIPELINE_H_

#include "../Singleton.h"
#include "../DirectX/Core/PSOManager.h"
#include "../DirectX/DirectXDevice.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <SimpleMath.h>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class DirectXGraphics;
class Buffer;
class SkyBox;

class FluidRendringPipeLine
	:public Singleton< FluidRendringPipeLine>
{
private:
	struct ConstDatas
	{
		DirectX::SimpleMath::Matrix Inverse_ProjectMat;
		DirectX::SimpleMath::Matrix Inverse_ViewMat;
		DirectX::SimpleMath::Matrix Proj;
		DirectX::SimpleMath::Matrix View;
	};

	struct RenderingParams
	{
		SimpleMath::Vector4 FluidColor;
		SimpleMath::Vector4 SpecularLight;
		SimpleMath::Vector4 AmbinetLight;
		SimpleMath::Vector4 DiffuseLight;
		DirectX::SimpleMath::Vector4 LightDirection;
		DirectX::SimpleMath::Vector4 EyePos;
		float ClampDepth;
	};

	struct BlurParams
	{
		float BluerScale;
		float BluerDepthFallOff;
		float FilterRadius;
	};

	struct ResultBuffer
	{
		int wallPixelCount;
		int wallOnInkCount;
	};
	
public:
	
	friend class Singleton<FluidRendringPipeLine>;	
	FluidRendringPipeLine();
	~FluidRendringPipeLine();
	HRESULT Init();

	//void DrawDepthBegin();
	//void DrawDepthEnd();

	void DrawFluid(ID3D12DescriptorHeap* pBackTextureDescHeap, ID3D12DescriptorHeap* pCubemapDescHeap, ID3D12DescriptorHeap* pBackDepthTextureDescHeap);

	HRESULT SmoothDepthMap();
	HRESULT SmoothThicknesshMap();
	ID3D12DescriptorHeap* GetFluidRenderDescriptorHeaps();
	const ResultBuffer& GetResultBufferData();

	void DrawDepthMapBegin();
	void DrawDepthMap();
	void DrawDepthMapEnd();
	void DrawPolygon();
	
	void DrawWallMapBegin();
	void DrawWallMapEnd();
	void UpdateResultData();

private:

	void UpdateRenderParams();
	void ConstBufferUpdate();
	void DrawFluidBegin();
	void DrawFluidEnd();
	bool UpdateGUI();
	void OpenFile();	
	void SaveFile();

	bool CreateResultBuffer();

	HRESULT CreateDepthHeap();
	HRESULT CreateConstantDeschHeap();
	//HRESULT CreateDepthResources();
	HRESULT CreateRenderResource();
	HRESULT CreateRenderHeaps();
	HRESULT CreatePSO();
	HRESULT CreateBluerPSO();
	HRESULT CreatePeraPolygon();
	HRESULT CreateDrawPolygonPSO();
	HRESULT CreateConstantBufferAndView();
	

	//float GetGaussRadius();
	//SimpleMath::Vector4 GetGausWeights();

private:
	ID3D12GraphicsCommandList* m_pCommandList;

private:



	ComPtr<ID3D12Resource> m_ResultBuffer;
	ResultBuffer m_ResultBufferData;

	ComPtr<ID3D12Resource> m_UploadResultBuffer;
	ComPtr<ID3D12Resource> m_ResultreadbackBuffer;

	ComPtr<ID3D12Resource> m_DepthBuffer;
	ComPtr<ID3D12Resource> m_FluidRenderResource;
	
	ComPtr<ID3D12Resource> m_WallRenderResource;

	

	ComPtr<ID3D12Resource> m_DepthMapResource0;
	ComPtr<ID3D12Resource> m_DepthMapResource1;
	ComPtr<ID3D12Resource> m_DepthMapResource2;
	ComPtr<ID3D12Resource> m_tickness_resource0;
	ComPtr<ID3D12Resource> m_tickness_resource1;
	ComPtr<ID3D12Resource> m_tickness_resource2;
	
	ComPtr<ID3D12Resource> m_bkWeightsResource;

	// ブラー用のウェイトバッファ
	//ComPtr<ID3D12Resource> m_BlurPramResource;

	std::shared_ptr<Buffer> m_BlurParamBuffer;

	ComPtr<ID3D12DescriptorHeap> m_DepthHeaps;
	ComPtr<ID3D12DescriptorHeap> m_DepthSRVHeaps;

	ComPtr<ID3D12DescriptorHeap> m_RenderRTVHeap;
	ComPtr<ID3D12DescriptorHeap> m_RenderSRVHeap;
	
	ComPtr<ID3D12DescriptorHeap> m_ResultViewHeap;

	ComPtr<ID3D12DescriptorHeap> m_ConstantDeschHeap;

	std::shared_ptr<Buffer> m_ConstDataBuffer;
	std::shared_ptr<Buffer> m_RenderingParamsBuffer;

	enum RootParamterIndex
	{
		RootParamterIndex_DepthMap = 0,
		RootParamterIndex_Matrix = 1,
		RootParamterIndex_ThickNessMap = 2,
		RootParamterIndex_BackTexture = 3,
		RootParamterIndex_CubeMap = 4,
		RootParamterIndex_WallTexture = 5,
		RootParamterIndex_RenderParams = 6,
		RootParamterIndex_ResultBuffer = 7,
	};

	PSO m_FluidRenderingPSO;
	PSO m_PeraPolygonRenderingPSO;
	PSO m_DepthRenderingPSO;

	PSO m_DepthHorizontalBluarPSO;
	PSO m_DepthVerticalBluarPSO;


	PSO m_DepthHorizontalGausBluarPSO;
	PSO m_DepthVerticalGausBluarPSO;

	// 描画用の頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW m_PeraVBV;
	ComPtr<ID3D12Resource> m_peraVB;


	struct FluidProperty
	{
		float m_gaus_radius = 4.0f;
		float m_bilateral_bluer_scale = 0.083f;
		float m_bilateral_bluerdepth_falloff = 1.2f;
		float m_bilateral_filtter_radius = 6.0f;

		SimpleMath::Vector4 m_fluid_color_ = SimpleMath::Vector4(0.2f, 0.6f, 0.9f, 1.0f);
		SimpleMath::Vector4 specular_light_ = SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		SimpleMath::Vector4 ambinet_light_ = SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		SimpleMath::Vector4 diffuse_light_ =SimpleMath::Vector4(1.0f, 0.0f, 0.4f, 0.3f);
	};

	FluidProperty* m_default_fluidprop;
	FluidProperty* m_opend_fluidprop;

	std::vector<FluidProperty*> m_LoadProps;
	std::vector<std::vector<std::string>> m_pLoadData;

	int m_BlurCount = 1;
	float m_ClampDepth = 0.002f;

};

#endif