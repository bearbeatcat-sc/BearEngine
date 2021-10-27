#ifndef _RENDERING_PIPELINE_H_
#define _RENDERING_PIPELINE_H_

#include "Singleton.h"
#include "DirectX/Core/PSOManager.h"
#include "DirectX/DirectXDevice.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <SimpleMath.h>
#include <array>

#include "Utility/Timer.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;

class DirectXGraphics;
class SkyBox;
class Buffer;

// 後で、更に分岐するかも
class RenderingPipeLine
	:public Singleton<RenderingPipeLine>
{
public:
	friend class Singleton<RenderingPipeLine>;
	RenderingPipeLine();
	~RenderingPipeLine();
	HRESULT Init();
	
	void SetSkyBox(const std::string& texturePath, const SimpleMath::Vector3& scale);
	void DrawBegin();
	void Draw();
	void DrawEnd();
	bool DefaultRenderingBegin();
	bool DefaultRenderingEnd();
	
	void DrawPostEffectPolygon();
	void EffectBloom();
	void EffectDepthOfField();
	void SetDrawFluidFlag(bool flag);
	
	ID3D12Resource* GetOutputRenderResource();
	ID3D12DescriptorHeap* GetPeraSRVHeap();

private:
	void BeginDrawShadow();
	HRESULT CreateRenderResource();
	HRESULT CreateBloomResource();
	HRESULT CreateBlurWeightResource();
	HRESULT CreateDofParameterResource();
	HRESULT CreatePipeLines();
	
	HRESULT CreateBloomPSO();
	HRESULT CreateResultPSO();
	HRESULT CreateDOFPSO();

	HRESULT CreateRTV();
	HRESULT CreateSRV();
	
	HRESULT CreatePeraPolygon();
	HRESULT CreateDOFBuffer();
	void RenderingHighLight();
	void DrawPostEffect();
	HRESULT InitCubeMapResource();
	void RenderingCubeMap();
	void UpdateCubeMapCameraTargets(const SimpleMath::Vector3& cameraPos);
	void UpdateConstantBuffers();

	void UpdareDofParameterBuffer();

private:
	ID3D12GraphicsCommandList* m_pCommandList;

private:

	enum Bloom_RootParamter
	{
		Bloom_RootParamter_NormalColor = 0,
		Bloom_RootParamter_HighLight = 1,
		Bloom_RootParamter_HighLightReduction = 2,
		Bloom_RootParamter_BlurParameter = 3,
	};

	struct DOF_Parameters
	{
		float Pint;
	};
		
	// ポストエフェク用のリソース
	ComPtr<ID3D12Resource> m_OutputRenderResource;
	// レンダーターゲットビューヒープ / シェーダーリソースヒープ
	ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap; 
	ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap;
	// ポストエフェクト処理先の板ポリ
	ComPtr<ID3D12Resource> m_result_bloom_resource;
	// Bloomの縮小バッファ
	std::vector<ComPtr<ID3D12Resource>> m_BloomBuffer;

	// キューブマップ用描画リソース
	// 後でクラスとして独立する
	ComPtr<ID3D12Resource> mCubeMapTex;
	
	ComPtr<ID3D12DescriptorHeap> mCubeMapDescRTVHeaps;
	ComPtr<ID3D12DescriptorHeap> mCubeMapDescSRVHeaps;

	std::vector<SimpleMath::Vector3> mCubeMapCameraTargets;
	std::vector<SimpleMath::Vector3> mCubeMapCameraUps;

	D3D12_VIEWPORT mCubeMapRenderViewPort;
	D3D12_RECT mCubeMapRenderRect;

	SimpleMath::Vector3 mCubeMapCameraCenterPos;
	
	// ブラー用のウェイトバッファ
	ComPtr<ID3D12Resource> m_BlurPramResource;
	D3D12_CPU_DESCRIPTOR_HANDLE BlurParamResource_SRVHandle;

	// 被写界深度用のパラメータバッファ
	std::shared_ptr<Buffer> mDofParameterResource;

	// 被写界深度用のテクスチャ
	ComPtr<ID3D12Resource> m_dof_texture0;
	ComPtr<ID3D12Resource> m_dof_texture1;
	ComPtr<ID3D12Resource> m_result_dof_resource;
	
	D3D12_CPU_DESCRIPTOR_HANDLE DOFTex_CPU_FirstRTVHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE DOFTex_CPU_FirstSRVHandle;

	// クリアカラー
	std::array<float, 4> clerColorArray;
	
	// ポストエフェクト描画用の頂点バッファ
	D3D12_VERTEX_BUFFER_VIEW m_PeraVBV;
	ComPtr<ID3D12Resource> m_peraVB;

	const int OutputRenderResouce = 1;
	const int ResultBloomResource = 1;
	const int ResultDOFResource = 1;
	
	const int m_BloomBufferCount = 8;
	const int m_BlurWeights = 1;
	const int BlurTextureCount = 2;
	const int DofParamterCount = 1;

	PSO posteffect_bloom_hightLight_pso; // 高輝度の抽出
	PSO posteffect_bloomblur_horizontal_pso; //　横ブラー
	PSO posteffect_bloomblur_vertical_pso; // 縦ブラー
	
	PSO posteffect_dof_horizontal_pso; // 縦ブラー
	PSO posteffect_dof_result_pso; // 縦ブラー
	
	PSO posteffect_result_pso; //　最終的なレンダリング結果を表示

	SkyBox* m_pSkyBox;

	bool m_isDrawFluid;

	float mDofPint = 0.16f;

};

#endif