#include "Sprite2D.h"
#include "DirectX/Core/Buffer.h"

Sprite2D::Sprite2D(XMFLOAT2 anchorpoint, const std::string& effectName, int drawOrder)
	:Sprite(anchorpoint, effectName, drawOrder)
{
}

Sprite2D::Sprite2D(XMFLOAT2 anchorpoint, const std::string& effectName, const std::string& cameraName)
	: Sprite(anchorpoint, effectName)
{

}

Sprite2D::~Sprite2D()
{
}

void Sprite2D::Draw(ID3D12GraphicsCommandList* tempCommand)
{
	DirectX::SimpleMath::Matrix mat;
	mat *= XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);

	ConstBufferData* constMap = nullptr;
	m_ConstantBuffer->getBuffer()->Map(0, nullptr, (void**)&constMap);
	constMap->color = m_Color;
	constMap->mat = XMMatrixIdentity();
	constMap->mat = mat * projMat;
	m_ConstantBuffer->getBuffer()->Unmap(0, nullptr);

	DrawCall(tempCommand);
}
