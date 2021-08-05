#ifndef _SPRITE_3D_H_
#define _SPRITE_3D_H_

#include "Sprite.h"


class Camera;

class Sprite3D
	:public Sprite
{
public:
	Sprite3D(XMFLOAT2 anchorpoint, const std::string& effectName);
	Sprite3D(XMFLOAT2 anchorpoint, const std::string& effectName,const std::string& cameraName);
	~Sprite3D();

	enum BillBoardType
	{
		BillBoard_Type_None,
		BillBoard_Type_Normal,
		BillBoard_Type_Y,
	};

	virtual void Draw(ID3D12GraphicsCommandList* tempCommand) override;
	void SetScale(SimpleMath::Vector3 scale);
	void SetRotate(SimpleMath::Vector3 rotate);
	void SetBillBoardType(BillBoardType billBoardType);

private:
	std::shared_ptr<Camera> m_Camera;
	SimpleMath::Vector3 m_Scale;
	SimpleMath::Vector3 m_Rotate;
	BillBoardType m_BillBoardType;
};

#endif