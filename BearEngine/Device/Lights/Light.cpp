#include "Light.h"

Light::Light()
	:m_Direction(SimpleMath::Vector3(0, 0, 1)), m_LightColor(1, 1, 1, 1)
{
}

Light::~Light()
{
}

const SimpleMath::Vector3& Light::GetDirection()
{
	return m_Direction;
}

const SimpleMath::Color& Light::GetColor()
{
	return m_LightColor;
}

void Light::SetDirection(SimpleMath::Vector3& dir)
{
	dir.Normalize();
	m_Direction = dir;
	m_DirtyFlag = true;
}
void Light::SetColor(const SimpleMath::Color& color)
{
	m_LightColor = color;
	m_DirtyFlag = true;
}

