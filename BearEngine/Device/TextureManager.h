#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include "Singleton.h"
#include <memory>
#include <string>
#include <map>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Texture;

class TextureManager
	:public Singleton<TextureManager>
{
public:
	friend class Singleton<TextureManager>;
	bool AddTexture(const std::string& filePath, const std::string& textureName);
	ComPtr<ID3D12Resource> GetTexture(const std::string& textureName);
	ComPtr<ID3D12Resource> GetWhiteTex();
	ComPtr<ID3D12DescriptorHeap> GetHeaps();
	HRESULT CreateShaderResourceView(ComPtr<ID3D12Resource> tex);

protected:
	TextureManager();
	~TextureManager();
	void CreateWhiteBuff();
	void CreateDescriptorHeap();

private:
	ComPtr<ID3D12Resource> GenerateTextureResource(std::shared_ptr<Texture> texture);

private:
	std::map<std::string, ComPtr<ID3D12Resource>> m_Textures;
	ComPtr<ID3D12Resource> m_WhiteBuff;
	ComPtr<ID3D12DescriptorHeap> _Heaps;

	const UINT _MaxTextureCount = 200;
	UINT _RegistTextureCount = 0;
};

#endif