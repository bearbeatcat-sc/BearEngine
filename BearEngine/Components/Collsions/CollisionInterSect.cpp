#include "CollisionInterSect.h"

#include "Components/Collsions/SphereCollisionComponent.h"
#include "Components/Collsions/OBBCollisionComponent.h"

const bool CollisionInterSect::SphereToOBBInterSect(SphereCollisionComponent* sphere, OBBCollisionComponent* obb, SimpleMath::Vector3& point)
{
	auto pos = sphere->GetPosition();
	ClosetPtPointOBB(pos, obb, point);

	float radius = sphere->GetRadius();

	SimpleMath::Vector3 v = point - pos;

	float dot = v.Dot(v);

	return dot <= radius * radius;
}

// http://marupeke296.com/COL_3D_No13_OBBvsOBB.html
const bool CollisionInterSect::OBBToOBBInterSect(OBBCollisionComponent* obb1, OBBCollisionComponent* obb2)
{
	auto obb1DirectionVec = obb1->GetDirectionVec();
	auto obb2DirectionVec = obb2->GetDirectionVec();

	SimpleMath::Vector3 obb1Size = obb1->GetSize();
	SimpleMath::Vector3 obb2Size = obb2->GetSize();

	SimpleMath::Vector3 NAe1 = obb1DirectionVec[0];
	SimpleMath::Vector3 Ae1 = obb1DirectionVec[0] * obb1Size;

	SimpleMath::Vector3 NAe2 = obb1DirectionVec[1];
	SimpleMath::Vector3 Ae2 = obb1DirectionVec[1] * obb1Size;

	SimpleMath::Vector3 NAe3 = obb1DirectionVec[2];
	SimpleMath::Vector3 Ae3 = obb1DirectionVec[2] * obb1Size;

	SimpleMath::Vector3 NBe1 = obb2DirectionVec[0];
	SimpleMath::Vector3 Be1 = obb2DirectionVec[0] * obb2Size;

	SimpleMath::Vector3 NBe2 = obb2DirectionVec[1];
	SimpleMath::Vector3 Be2 = obb2DirectionVec[1] * obb2Size;

	SimpleMath::Vector3 NBe3 = obb2DirectionVec[2];
	SimpleMath::Vector3 Be3 = obb2DirectionVec[2] * obb2Size;

	SimpleMath::Vector3 interval = obb1->GetCenter() - obb2->GetCenter();

	//  分離軸 Ae1
	float rA = Ae1.Length();
	float rB = LensegOnSeparateAxis(NAe1, Be1, Be2, Be3);
	float l = fabs(interval.Dot(NAe1));

	if (l > rA + rB)
		return false;

	//  分離軸 Ae2
	rA = Ae2.Length();
	rB = LensegOnSeparateAxis(NAe2, Be1, Be2, Be3);
	l = fabs(interval.Dot(NAe2));

	if (l > rA + rB)
		return false;

	//  分離軸 Ae3
	rA = Ae3.Length();
	rB = LensegOnSeparateAxis(NAe3, Be1, Be2, Be3);
	l = fabs(interval.Dot(NAe3));

	if (l > rA + rB)
		return false;


	//  分離軸 Be1
	rA = LensegOnSeparateAxis(NBe1, Ae1, Ae2, Ae3);
	rB = Be1.Length();
	l = fabs(interval.Dot(NBe1));

	if (l > rA + rB)
		return false;

	//  分離軸 Be2
	rA = LensegOnSeparateAxis(NBe2, Ae1, Ae2, Ae3);
	rB = Be2.Length();
	l = fabs(interval.Dot(NBe2));

	if (l > rA + rB)
		return false;

	//  分離軸 Be3
	rA = LensegOnSeparateAxis(NBe3, Ae1, Ae2, Ae3);
	rB = Be3.Length();
	l = fabs(interval.Dot(NBe3));

	if (l > rA + rB)
		return false;


	// 分離軸 C11
	SimpleMath::Vector3 cross = NAe1.Cross(NBe1);
	rA = LensegOnSeparateAxis(cross, Ae2, Ae3);
	rB = LensegOnSeparateAxis(cross, Be2, Be3);
	l = fabs(interval.Dot(cross));
	
	if (l > rA + rB)
		return false;


	// 分離軸 C12
	 cross = NAe1.Cross(NBe2);
	rA = LensegOnSeparateAxis(cross, Ae2, Ae3);
	rB = LensegOnSeparateAxis(cross, Be1, Be3);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C13
	cross = NAe1.Cross(NBe3);
	rA = LensegOnSeparateAxis(cross, Ae2, Ae3);
	rB = LensegOnSeparateAxis(cross, Be1, Be2);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C21
	cross = NAe2.Cross(NBe1);
	rA = LensegOnSeparateAxis(cross, Ae1, Ae3);
	rB = LensegOnSeparateAxis(cross, Be2, Be3);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C22
	cross = NAe2.Cross(NBe2);
	rA = LensegOnSeparateAxis(cross, Ae1, Ae3);
	rB = LensegOnSeparateAxis(cross, Be1, Be3);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C23
	cross = NAe2.Cross(NBe3);
	rA = LensegOnSeparateAxis(cross, Ae1, Ae3);
	rB = LensegOnSeparateAxis(cross, Be1, Be2);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C31
	cross = NAe3.Cross(NBe1);
	rA = LensegOnSeparateAxis(cross, Ae1, Ae2);
	rB = LensegOnSeparateAxis(cross, Be2, Be3);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C32
	cross = NAe3.Cross(NBe2);
	rA = LensegOnSeparateAxis(cross, Ae1, Ae2);
	rB = LensegOnSeparateAxis(cross, Be1, Be3);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;

	// 分離軸 C33
	cross = NAe3.Cross(NBe3);
	rA = LensegOnSeparateAxis(cross, Ae1, Ae2);
	rB = LensegOnSeparateAxis(cross, Be1, Be2);
	l = fabs(interval.Dot(cross));

	if (l > rA + rB)
		return false;


	return true;
}

const bool CollisionInterSect::SphereToSphereInterSect(SphereCollisionComponent* sphere1,
	SphereCollisionComponent* sphere2)
{	
	DirectX::SimpleMath::Vector3 thisPos = sphere1->GetPosition();
	DirectX::SimpleMath::Vector3 otherPos = sphere2->GetPosition();
	float thisRadius = sphere1->GetRadius();
	float otherRadius = sphere2->GetRadius();


	DirectX::SimpleMath::Vector3 temp = otherPos - thisPos;
	float sum_Radius = thisRadius + otherRadius;

	return (temp.x * temp.x) + (temp.y * temp.y) + (temp.z * temp.z) <= ((thisRadius + otherRadius) * (thisRadius + otherRadius));

}

const void CollisionInterSect::ClosetPtPointOBB(SimpleMath::Vector3& p, OBBCollisionComponent* obb,
                                                SimpleMath::Vector3& q)
{
	SimpleMath::Vector3 d = p - obb->GetCenter();
	q = obb->GetCenter();

	auto directionVec = obb->GetDirectionVec();
	XMVECTOR size = obb->GetSize();

	float dist;

	for (int i = 0; i < 3; ++i)
	{
		dist = d.Dot(directionVec[i]);

		if (dist > size.m128_f32[i])
		{
			dist = size.m128_f32[i];
		}
		if (dist < -size.m128_f32[i])
		{
			dist = -size.m128_f32[i];
		}

		q += dist * directionVec[i];
	}

}

// 分離軸に投影した軸成分からの投影線分長を算出
const float CollisionInterSect::LensegOnSeparateAxis(const SimpleMath::Vector3& sep, const SimpleMath::Vector3& e1,
	const SimpleMath::Vector3& e2, const SimpleMath::Vector3& e3)
{
	float r1 = fabs(sep.Dot(e1));
	float r2 = fabs(sep.Dot(e2));
	float r3 = 0;


	if (!e3.Length() <= 0)
	{
		r3 = fabs(sep.Dot(e3));
	}

	return r1 + r2 + r3;
}
