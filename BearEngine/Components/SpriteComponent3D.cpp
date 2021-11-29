#include "SpriteComponent3D.h"
#include "../Device/SpriteDrawer.h"

SpriteComponent3D::SpriteComponent3D(Actor* user, const std::string& textureName, const std::string& effectName, int updateOrder)
	:Component(user,updateOrder)
{
	Init(textureName, effectName);
}

SpriteComponent3D::~SpriteComponent3D()
{
	m_Sprite->Destroy();
}

void SpriteComponent3D::Update()
{
}

void SpriteComponent3D::Init(const std::string& textureName, const std::string& effectName)
{
	m_Sprite = std::make_shared<Sprite3D>(DirectX::XMFLOAT2(0.5f, 0.5f), effectName);
	m_Sprite->Init(textureName);
	m_Sprite->SetEffectName(effectName);
	SpriteDrawer::GetInstance().AddSprite(m_Sprite);
}

void SpriteComponent3D::SetPosition(DirectX::SimpleMath::Vector3 pos)
{
	m_Sprite->SetPosition(pos);
}

void SpriteComponent3D::SetSize(DirectX::SimpleMath::Vector2 size)
{
	m_Sprite->SetSpriteSize(size.x, size.y);
}

void SpriteComponent3D::SetColor(DirectX::SimpleMath::Color color)
{
	m_Sprite->SetColor(color);
}

void SpriteComponent3D::SetFlip(bool xFLag, bool yFlag)
{
	m_Sprite->SetFlip(xFLag, yFlag);
}

void SpriteComponent3D::SetTexture(const std::string& textureName)
{
	m_Sprite->SetTexture(textureName);
}

void SpriteComponent3D::SetTextureRange(float tex_x, float tex_y, float tex_Width, float tex_Height)
{
	m_Sprite->SetTextureRange(tex_x, tex_y, tex_Width, tex_Height);
}

void SpriteComponent3D::SetAnchorPoint(DirectX::SimpleMath::Vector2 anchorPoint)
{
	m_Sprite->SetAnchorPoint(anchorPoint);
}

void SpriteComponent3D::SetScale(SimpleMath::Vector3 scale)
{
	m_Sprite->SetScale(scale);
}

void SpriteComponent3D::SetRotate(SimpleMath::Vector3 rotate)
{
	m_Sprite->SetRotate(rotate);
}

void SpriteComponent3D::SetBillBoardType(Sprite3D::BillBoardType billBoardType)
{
	m_Sprite->SetBillBoardType(billBoardType);
}

void SpriteComponent3D::DrawProperties()
{
}
