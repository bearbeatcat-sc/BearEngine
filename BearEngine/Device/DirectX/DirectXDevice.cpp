#include "DirectXDevice.h"

#include <dxcapi.h>
#include <string>
#include "../WindowApp.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


HRESULT DirectXDevice::InitDirectX()
{
	HRESULT result = S_OK;

	result = CreateDevice();
	result = CreateDxgiFactory();
	FindAdapter();


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
	ID3D12Debug* debugLayer = nullptr;

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();
		debugLayer->Release();
	}

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
		result = D3D12CreateDevice(nullptr, levels[i], IID_PPV_ARGS(&device_));

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
	std::vector<ComPtr<IDXGIAdapter>> adapters;

	for (int i = 0; dxgi_factory_->EnumAdapters(i, &adapter_) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(adapter_.Get());
	}

	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"Microsoft") == std::wstring::npos)
		{
			adapter_ = adapters[i];
			break;
		}
	}
}

// DXR�ɑΉ����Ă��邩�H
bool DirectXDevice::CheckSupportedDXR()
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
	HRESULT hr = device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));

	if (SUCCEEDED(hr) && features5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
		return true;

	return false;
}

ComPtr<ID3D12RootSignature> DirectXDevice::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& desc)
{
	ComPtr<ID3DBlob> pSigBlob;
	ComPtr<ID3DBlob> pErrorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, 
		pSigBlob.GetAddressOf(), pErrorBlob.GetAddressOf());

	if(FAILED(hr))
	{
		// TODO:後でエラー文の処理
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
