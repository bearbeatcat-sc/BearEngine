#pragma once
#include <d3d12.h>
#include "../../Singleton.h"
#include <vector>
#include <wrl/client.h>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

class GraphicsPipelineState;

struct PSO
{
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
};

class PSOManager
	:public Singleton<PSOManager>
{
public:
	friend class Singleton<PSOManager>;
	bool CreatePSO(PSO& pso, const std::string& vertexShaderName, const std::string& pixelShaderName, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc);
	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc);
	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc);

	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, D3D12_PRIMITIVE_TOPOLOGY_TYPE toplogyType);
	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc, D3D12_PRIMITIVE_TOPOLOGY_TYPE toplogyType);

	bool CreatePSO(PSO& pso, const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geometryShaderName,D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc, D3D12_PRIMITIVE_TOPOLOGY_TYPE toplogyType);

	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc);
	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc);
	bool CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc,const D3D12_DEPTH_STENCIL_DESC& depthStensilDesc);
	bool CreatePSO(PSO& pso, GraphicsPipelineState& gps);
	bool LoadShader(const wchar_t* fileName, const char* target, ID3DBlob** blob,const char* entryPoint = "main");

protected:
	PSOManager();
	~PSOManager();


};

