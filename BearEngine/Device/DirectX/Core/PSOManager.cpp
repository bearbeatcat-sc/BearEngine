#include "PSOManager.h"
#include "../DirectXDevice.h"
#include <d3dcompiler.h>
#include "../../WindowApp.h"
#include "ShaderManager.h"
#include "GraphicsPipelineState.h"

bool PSOManager::LoadShader(const wchar_t* fileName, const char* target, ID3DBlob** blob, const char* entryPoint)
{
	ComPtr<ID3DBlob> errorBuffer = nullptr;

	if (FAILED(D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, blob, &errorBuffer)))
	{
		if (errorBuffer)
		{
			OutputDebugStringA(static_cast<char*>(errorBuffer->GetBufferPointer()));
		}

		return false;
	}

	return true;


}

bool PSOManager::CreatePSO(PSO& pso, const std::string& vertexShaderName, const std::string& pixelShaderName, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc)
{

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;


	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(vertexShaderName));
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(pixelShaderName));

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc,const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;


	


	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSigDesc,D3D_ROOT_SIGNATURE_VERSION_1_0,&rootSigBlob,&errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;





	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, D3D12_PRIMITIVE_TOPOLOGY_TYPE toplogyType)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}




	HRESULT result = S_OK;
	ID3DBlob* errorBlob;




	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);


	if (gs != L"")
	{
		ID3DBlob* geoShaderBlob;
		if (!LoadShader(gs, "gs_5_0", &geoShaderBlob))
		{
			MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
			return false;
		}
		gpipeline.GS = CD3DX12_SHADER_BYTECODE(geoShaderBlob);
	}



	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;


	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);

	if (gs != L"")
	{
		ID3DBlob* geoShaderBlob;
		if (!LoadShader(gs, "gs_5_0", &geoShaderBlob))
		{
			MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
			return false;
		}
		gpipeline.GS = CD3DX12_SHADER_BYTECODE(geoShaderBlob);
	}

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc, D3D12_PRIMITIVE_TOPOLOGY_TYPE toplogyType)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;





	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);

	if (gs != L"")
	{
		ID3DBlob* geoShaderBlob;
		if (!LoadShader(gs, "gs_5_0", &geoShaderBlob))
		{
			MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
			return false;
		}
		gpipeline.GS = CD3DX12_SHADER_BYTECODE(geoShaderBlob);
	}


	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = toplogyType;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geometryShaderName, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSigDesc, D3D12_PRIMITIVE_TOPOLOGY_TYPE toplogyType)
{



	HRESULT result = S_OK;
	ID3DBlob* errorBlob;





	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(vertexShaderName));
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(pixelShaderName));

	if (geometryShaderName != "")
	{
		gpipeline.GS = CD3DX12_SHADER_BYTECODE(ShaderManager::GetInstance().GetShader(geometryShaderName));
	}

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = toplogyType;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, const wchar_t* vs, const wchar_t* ps, const wchar_t* gs, D3D12_INPUT_ELEMENT_DESC* inputLayout, int elementSize, const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
		return false;
	}

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;





	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);

	if (gs != L"")
	{
		ID3DBlob* geoShaderBlob;
		if (!LoadShader(gs, "gs_5_0", &geoShaderBlob))
		{
			MessageBox(WindowApp::GetInstance().GetHWND(), L"�V�F�[�_�[���ǂݍ��߂܂���ł����B", L"Error", 0);
			return false;
		}
		gpipeline.GS = CD3DX12_SHADER_BYTECODE(geoShaderBlob);
	}

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState.DepthEnable = true;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO & pso, const wchar_t * vs, const wchar_t * ps, D3D12_INPUT_ELEMENT_DESC * inputLayout, int elementSize, const D3D12_RASTERIZER_DESC & rasterizerDesc, const D3D12_BLEND_DESC & blendDesc, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC & rootSigDesc, const D3D12_DEPTH_STENCIL_DESC & depthStensilDesc)
{
	ID3DBlob* vertexShaderBlob;

	if (!LoadShader(vs, "vs_5_0", &vertexShaderBlob))
	{
		return false;
	}

	ID3DBlob* pixelShaderBlob;

	if (!LoadShader(ps, "ps_5_0", &pixelShaderBlob))
	{
		return false;
	}

	HRESULT result = S_OK;
	ID3DBlob* errorBlob;





	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline{};
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob);
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob);

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gpipeline.RasterizerState = rasterizerDesc;
	gpipeline.BlendState = blendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;
	gpipeline.InputLayout.NumElements = elementSize;

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpipeline.SampleDesc.Count = 1;

	gpipeline.DepthStencilState = depthStensilDesc;


	ID3DBlob* rootSigBlob;
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

bool PSOManager::CreatePSO(PSO& pso, GraphicsPipelineState& gps)
{
	//PSO�̍쐬
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = gps.GetStateDesc();


	ID3DBlob* rootSigBlob;
	ID3DBlob* errorBlob;
	auto rootSig = gps.GetRootDesc();
	if (FAILED(D3DX12SerializeVersionedRootSignature(&rootSig, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob)))
	{
		return false;
	}

	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&pso.rootSignature))))
	{
		return false;
	}

	rootSigBlob->Release();


	gpipeline.pRootSignature = pso.rootSignature.Get();
	if (FAILED(DirectXDevice::GetInstance().GetDevice()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&pso.pso))))
	{
		return false;
	}

	return true;
}

PSOManager::PSOManager()
{

}

PSOManager::~PSOManager()
{
}

