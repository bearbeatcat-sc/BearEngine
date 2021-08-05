#pragma once
#include <DirectXTex.h>
#include <memory>

#pragma comment (lib,"DirectXTex.lib")

class Texture
{
public:
	Texture();
	~Texture();
	bool Init(const wchar_t* path);
	bool Init(const std::wstring path);

	DirectX::TexMetadata GetData();
	const DirectX::Image* GetImage();
private:
	bool LoadTexture(const wchar_t* path);
	bool LoadTexture(const std::wstring path);


private:
	DirectX::TexMetadata m_metadata;
	const DirectX::Image* m_Image;
	DirectX::ScratchImage m_ScrtchImage{};
};

