#include "TextureManager.h"
#include "Texture.h"
#include "../Utility/StringUtil.h"
#include "DirectX/DirectXDevice.h"

bool TextureManager::AddTexture(const std::string& filePath, const std::string& textureName)
{
	std::shared_ptr<Texture> temp = std::make_shared<Texture>();
	bool result = temp->Init(StringUtil::GetWideStringFromString(filePath));
	if (!result) return false;

	auto resource = GenerateTextureResource(temp);
	m_Textures.emplace(textureName, resource);
}

ComPtr<ID3D12Resource> TextureManager::GetTexture(const std::string& textureName)
{
	if (m_Textures.find(textureName) != m_Textures.end())
	{
		return m_Textures.at(textureName);
	}

	return nullptr;
}

ComPtr<ID3D12Resource> TextureManager::GetWhiteTex()
{
	return m_WhiteBuff;
}

ComPtr<ID3D12DescriptorHeap> TextureManager::GetHeaps()
{
	return _Heaps;
}

HRESULT TextureManager::CreateShaderResourceView(ComPtr<ID3D12Resource> tex)
{
	auto incSize = DirectXDevice::GetInstance().GetDevice()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto cpu_start = _Heaps->GetCPUDescriptorHandleForHeapStart();
	auto gpu_start = _Heaps->GetGPUDescriptorHandleForHeapStart();

	cpu_start.ptr += incSize * _RegistTextureCount;
	gpu_start.ptr += incSize * _RegistTextureCount;

	D3D12_SHADER_RESOURCE_VIEW_DESC srVDesc = {};
	srVDesc.Format = tex.Get()->GetDesc().Format;
	srVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srVDesc.Texture2D.MipLevels = 1;

	DirectXDevice::GetInstance().GetDevice()->CreateShaderResourceView(
		tex.Get(),
		&srVDesc,
		cpu_start);

	return S_OK;
}

TextureManager::TextureManager()
{
	// 後でテクスチャを一つのヒープで管理するようにする。
	CreateDescriptorHeap();

	CreateWhiteBuff();
}

TextureManager::~TextureManager()
{
	m_Textures.clear();
}

void TextureManager::CreateWhiteBuff()
{
	D3D12_HEAP_PROPERTIES texHeapProp = {};

	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resDEsc = {};
	resDEsc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDEsc.Width = 4;
	resDEsc.Height = 4;
	resDEsc.DepthOrArraySize = 1;
	resDEsc.SampleDesc.Count = 1;
	resDEsc.SampleDesc.Quality = 1;
	resDEsc.MipLevels = 1;
	resDEsc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDEsc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDEsc.Flags = D3D12_RESOURCE_FLAG_NONE;


	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDEsc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&m_WhiteBuff));

	if (FAILED(result))
	{
		return;
	}

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	result = m_WhiteBuff->WriteToSubresource(
		0,
		nullptr,
		data.data(),
		4 * 4,
		data.size());

}

void TextureManager::CreateDescriptorHeap()
{
	_Heaps = DirectXDevice::GetInstance().CreateDescriptorHeap(
		_MaxTextureCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);

}

ComPtr<ID3D12Resource> TextureManager::GenerateTextureResource(std::shared_ptr<Texture> texture)
{
	ComPtr<ID3D12Resource> temp;

	D3D12_HEAP_PROPERTIES texHeapProp{};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;


	D3D12_RESOURCE_DESC textureDesc{};
	auto metadata = texture->GetData();
	textureDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	textureDesc.Format = metadata.format;
	textureDesc.Width = metadata.width;
	textureDesc.Height = (UINT)metadata.height;
	textureDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
	textureDesc.MipLevels = (UINT16)metadata.mipLevels;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto result = DirectXDevice::GetInstance().GetDevice()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&temp));

	result = temp->WriteToSubresource(
		0,
		nullptr,
		texture->GetImage()->pixels,
		texture->GetImage()->rowPitch,
		texture->GetImage()->slicePitch
	);

	return temp;
}

