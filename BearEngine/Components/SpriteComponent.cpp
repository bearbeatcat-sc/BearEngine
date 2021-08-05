#include "SpriteComponent.h"
#include "../Device/SpriteDrawer.h"

SpriteComponent::SpriteComponent(Actor* user, const std::string& textureName, const std::string& effectName, int updateOrder, bool drawFlag)
	:Component(user,updateOrder), m_DrawFlag(drawFlag)
{
	Init(textureName,effectName,updateOrder);
}

SpriteComponent::~SpriteComponent()
{
	m_Sprite->Destroy();
}

void SpriteComponent::Update()
{
}

void SpriteComponent::Init(const std::string& textureName, const std::string& effectName,int updateOrder)
{
	m_Sprite = std::make_shared<Sprite2D>(DirectX::XMFLOAT2(0.5f, 0.5f), effectName,updateOrder);
	m_Sprite->Init(textureName);
	m_Sprite->SetEffectName(effectName);
	SpriteDrawer::GetInstance().AddSprite(m_Sprite);
	m_Sprite->SetDrawFlag(m_DrawFlag);
}

void SpriteComponent::SetPosition(DirectX::SimpleMath::Vector2 pos)
{
	m_Sprite->SetPosition(DirectX::XMFLOAT3(pos.x, pos.y, 0));
}

void SpriteComponent::SetSize(DirectX::SimpleMath::Vector2 size)
{
	m_Sprite->SetSpriteSize(size.x, size.y);
}

void SpriteComponent::SetColor(DirectX::SimpleMath::Color color)
{
	m_Sprite->SetColor(color);
}

void SpriteComponent::SetFlip(bool xFLag, bool yFlag)
{
	m_Sprite->SetFlip(xFLag, yFlag);
}

void SpriteComponent::SetTextureRange(float tex_x, float tex_y, float tex_Width, float tex_Height)
{
	m_Sprite->SetTextureRange(tex_x, tex_y, tex_Width, tex_Height);
}

void SpriteComponent::SetTexture(const std::string& textureName)
{
	m_Sprite->SetTexture(textureName);
}

void SpriteComponent::SetAnchorPoint(DirectX::SimpleMath::Vector2 anchorPoint)
{
	m_Sprite->SetAnchorPoint(anchorPoint);
}

void SpriteComponent::SetDrawFlag(bool flag)
{
	m_Sprite->SetDrawFlag(flag);
}
