#include "MeshEffect.h"

MeshEffect::MeshEffect()
{
}

MeshEffect::~MeshEffect()
{
}

bool MeshEffect::Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName)
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
	CD3DX12_ROOT_PARAMETER rootparam[5] = {};
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

	// CubeMap用の定数バッファ
	rootparam[4].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);





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
	rasterizeDesc.CullMode = D3D12_CULL_MODE_NONE;

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

	if (!PSOManager::GetInstance().CreatePSO(m_PSO, vertexShaderName, pixelShaderName,
		&inputLayout[0], _countof(inputLayout), rasterizeDesc, blendDesc, rootSignatureDesc))
	{
		return false;
	}

	return true;
}
