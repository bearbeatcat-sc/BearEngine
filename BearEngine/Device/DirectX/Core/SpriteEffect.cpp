#include "SpriteEffect.h"
#include "GraphicsPipelineState.h"

SpriteEffect::SpriteEffect()
{
}

SpriteEffect::~SpriteEffect()
{
}

bool SpriteEffect::Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName)
{
	D3D12_DESCRIPTOR_RANGE descTblRange{};
	descTblRange.NumDescriptors = 1;
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTblRange.BaseShaderRegister = 0;
	descTblRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descRangeSRV{};
	descRangeSRV.NumDescriptors = 1;
	descRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRangeSRV.BaseShaderRegister = 0;
	descRangeSRV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange;
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descRangeSRV;
	rootparam[1].DescriptorTable.NumDescriptorRanges = 1;
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;






	// PSO�̍쐬
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



	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	GraphicsPipelineState gps;
	gps.SetInputLayout(&inputLayout[0], _countof(inputLayout));

	D3D12_RASTERIZER_DESC rasterizeDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gps.SetRasterizerState(rasterizeDesc);

	D3D12_STATIC_SAMPLER_DESC samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	D3D12_DEPTH_STENCIL_DESC depthDEsc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	depthDEsc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;


	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_0(_countof(rootparam), rootparam, 1, &samplerDesc,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	gps.SetRootSignatureDesc(rootSignatureDesc);
	gps.SetBlendeState(renderTargetBlendDesc);
	gps.SetDepthStencilState(depthDEsc);
	gps.Init("BasicSpritePixelShader", "BasicSpriteVertexShader");

	if (!PSOManager::GetInstance().CreatePSO(m_PSO, gps))
	{
		return false;
	}

	return true;
}
