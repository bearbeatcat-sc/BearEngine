#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "../Utility/Color.h"
#include <wrl/client.h>
#include "DirectX/Core/PSOManager.h"
#include "Texture.h"
#include <memory>
#include "DirectX/Core/Model/MeshDrawer.h"
#include "DirectX/Core/Effect.h"
#include <SimpleMath.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Buffer;

class Sprite
{
public:
	Sprite(XMFLOAT2 anchorpoint, const std::string& effectName, int drawOreder = 100);
	virtual ~Sprite();
	bool Init(const std::string& textureName);
	bool Update();
	void SetPosition(XMFLOAT3 pos);
	void SetColor(SimpleMath::Color color);
	void SetSpriteSize(float witdh, float height);
	void SetEffectName(const std::string& effectName);
	void SetFlip(bool x_flag, bool y_flag);
	void SetTextureRange(float tex_x, float tex_y, float tex_Width, float tex_Height);
	void SetTexture(const std::string& textureName);
	void SetAnchorPoint(XMFLOAT2 anchorPoint);
	int GetDrawOrder();
	const std::string& GetEffectName();
	virtual void Draw(ID3D12GraphicsCommandList* tempCommand) = 0;
	std::shared_ptr<Buffer> GetConstanBuffer();
	ComPtr<ID3D12Resource> GetTextureBuffer();	
	bool m_DestroyFlag;
	void Destroy();
	bool GetDrawFlag();
	void SetDrawFlag(bool flag);
	bool GetUpdateTextureFlag();
	void SetUpdateTextureFlag(bool flag);

	MeshDrawer::CPU_GPU_Handles* m_ConstantDescHandle;

protected:
	bool DrawCall(ID3D12GraphicsCommandList* tempCommand);
	void GenerateVertexBuff();
	void UpdateVertexBuff();
	void GenerateIndexBuff();
	void GenerateConBuff();
	void GenerateTextureBuff(const std::string& textureName);
	//void GenerateTextureBuff();

protected:
	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};

	struct ConstBufferData
	{
		XMFLOAT4 color;
		DirectX::SimpleMath::Matrix mat;
	};

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;


protected:
	std::shared_ptr<Buffer> m_VertexBuffer;
	std::shared_ptr<Buffer> m_IndexBuffer;
	std::shared_ptr<Buffer> m_ConstantBuffer;
	ComPtr<ID3D12Resource> m_TextureBuffer;
	MeshDrawer::CPU_GPU_Handles m_Handles;

	DirectX::SimpleMath::Matrix projMat;

	// 各種パラメータ
protected:
	XMFLOAT3 m_pos;
	XMFLOAT2 m_archopoint;
	XMFLOAT2 m_Size;
	SimpleMath::Color m_Color;
	float m_rotate;

	bool m_isFlipX;
	bool m_isFlipY;


	float m_Tex_x;
	float m_Tex_y;
	float m_Tex_Width;
	float m_Tex_Height;

	std::string m_EffectName;
	int m_DrawOrder;
	bool m_isDraw;

	bool m_IsUpdateTexture;
};

