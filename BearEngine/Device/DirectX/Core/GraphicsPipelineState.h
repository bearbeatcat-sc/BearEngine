#ifndef _GRAPHICS_PIPELINE_STATE_H_
#define _GRAPHICS_PIPELINE_STATE_H_

#include <d3d12.h>
#include <string>
#include <d3dx12.h>

class GraphicsPipelineState
{
public:
	GraphicsPipelineState();
	~GraphicsPipelineState();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetStateDesc();
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC GetRootDesc();
	void Init(const std::string& pixelShaderName, const std::string& vertexShaderName);
	void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc);
	void SetBlendeState(const D3D12_RENDER_TARGET_BLEND_DESC& blendDesc);
	void SetInputLayout(D3D12_INPUT_ELEMENT_DESC* inputLayout,int elementSize);
	void SetPirimitveToplogyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);
	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStensilDesc);
	void SetRootSignatureDesc(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc);

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc;
	D3D12_RASTERIZER_DESC m_RasterizerDesc;
	D3D12_RENDER_TARGET_BLEND_DESC m_BlendDesc;
	D3D12_INPUT_ELEMENT_DESC* m_InputLayout;
	int m_ElementSize;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE m_Type;
	D3D12_DEPTH_STENCIL_DESC m_DepthStensilDesc;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_RootSigDesc;
	std::string m_PixelShaderName;
	std::string m_VertexShaderName;

private:
	bool isSetRasterizeDesc;
	bool isSetBlendeState;
	bool isSetInputLayOut;
	bool isSetPrimitiveToplogyType;
	bool isSetDepthStencilState;
	bool isSetRootSignatureDesc;
};


#endif