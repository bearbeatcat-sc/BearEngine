#include "Texture.h"

using namespace DirectX;

Texture::Texture()
{
}

Texture::~Texture()
{
	m_Image = 0;
}

bool Texture::Init(const wchar_t* path)
{
	return 	LoadTexture(path);	
}

bool Texture::Init(const std::wstring path)
{
	return 	LoadTexture(path);
}

bool Texture::LoadTexture(const std::wstring path)
{

	if (LoadFromWICFile(path.data(),
		WIC_FLAGS_NONE,
		&m_metadata, m_ScrtchImage))
	{
		return false;
	}

	m_Image = m_ScrtchImage.GetImage(0, 0, 0);

	return true;

}

bool Texture::LoadTexture(const wchar_t* path)
{

	if (LoadFromWICFile(path,
		WIC_FLAGS_NONE,
		&m_metadata, m_ScrtchImage))
	{
		return false;
	}

	m_Image = m_ScrtchImage.GetImage(0, 0, 0);
	return true;
}


DirectX::TexMetadata Texture::GetData()
{
	return m_metadata;
}

const DirectX::Image* Texture::GetImage()
{
	return m_Image;
}
