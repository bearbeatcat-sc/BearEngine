#pragma once

#include <SimpleMath.h>

class SphereCollisionComponent;
class OBBCollisionComponent;

using namespace DirectX;

class CollisionInterSect
{
public:
	static const bool SphereToOBBInterSect(SphereCollisionComponent* sphere, OBBCollisionComponent* obb,SimpleMath::Vector3& point);
	static const bool OBBToOBBInterSect(OBBCollisionComponent* obb1, OBBCollisionComponent* obb2);
	static const bool SphereToSphereInterSect(SphereCollisionComponent* sphere1, SphereCollisionComponent* sphere2);

private:
	static const void ClosetPtPointOBB(SimpleMath::Vector3& p, OBBCollisionComponent* obb, SimpleMath::Vector3& q);
	
	static const float LensegOnSeparateAxis(const SimpleMath::Vector3& sep, const SimpleMath::Vector3& e1, const SimpleMath::Vector3& e2, const SimpleMath::Vector3& e3 = SimpleMath::Vector3(0,0,0));
};
