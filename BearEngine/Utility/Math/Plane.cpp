#include "Plane.h"

Plane::Plane(const SimpleMath::Vector3& normal, float distance)
	:m_Normal(normal),m_Distance(distance)
{
}

Plane::~Plane()
{
}
