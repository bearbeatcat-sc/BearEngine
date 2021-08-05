#include "DirectXDevice.h"
#include <string>
#include "../WindowApp.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

DirectXDevice::DirectXDevice()
{
}

DirectXDevice::~DirectXDevice()
{

}

HRESULT DirectXDevice::InitDirectX()
{
	HRESULT result = S_OK;

	result = CreateDevice();
	result = CreateDxgiFactory();
	FindAdapter();


	return result;
}

ID3D12Device* DirectXDevice::GetDevice()
{
	return m_Device.Get();
}

IDXGIFactory6* DirectXDevice::GetDxgiFactory()
{
	return m_DXGIFactory.Get();
}

ComPtr<ID3D12Resource1> DirectXDevice::CreateResource(const CD3DX12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES resourceStates, const D3D12_CLEAR_VALUE* clearValue, D3D12_HEAP_TYPE heapType)
{
	ComPtr<ID3D12Resource1> ret;

	m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(heapType),
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

	if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&pDred))))
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
		result = D3D12CreateDevice(nullptr, levels[i], IID_PPV_ARGS(&m_Device));

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

	result = CreateDXGIFactory(IID_PPV_ARGS(&m_DXGIFactory));

	return result;
}



void DirectXDevice::FindAdapter()
{
	std::vector<ComPtr<IDXGIAdapter>> adapters;

	for (int i = 0; m_DXGIFactory->EnumAdapters(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(m_Adapter.Get());
	}

	for (int i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC adesc{};
		adapters[i]->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"Microsoft") == std::wstring::npos)
		{
			m_Adapter = adapters[i];
			break;
		}
	}
}
