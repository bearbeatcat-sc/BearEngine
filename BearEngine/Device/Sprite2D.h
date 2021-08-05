#ifndef _SPRITE_2D_H_
#define _SPRITE_2D_H_

#include "Sprite.h"

class Camera;

class Sprite2D
	:public Sprite
{
public:
	Sprite2D(XMFLOAT2 anchorpoint, const std::string& effectName,int drawOrder = 100);
	Sprite2D(XMFLOAT2 anchorpoint, const std::string& effectName, const std::string& cameraName);
	~Sprite2D();
	virtual void Draw(ID3D12GraphicsCommandList* tempCommand) override;

private:
	std::shared_ptr<Camera> m_Camera;
};

#endif