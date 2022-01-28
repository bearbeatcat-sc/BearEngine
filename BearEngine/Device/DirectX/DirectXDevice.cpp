﻿#include "DirectXDevice.h"

#include <dxcapi.h>
#include <string>
#include "../WindowApp.h"
#include "imgui/imgui.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


HRESULT DirectXDevice::InitDirectX()
{
	HRESULT result = S_OK;

	if (CreateDxgiFactory() != S_OK)
	{
		MessageBoxA(nullptr, "Dxgiファクトリーの生成に失敗", "ERROR", MB_OK);
		throw std::logic_error("Dxgiファクトリーの生成に失敗");
	}

	FindAdapter();

	if(CreateDevice() != S_OK)
	{
		MessageBoxA(nullptr, "DirectX12デバイスの生成に失敗", "ERROR", MB_OK);
		throw std::logic_error("DirectX12デバイスの生成に失敗");
	}

	if (!CheckSupportedDXR())
	{
		WindowApp::GetInstance().MsgBox("Not Supported DXR.", "ERROR");
		return false;
	}


	return result;
}

ID3D12Device5* DirectXDevice::GetDevice()
{
	return device_.Get();
}

IDXGIFactory6* DirectXDevice::GetDxgiFactory()
{
	return dxgi_factory_.Get();
}

void DirectXDevice::GetVideoMemoryInfo(DXGI_QUERY_VIDEO_MEMORY_INFO* info)
{
	_pAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, info);
}

ComPtr<ID3D12Resource1> DirectXDevice::CreateResource(const CD3DX12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES resourceStates, const D3D12_CLEAR_VALUE* clearValue, D3D12_HEAP_TYPE heapType)
{
	ComPtr<ID3D12Resource1> ret;

	device_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(heapType),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		resourceStates,
		clearValue,
		IID_PPV_ARGS(&ret));

	return ret;
}

void DirectXDevice::EnableDebugLayer()
{
	ComPtr<ID3D12Debug1> pDebugController1 = nullptr;
	ComPtr<ID3D12Debug1> pDebugController2 = nullptr;

	if (!SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController1))))
	{
		WindowApp::GetInstance().MsgBox("DebugInterfaceの取得に失敗", "ERROR");
		throw std::runtime_error("DebugInterfaceの取得に失敗");
	}

	if (!SUCCEEDED(pDebugController1->QueryInterface(IID_PPV_ARGS(&pDebugController2))))
	{
		WindowApp::GetInstance().MsgBox("DebugInterfaceの取得に失敗", "ERROR");
		throw std::runtime_error("DebugInterfaceの取得に失敗");
	}

	pDebugController2->SetEnableGPUBasedValidation(true);


	ID3D12DeviceRemovedExtendedDataSettings1* d3dDredSettings1;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDredSettings1))))
	{
		d3dDredSettings1->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		d3dDredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		d3dDredSettings1->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
	}

	
}

void DirectXDevice::DeviceRemovedHandler()
{
	ComPtr<ID3D12DeviceRemovedExtendedData> pDred;

	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&pDred))))
	{
		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBredcrumbOutput;
		D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput;

		pDred->GetAutoBreadcrumbsOutput(&DredAutoBredcrumbOutput);
		pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput);
	}
}


HRESULT DirectXDevice::CreateDevice()
{
	HRESULT result = S_OK;

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	for (int i = 0; i < _countof(levels); i++)
	{
		result = D3D12CreateDevice(_pAdapter.Get(), levels[i], IID_PPV_ARGS(&device_));

		if (result == S_OK)
		{
			featureLevel = levels[i];
			break;
		}
	}
	
	return result;
}

HRESULT DirectXDevice::CreateDxgiFactory()
{
	HRESULT result = S_OK;

	result = CreateDXGIFactory(IID_PPV_ARGS(&dxgi_factory_));

	return result;
}



void DirectXDevice::FindAdapter()
{
	std::vector<ComPtr<IDXGIAdapter1>> adapters;

	UINT i = 0;
	IDXGIAdapter1* pAdapter = nullptr;

	// アダプターの列挙
	while (dxgi_factory_->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		adapters.push_back(pAdapter);
		++i;
	}

	IDXGIAdapter1* pSetAdpter = nullptr;

	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if ((strDesc.find(L"Microsoft") == std::wstring::npos && strDesc.find(L"Intel") == std::wstring::npos))
		{
			pSetAdpter = adapters[i].Get();
			break;
		}
	}

	if(pSetAdpter == nullptr)
	{
		MessageBoxA(nullptr, "対応しているアダプターの取得に失敗", "ERROR", MB_OK);
		throw std::logic_error("対応しているアダプターの取得に失敗");
	}

	_pAdapter = reinterpret_cast<ComPtr<IDXGIAdapter4>&>(pSetAdpter);
}

// DXR�ɑΉ����Ă��邩�H
bool DirectXDevice::CheckSupportedDXR()
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5{};
	HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, UINT(sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5)));

	if (FAILED(hr) || features5.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		return false;

	return true;
}

ComPtr<ID3D12RootSignature> DirectXDevice::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc)
{
	ComPtr<ID3DBlob> pSigBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, 
		pSigBlob.GetAddressOf(), pErrorBlob.GetAddressOf());

	if(FAILED(hr))
	{
		// TODO:後でエラー文の処理
		std::vector<char> infoLog(pErrorBlob->GetBufferSize() + 1);
		memcpy(infoLog.data(), pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize());
		infoLog[pErrorBlob->GetBufferSize()] = 0;

		std::string errorMsg = "ルートシグネチャの生成に失敗\n";
		errorMsg.append(infoLog.data());

		MessageBoxA(nullptr, errorMsg.c_str(), "ERROR", MB_OK);
		throw std::logic_error("ルートシグネチャの生成に失敗");
		return nullptr;
	}
	

	ComPtr<ID3D12RootSignature> pRootSig;
	device_->CreateRootSignature(
		0,
		pSigBlob.Get()->GetBufferPointer(),
		pSigBlob.Get()->GetBufferSize(),
		IID_PPV_ARGS(pRootSig.ReleaseAndGetAddressOf()));

	return pRootSig;
}

ComPtr<ID3D12DescriptorHeap> DirectXDevice::CreateDescriptorHeap(uint32_t count, D3D12_DESCRIPTOR_HEAP_TYPE type,
	bool shaderVisible)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = count;
	desc.Type = type;
	desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ComPtr<ID3D12DescriptorHeap> pHeap;
	device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pHeap));

	return pHeap;
}

void DirectXDevice::RenderDebug()
{
	DXGI_QUERY_VIDEO_MEMORY_INFO info;
	DirectXDevice::GetInstance().GetVideoMemoryInfo(&info);

	// メガバイトに変換
	size_t usedVRAM = info.CurrentUsage / 1024 / 1024;
	size_t VRAM = info.Budget / 1024 / 1024;
	float parcent = (float)usedVRAM / (float)VRAM;

	if (ImGui::BeginTabItem("Device Properties"))
	{
		ImGui::Text("VidemoRAM:%iMB", VRAM);
		ImGui::Text("UsedVideoRAM:%iMB",usedVRAM);
		ImGui::Text("Used:%f", parcent);
		ImGui::EndTabItem();
	}
}
