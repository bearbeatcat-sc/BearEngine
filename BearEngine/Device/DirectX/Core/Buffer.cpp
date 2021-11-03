#include "Buffer.h"
#include "../DirectXDevice.h"

Buffer::Buffer()
{

}

Buffer::~Buffer()
{
	//m_Buffer->Release();
}

ID3D12Resource* Buffer::getBuffer()
{
	return m_Buffer.Get();
}

const D3D12_RESOURCE_DESC& Buffer::GetResourceDesc()
{
	return m_Buffer->GetDesc();
}


ID3D12Resource* Buffer::init(D3D12_HEAP_TYPE type, UINT bufferSize, D3D12_RESOURCE_STATES state)
{
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Alignment = 0;
	resDesc.Width = bufferSize;
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Format = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc = { 1, 0 };
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	
	HRESULT result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(type),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		state,
		nullptr,
		IID_PPV_ARGS(&m_Buffer)
	);

	if (result != S_OK)
	{
		return nullptr;
	}

	return m_Buffer.Get();
}

D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetVirtualAdress()
{
	return m_Buffer->GetGPUVirtualAddress();
}
