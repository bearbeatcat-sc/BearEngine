#include "Sprite.h"
#include "./DirectX/DirectXDevice.h"
#include "./DirectX/DirectXGraphics.h"
#include "./DirectX/Core/PSOManager.h"
#include "./DirectX/DirectXInput.h"
#include "./Texture.h"
#include "WindowApp.h"
#include "DirectX/Core/Buffer.h"
#include "SpriteDrawer.h"
#include "TextureManager.h"


Sprite::Sprite(XMFLOAT2 anchorpoint, const std::string& effectName, int drawOreder)
	:m_archopoint(anchorpoint), m_rotate(0.0f),m_Tex_x(0), m_Tex_y(0)
	,m_DestroyFlag(false),m_Color(1,1,1,1), m_isFlipX(false), m_isFlipY(false),
	m_DrawOrder(drawOreder),m_isDraw(true), m_IsUpdateTexture(false)
{
}

Sprite::~Sprite()
{
	m_VertexBuffer = 0;
	m_ConstantBuffer = 0;
	m_IndexBuffer = 0;

}

bool Sprite::Init(const std::string& textureName)
{
	GenerateTextureBuff(textureName);
	GenerateVertexBuff();
	GenerateIndexBuff();
	GenerateConBuff();

	projMat = XMMatrixOrthographicOffCenterLH(
		0.0f, WindowApp::GetInstance().GetWindowSize().window_Width,
		WindowApp::GetInstance().GetWindowSize().window_Height, 0,
		0, 1000.0f
	);



	return true;
}

bool Sprite::Update()
{
	return true;
}

void Sprite::SetPosition(XMFLOAT3 pos)
{
	m_pos = pos;
}

void Sprite::SetColor(SimpleMath::Color color)
{
	m_Color = color;
}

void Sprite::SetSpriteSize(float width, float height)
{
	m_Size.x = width;
	m_Size.y = height;

	UpdateVertexBuff();
}

void Sprite::SetEffectName(const std::string& effectName)
{
	m_EffectName = effectName;
}

void Sprite::SetFlip(bool xFlag,bool yFlag)
{
	m_isFlipX = xFlag;
	m_isFlipY = yFlag;

	UpdateVertexBuff();
}

void Sprite::SetTextureRange(float tex_x, float tex_y, float tex_Width, float tex_Height)
{
	m_Tex_x = tex_x;
	m_Tex_y = tex_y;
	m_Tex_Width = tex_Width;
	m_Tex_Height = tex_Height;	

	m_Size.x = tex_Width;
	m_Size.y = tex_Height;

	UpdateVertexBuff();
}

void Sprite::SetTexture(const std::string& textureName)
{
	GenerateTextureBuff(textureName);
	SetUpdateTextureFlag(true);
}

void Sprite::SetAnchorPoint(XMFLOAT2 anchorPoint)
{
	m_archopoint = anchorPoint;
}

int Sprite::GetDrawOrder()
{
	return m_DrawOrder;
}

const std::string& Sprite::GetEffectName()
{
	return m_EffectName;
}


std::shared_ptr<Buffer> Sprite::GetConstanBuffer()
{
	return m_ConstantBuffer;
}

ComPtr<ID3D12Resource> Sprite::GetTextureBuffer()
{
	return m_TextureBuffer;
}

void Sprite::Destroy()
{
	m_DestroyFlag = true;
}

bool Sprite::GetDrawFlag()
{
	return m_isDraw;
}

void Sprite::SetDrawFlag(bool flag)
{
	m_isDraw = flag;
}

void Sprite::SetUpdateTextureFlag(bool flag)
{
	m_IsUpdateTexture = flag;
}

bool Sprite::GetUpdateTextureFlag ()
{
	return m_IsUpdateTexture;
}

bool Sprite::DrawCall(ID3D12GraphicsCommandList* tempCommand)
{

	tempCommand->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	tempCommand->IASetVertexBuffers(0, 1, &m_vbView);
	tempCommand->IASetIndexBuffer(&m_ibView);
	tempCommand->DrawIndexedInstanced(6, 1, 0, 0, 0);

	return true;
}

void Sprite::GenerateVertexBuff()
{
	//m_Size.x = 1.0f;
	//m_Size.y = 1.0f;

	float left = (0.0f - m_archopoint.x) * m_Size.x;
	float right = (1.0f - m_archopoint.x) * m_Size.x;

	float top = (0.0f - m_archopoint.y) * m_Size.y;
	float bottom = (1.0f - m_archopoint.y) * m_Size.y;

	Vertex vertices[] =
	{
		{{left,bottom ,0.0f},{0.0f,1.0f}},// ç∂â∫
		{{left,top,0.0f},{0.0f,0.0f}},// ç∂â∫
		{{right,bottom,0.0f},{1.0f,1.0f}},// ç∂â∫
		{{right,top,0.0f},{1.0f,0.0f}},// ç∂â∫
	};

	m_VertexBuffer = std::make_shared<Buffer>();
	UINT buffSize = sizeof(vertices);
	ID3D12Resource* vertBuff = m_VertexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	auto textureData = m_TextureBuffer->GetDesc();
	float tex_left = m_Tex_x / textureData.Width;
	float tex_right = (m_Tex_x + m_Tex_Width) / textureData.Width;
	float tex_top = m_Tex_y / textureData.Height;
	float tex_bottom = (m_Tex_y + m_Tex_Height) / textureData.Height;

	vertices[0].uv.x = tex_left;
	vertices[0].uv.y = tex_bottom;

	vertices[1].uv.x = tex_left;
	vertices[1].uv.y = tex_top;

	vertices[2].uv.x = tex_right;
	vertices[2].uv.y = tex_bottom;

	vertices[3].uv.x = tex_right;
	vertices[3].uv.y = tex_top;


	Vertex* vertMap = nullptr;

	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return;
	}

	for (int i = 0; i < 4; i++)
	{
		vertMap[i] = vertices[i];
	}

	vertBuff->Unmap(0, nullptr);

	m_vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();
	m_vbView.SizeInBytes = sizeof(vertices);
	m_vbView.StrideInBytes = sizeof(vertices[0]);
}

void Sprite::UpdateVertexBuff()
{
	float left = (0.0f - m_archopoint.x) * m_Size.x;
	float right = (1.0f - m_archopoint.x) * m_Size.x;

	float top = (0.0f - m_archopoint.y) * m_Size.y;
	float bottom = (1.0f - m_archopoint.y) * m_Size.y;

	if (m_isFlipX)
	{
		left = -left;
		right = -right;
	}

	if (m_isFlipY)
	{
		top = -top;
		bottom = -bottom;
	}

	Vertex vertices[] =
	{
		{{left,bottom ,0.0f},{0.0f,1.0f}},// ç∂â∫
		{{left,top,0.0f},{0.0f,0.0f}},// ç∂â∫
		{{right,bottom,0.0f},{1.0f,1.0f}},// ç∂â∫
		{{right,top,0.0f},{1.0f,0.0f}},// ç∂â∫
	};

	auto textureData = m_TextureBuffer->GetDesc();
	float tex_left = m_Tex_x / textureData.Width;
	float tex_right = (m_Tex_x + m_Tex_Width) / textureData.Width;
	float tex_top = m_Tex_y / textureData.Height;
	float tex_bottom = (m_Tex_y + m_Tex_Height) / textureData.Height;

	vertices[0].uv.x = tex_left;
	vertices[0].uv.y = tex_bottom;

	vertices[1].uv.x = tex_left;
	vertices[1].uv.y = tex_top;

	vertices[2].uv.x = tex_right;
	vertices[2].uv.y = tex_bottom;

	vertices[3].uv.x = tex_right;
	vertices[3].uv.y = tex_top;


	Vertex* vertMap = nullptr;
	auto vertBuff = m_VertexBuffer->getBuffer();

	if (FAILED(vertBuff->Map(0, nullptr, (void**)&vertMap)))
	{
		return;
	}

	for (int i = 0; i < 4; i++)
	{
		vertMap[i] = vertices[i];
	}

	vertBuff->Unmap(0, nullptr);
}



void Sprite::GenerateIndexBuff()
{


	unsigned short indices[] =
	{
		0,1,2,
		2,1,3
	};

	
	m_IndexBuffer = std::make_shared<Buffer>();
	UINT buffSize = sizeof(indices);
	ID3D12Resource* indexBuff = m_IndexBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);


	unsigned short* indexMap = nullptr;
	indexBuff->Map(0, nullptr, (void**)&indexMap);

	for (int i = 0; i < _countof(indices); i++)
	{
		indexMap[i] = indices[i];
	}

	indexBuff->Unmap(0, nullptr);


	m_ibView.BufferLocation = indexBuff->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = sizeof(indices);

}

void Sprite::GenerateConBuff()
{
	m_ConstantBuffer = std::make_shared<Buffer>();
	UINT buffSize = (sizeof(ConstBufferData) + 0xff) & ~0xff;
	ID3D12Resource* constBuff = m_ConstantBuffer->init(D3D12_HEAP_TYPE_UPLOAD, buffSize, D3D12_RESOURCE_STATE_GENERIC_READ);

	ConstBufferData* constMap = nullptr;
	constBuff->Map(0, nullptr, (void**)&constMap);
	constMap->mat = XMMatrixIdentity();
	constMap->color = m_Color;
	constBuff->Unmap(0, nullptr);
}

void Sprite::GenerateTextureBuff(const std::string& textureName)
{
	m_TextureBuffer = TextureManager::GetInstance().GetTexture(textureName);

	m_Tex_Width = m_TextureBuffer->GetDesc().Width;
	m_Tex_Height = m_TextureBuffer->GetDesc().Height;

	m_Size.x = m_Tex_Width;
	m_Size.y = m_Tex_Height;
}

