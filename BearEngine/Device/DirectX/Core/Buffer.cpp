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

ID3D12Resource* Buffer::init(D3D12_HEAP_TYPE type, UINT bufferSize, D3D12_RESOURCE_STATES state)
{
	HRESULT result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(type),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
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
