#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <d3dx12.h>

using Microsoft::WRL::ComPtr;

class Buffer
{
public:
	Buffer();
	~Buffer();
	ID3D12Resource* getBuffer();
	ID3D12Resource* init(D3D12_HEAP_TYPE type, UINT bufferSize, D3D12_RESOURCE_STATES state);


private:
	//ComPtr<ID3D12Resource> m_Buffer;
	ComPtr<ID3D12Resource> m_Buffer;
};

