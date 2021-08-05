#ifndef _PLANE_H_
#define _PLANE_H_

#include <SimpleMath.h>

using namespace DirectX;

class Plane
{
public:
	Plane(const SimpleMath::Vector3& normal,float distance = 0.0f);
	~Plane();


public:
	SimpleMath::Vector3 m_Normal{ 0,1,0};
	float m_Distance = 0.0f;
};

#endif