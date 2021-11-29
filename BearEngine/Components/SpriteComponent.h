#ifndef _SPRITECOMPONENT_H_
#define _SPRITECOMPONENT_H_

#include "Component.h"
#include "../Device/Sprite2D.h"
#include <memory>
#include "SimpleMath.h"

class SpriteComponent
	:public Component
{
public:

	SpriteComponent(Actor* user,const std::string& textureName, const std::string& effectName, int updateOrder = 100, bool drawFlag = true);
	virtual ~SpriteComponent();

	virtual void Update() override;
	void Init(const std::string& textureName, const std::string& effectName, int updateOrder = 100);
	void SetPosition(DirectX::SimpleMath::Vector2 pos);
	void SetSize(DirectX::SimpleMath::Vector2 size);
	void SetColor(DirectX::SimpleMath::Color color);
	void SetFlip(bool xFLag, bool yFlag);
	void SetTextureRange(float tex_x, float tex_y, float tex_Width, float tex_Height);
	void SetTexture(const std::string& textureName);
	void SetAnchorPoint(DirectX::SimpleMath::Vector2 anchorPoint);
	void SetDrawFlag(bool flag);
	void DrawProperties() override;

protected:
	std::shared_ptr<Sprite2D> m_Sprite;
	bool m_DrawFlag;

};

#endif