#ifndef _SPRITE_COMPONENT_3D_H_
#define _SPRITE_COMPONENT_3D_H_

#include "Component.h"
#include <memory>
#include "SimpleMath.h"
#include "../Device/Sprite3D.h"


class SpriteComponent3D
	:public Component

{
public:
	SpriteComponent3D(Actor* user, const std::string& textureName, const std::string& effectName, int updateOrder = 100);
	~SpriteComponent3D();
	void Init(const std::string& textureName, const std::string& effectName);
	virtual void Update() override;
	void SetPosition(DirectX::SimpleMath::Vector3 pos);
	void SetSize(DirectX::SimpleMath::Vector2 size);
	void SetColor(DirectX::SimpleMath::Color color);
	void SetFlip(bool xFLag, bool yFlag);
	void SetTexture(const std::string& textureName);
	void SetTextureRange(float tex_x, float tex_y, float tex_Width, float tex_Height);
	void SetAnchorPoint(DirectX::SimpleMath::Vector2 anchorPoint);
	void SetScale(SimpleMath::Vector3 scale);
	void SetRotate(SimpleMath::Vector3 rotate);
	void SetBillBoardType(Sprite3D::BillBoardType billBoardType);

protected:
	std::shared_ptr<Sprite3D> m_Sprite;

};

#endif