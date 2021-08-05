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

protected:
	TextureManager();
	~TextureManager();
	void CreateWhiteBuff();

private:
	ComPtr<ID3D12Resource> GenerateTextureResource(std::shared_ptr<Texture> texture);

private:
	std::map<std::string, ComPtr<ID3D12Resource>> m_Textures;
	ComPtr<ID3D12Resource> m_WhiteBuff;
};

#endif