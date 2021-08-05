#ifndef _FONT_SYSTEM_H_
#define _FONT_SYSTEM_H_

#include "DirectXDevice.h"
#include "DirectXGraphics.h"
#include <SpriteFont.h>
#include <ResourceUploadBatch.h>
#include <wrl/client.h>
#include <vector>
#include "SimpleMath.h"

#pragma comment(lib,"DirectXTK12.lib")
#pragma comment(lib,"dxguid.lib")

using Microsoft::WRL::ComPtr;

class FontSystem
{
public:
	struct TextMessage
	{
		std::string text;
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 scale;
		DirectX::XMVECTORF32 color;
	};

	struct WideTextMessage
	{
		const wchar_t* text;
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 scale;
		DirectX::XMVECTORF32 color;
	};

	FontSystem();
	~FontSystem();
	HRESULT Init(const wchar_t* fontName);
	HRESULT Init();
	void Draw(std::string text, DirectX::XMFLOAT2 scale,DirectX::XMFLOAT2 pos, DirectX::XMVECTORF32 color);
	void Draw(const wchar_t* text, DirectX::XMFLOAT2 scale,DirectX::XMFLOAT2 pos, DirectX::XMVECTORF32 color);
	void PushCommand();
	void Begin();
	void End();
	void Commit();
	ComPtr<ID3D12DescriptorHeap> GetDescHeap();
private:
	void ClearMessages();

private:
	DirectX::GraphicsMemory* m_Gmemory;
	DirectX::SpriteFont* m_SpriteFont;
	DirectX::SpriteBatch* m_SpriteBatch;
	ComPtr<ID3D12DescriptorHeap> m_Heap;
	std::vector<TextMessage> m_TextMessages;
	std::vector<WideTextMessage> m_WideTextMessages;

	wchar_t* m_FontName;
};

#endif