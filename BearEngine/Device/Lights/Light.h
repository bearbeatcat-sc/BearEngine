#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <SimpleMath.h>

using namespace DirectX;

class Light
{
public:
	Light();
	virtual ~Light();
	const SimpleMath::Vector3& GetDirection();
	const SimpleMath::Color& GetColor();
	void SetDirection(SimpleMath::Vector3& dir);
	void SetColor(const SimpleMath::Color& color);

protected:
	SimpleMath::Vector3 m_Direction;
	SimpleMath::Color m_LightColor;

	bool m_DirtyFlag;
};

#endif