#include "FontSystem.h"


FontSystem::FontSystem()
{
}

FontSystem::~FontSystem()
{
	delete m_Gmemory;
	delete m_SpriteBatch;
	delete m_SpriteFont;
}

HRESULT FontSystem::Init(const wchar_t* fontName)
{
	auto device = DirectXDevice::GetInstance().GetDevice();

	m_Gmemory = new DirectX::GraphicsMemory(device);

	// SpriteBatchオブジェクトの初期化
	DirectX::ResourceUploadBatch resUploadatch(device);
	resUploadatch.Begin();


	DirectX::RenderTargetState rtState(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT
	);

	DirectX::SpriteBatchPipelineStateDescription pd(rtState);


	m_SpriteBatch = new DirectX::SpriteBatch(device,
		resUploadatch, pd);

	m_SpriteBatch->SetViewport(DirectXGraphics::GetInstance().GetViewPort());

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	// SpriteFontオブジェクト初期化
	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));

	m_SpriteFont = new DirectX::SpriteFont(
		device,
		resUploadatch,
		fontName,
		m_Heap->GetCPUDescriptorHandleForHeapStart(),
		m_Heap->GetGPUDescriptorHandleForHeapStart()
	);


	auto future = resUploadatch.End(DirectXGraphics::GetInstance().GetCmdQueue());
	future.wait();

	return S_OK;
}

HRESULT FontSystem::Init()
{
	auto device = DirectXDevice::GetInstance().GetDevice();

	m_Gmemory = new DirectX::GraphicsMemory(device);

	// SpriteBatchオブジェクトの初期化
	DirectX::ResourceUploadBatch resUploadatch(device);
	resUploadatch.Begin();


	DirectX::RenderTargetState rtState(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT
	);

	DirectX::SpriteBatchPipelineStateDescription pd(rtState);

	m_SpriteBatch = new DirectX::SpriteBatch(device,
		resUploadatch, pd);



	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	// SpriteFontオブジェクト初期化
	HRESULT result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_Heap.ReleaseAndGetAddressOf()));

	m_SpriteFont = new DirectX::SpriteFont(
		device,
		resUploadatch,
		L"font/fonttest.spritefont",
		m_Heap->GetCPUDescriptorHandleForHeapStart(),
		m_Heap->GetGPUDescriptorHandleForHeapStart()
	);

	auto future = resUploadatch.End(DirectXGraphics::GetInstance().GetCmdQueue());
	future.wait();

	m_SpriteBatch->SetViewport(DirectXGraphics::GetInstance().GetViewPort());

	return S_OK;
}

void FontSystem::Draw(std::string text, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, DirectX::XMVECTORF32 color)
{
	//// これはBeginのあとに呼ばないといけないみたい？？？なら、コマンドリストみたいな形でやれば解決するかも！
	//m_SpriteFont->DrawString(m_SpriteBatch,
	//	text,
	//	pos,
	//	color);

	TextMessage message = TextMessage{ text,pos,scale,color };

	m_TextMessages.push_back(message);
}

void FontSystem::Draw(const wchar_t* text, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, DirectX::XMVECTORF32 color)
{
	//// これはBeginのあとに呼ばないといけないみたい？？？なら、コマンドリストみたいな形でやれば解決するかも！
	//m_SpriteFont->DrawString(m_SpriteBatch,
	//	text,
	//	pos,
	//	color);

	WideTextMessage message = WideTextMessage{ text,pos,scale,color };

	m_WideTextMessages.push_back(message);
}


void FontSystem::PushCommand()
{
	for (int i = 0; i < m_TextMessages.size(); i++)
	{
		const char* text = m_TextMessages[i].text.data();
		DirectX::XMFLOAT2 pos = m_TextMessages[i].pos;
		DirectX::XMFLOAT2 scale = m_TextMessages[i].scale;
		DirectX::XMVECTORF32 color = m_TextMessages[i].color;
		auto ms = m_SpriteFont->MeasureString(text);
		float x = ms.m128_f32[0] * 0.5f;
		float y = ms.m128_f32[1] * 0.5f;
		DirectX::XMFLOAT2 center = DirectX::XMFLOAT2(x , y);

		m_SpriteFont->DrawString(m_SpriteBatch,
			text,
			pos,
			color,
			0.0f,
			center,
			scale);
	}

	for (int i = 0; i < m_WideTextMessages.size(); i++)
	{
		const wchar_t* text = m_WideTextMessages[i].text;
		DirectX::XMFLOAT2 pos = m_WideTextMessages[i].pos;
		DirectX::XMFLOAT2 scale = m_TextMessages[i].scale;
		DirectX::XMVECTORF32 color = m_WideTextMessages[i].color;
		auto ms = m_SpriteFont->MeasureString(text);
		float x = ms.m128_f32[0] * 0.5f;
		float y = ms.m128_f32[1] * 0.5f;
		DirectX::XMFLOAT2 center = DirectX::XMFLOAT2(x, y);

		m_SpriteFont->DrawString(m_SpriteBatch,
			text,
			pos,
			color,
			0.0f,
			center,
			scale);
	}

	ClearMessages();


}

void FontSystem::Begin()
{
	auto cmd = DirectXGraphics::GetInstance().GetCommandList();
	m_SpriteBatch->Begin(cmd);


}

void FontSystem::End()
{
	m_SpriteBatch->End();

}

void FontSystem::Commit()
{
	m_Gmemory->Commit(DirectXGraphics::GetInstance().GetCmdQueue());
}

ComPtr<ID3D12DescriptorHeap> FontSystem::GetDescHeap()
{
	return m_Heap;
}

void FontSystem::ClearMessages()
{
	m_TextMessages.clear();
	m_WideTextMessages.clear();
}
