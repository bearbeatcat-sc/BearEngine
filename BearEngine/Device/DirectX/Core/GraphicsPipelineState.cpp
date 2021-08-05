#include "GraphicsPipelineState.h"
#include "ShaderManager.h"


GraphicsPipelineState::GraphicsPipelineState()
	:isSetBlendeState(false),isSetDepthStencilState(false),
	isSetInputLayOut(false),isSetPrimitiveToplogyType(false),
	isSetRasterizeDesc(false),isSetRootSignatureDesc(false)
{
}

GraphicsPipelineState::~GraphicsPipelineState()
{

}

D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipelineState::GetStateDesc()
{
	return m_Desc;
}

CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC GraphicsPipelineState::GetRootDesc()
{
	return m_RootSigDesc;
}

void GraphicsPipelineState::Init(const std::string& pixelShaderName, const std::string& vertexShaderName)
{
	m_PixelShaderName = pixelShaderName;
	m_VertexShaderName = vertexShaderName;

	m_Desc.PS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(pixelShaderName));
	m_Desc.VS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(vertexShaderName));



	m_Desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;


	if (!isSetBlendeState)
	{
		m_Desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}

	if (!isSetRasterizeDesc)
	{
		m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

		m_Desc.RasterizerState = m_RasterizerDesc;
	}

	m_Desc.DSVFormat =  DXGI_FORMAT_D32_FLOAT;
	m_Desc.NumRenderTargets = 1;

	// SRGB
	//m_Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_Desc.SampleDesc.Count = 1;

	// 独自の設定が必要なので、未設定では返す
	if (!isSetInputLayOut)
	{
		return;
	}

	if (!isSetPrimitiveToplogyType)
	{
		m_Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	}

	if (!isSetDepthStencilState)
	{		
		m_DepthStensilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		m_Desc.DepthStencilState = m_DepthStensilDesc;
	}

	// 独自の設定が必要なので、未設定では返す
	if (!isSetRootSignatureDesc)
	{
		return;
	}
}

void GraphicsPipelineState::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc)
{
	m_RasterizerDesc.AntialiasedLineEnable = rasterizerDesc.AntialiasedLineEnable;
	m_RasterizerDesc.ConservativeRaster = rasterizerDesc.ConservativeRaster;
	m_RasterizerDesc.CullMode = rasterizerDesc.CullMode;
	m_RasterizerDesc.DepthBias = rasterizerDesc.DepthBias;
	m_RasterizerDesc.DepthClipEnable = rasterizerDesc.DepthClipEnable;
	m_RasterizerDesc.FillMode = rasterizerDesc.FillMode;
	m_RasterizerDesc.FrontCounterClockwise = rasterizerDesc.FrontCounterClockwise;
	m_RasterizerDesc.MultisampleEnable = rasterizerDesc.MultisampleEnable;
	m_RasterizerDesc.SlopeScaledDepthBias = rasterizerDesc.SlopeScaledDepthBias;
	
	isSetRasterizeDesc = true;

	m_Desc.RasterizerState = m_RasterizerDesc;
}

void GraphicsPipelineState::SetBlendeState(const D3D12_RENDER_TARGET_BLEND_DESC& blendDesc)
{
	m_BlendDesc.BlendEnable = blendDesc.BlendEnable;
	m_BlendDesc.BlendOp = blendDesc.BlendOp;
	m_BlendDesc.BlendOpAlpha = blendDesc.BlendOpAlpha;
	m_BlendDesc.DestBlend = blendDesc.DestBlend;
	m_BlendDesc.DestBlendAlpha = blendDesc.DestBlendAlpha;
	m_BlendDesc.LogicOp = blendDesc.LogicOp;
	m_BlendDesc.LogicOpEnable = blendDesc.LogicOpEnable;
	m_BlendDesc.RenderTargetWriteMask = blendDesc.RenderTargetWriteMask;
	m_BlendDesc.SrcBlend = blendDesc.SrcBlend;
	m_BlendDesc.SrcBlendAlpha = blendDesc.SrcBlendAlpha;

	isSetBlendeState = true;

	m_Desc.BlendState.RenderTarget[0] = m_BlendDesc;
	m_Desc.BlendState.AlphaToCoverageEnable = false;
	m_Desc.BlendState.IndependentBlendEnable = false;
}

void GraphicsPipelineState::SetInputLayout(D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize)
{
	m_InputLayout = inputLayout;
	m_ElementSize = elementSize;
	isSetInputLayOut = true;


	m_Desc.InputLayout.pInputElementDescs = inputLayout;
	m_Desc.InputLayout.NumElements = m_ElementSize;
}

void GraphicsPipelineState::SetPirimitveToplogyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
	m_Type = type;
	isSetPrimitiveToplogyType = true;

	m_Desc.PrimitiveTopologyType = m_Type;
}

void GraphicsPipelineState::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStensilDesc)
{
	m_DepthStensilDesc = depthStensilDesc;
	isSetDepthStencilState = true;

	m_Desc.DepthStencilState = m_DepthStensilDesc;
}

void GraphicsPipelineState::SetRootSignatureDesc(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc)
{
	m_RootSigDesc = rootSigDesc;
	isSetRootSignatureDesc = true;
}

