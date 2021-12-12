#include "CollisionInterSect.h"

#include "Components/Collsions/SphereCollisionComponent.h"
#include "Components/Collsions/OBBCollisionComponent.h"
#include <Components/Collsions/InterSectInfo.h>

#define CMP(x, y) \
	(fabsf(x - y) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

const bool CollisionInterSect::SphereToOBBInterSect(SphereCollisionComponent* sphere, OBBCollisionComponent* obb, SimpleMath::Vector3& point, InterSectInfo& interSect)
{
	auto pos = sphere->GetPosition();
	ClosetPtPointOBB(pos, obb, point);

	float radius = sphere->GetRadius();

	SimpleMath::Vector3 v = point - pos;

	float dot = v.Dot(v);

	if (dot >= radius * radius)
	{
		return false;
	}

	interSect._PoisitionA = pos;
	interSect._PoisitionB = obb->GetCenter();

	float test = CMP(dot, 0.0f);


	
	if (CMP(dot, 0.0f))
	{
		auto v2 = point - obb->GetCenter();
		float mSq = v2.Length();

		if (CMP(mSq, 0.0f))
		{
			return true;
		}

		interSect._Normal = v2;
	}
	else
	{


		interSect._Normal = pos - point;

	}

	interSect._Normal.Normalize();

	
	//interSect._InterSectPositionA = pos + interSect._Normal * radius;

	auto outsidePoint = pos - interSect._Normal * radius;
	auto vec = (point - outsidePoint);

	//float distance = sqrtf(vec.Dot(vec));
	float distance = sqrtf(vec.Length());

	interSect.depth = distance * 0.5f;

	interSect._InterSectPositions.push_back(
		point - (outsidePoint + point) * 0.5f);


	interSect._InterSectPositionA = pos + interSect._Normal * radius;
	interSect._InterSectPositionB = point + (outsidePoint - point) * 0.5f;

	return true;
}

// http://marupeke296.com/COL_3D_No13_OBBvsOBB.html
//const bool CollisionInterSect::OBBToOBBInterSect(OBBCollisionComponent* obb1, OBBCollisionComponent* obb2, InterSectInfo& interSect)
//{
//	auto obb1DirectionVec = obb1->GetDirectionVec();
//	auto obb2DirectionVec = obb2->GetDirectionVec();
//
//	SimpleMath::Vector3 obb1Size = obb1->GetSize();
//	SimpleMath::Vector3 obb2Size = obb2->GetSize();
//
//	SimpleMath::Vector3 NAe1 = obb1DirectionVec[0];
//	SimpleMath::Vector3 Ae1 = obb1DirectionVec[0] * obb1Size;
//
//	SimpleMath::Vector3 NAe2 = obb1DirectionVec[1];
//	SimpleMath::Vector3 Ae2 = obb1DirectionVec[1] * obb1Size;
//
//	SimpleMath::Vector3 NAe3 = obb1DirectionVec[2];
//	SimpleMath::Vector3 Ae3 = obb1DirectionVec[2] * obb1Size;
//
//	SimpleMath::Vector3 NBe1 = obb2DirectionVec[0];
//	SimpleMath::Vector3 Be1 = obb2DirectionVec[0] * obb2Size;
//
//	SimpleMath::Vector3 NBe2 = obb2DirectionVec[1];
//	SimpleMath::Vector3 Be2 = obb2DirectionVec[1] * obb2Size;
//
//	SimpleMath::Vector3 NBe3 = obb2DirectionVec[2];
//	SimpleMath::Vector3 Be3 = obb2DirectionVec[2] * obb2Size;
//
//	SimpleMath::Vector3 interval = obb1->GetCenter() - obb2->GetCenter();
//
//	//  分離軸 Ae1
//	float rA = Ae1.Length();
//	float rB = LensegOnSeparateAxis(NAe1, Be1, Be2, Be3);
//	float l = fabs(interval.Dot(NAe1));
//
//	if (l > rA + rB)
//		return false;
//
//	//  分離軸 Ae2
//	rA = Ae2.Length();
//	rB = LensegOnSeparateAxis(NAe2, Be1, Be2, Be3);
//	l = fabs(interval.Dot(NAe2));
//
//	if (l > rA + rB)
//		return false;
//
//	//  分離軸 Ae3
//	rA = Ae3.Length();
//	rB = LensegOnSeparateAxis(NAe3, Be1, Be2, Be3);
//	l = fabs(interval.Dot(NAe3));
//
//	if (l > rA + rB)
//		return false;
//
//
//	//  分離軸 Be1
//	rA = LensegOnSeparateAxis(NBe1, Ae1, Ae2, Ae3);
//	rB = Be1.Length();
//	l = fabs(interval.Dot(NBe1));
//
//	if (l > rA + rB)
//		return false;
//
//	//  分離軸 Be2
//	rA = LensegOnSeparateAxis(NBe2, Ae1, Ae2, Ae3);
//	rB = Be2.Length();
//	l = fabs(interval.Dot(NBe2));
//
//	if (l > rA + rB)
//		return false;
//
//	//  分離軸 Be3
//	rA = LensegOnSeparateAxis(NBe3, Ae1, Ae2, Ae3);
//	rB = Be3.Length();
//	l = fabs(interval.Dot(NBe3));
//
//	if (l > rA + rB)
//		return false;
//
//
//	// 分離軸 C11
//	SimpleMath::Vector3 cross = NAe1.Cross(NBe1);
//	rA = LensegOnSeparateAxis(cross, Ae2, Ae3);
//	rB = LensegOnSeparateAxis(cross, Be2, Be3);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//
//	// 分離軸 C12
//	cross = NAe1.Cross(NBe2);
//	rA = LensegOnSeparateAxis(cross, Ae2, Ae3);
//	rB = LensegOnSeparateAxis(cross, Be1, Be3);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C13
//	cross = NAe1.Cross(NBe3);
//	rA = LensegOnSeparateAxis(cross, Ae2, Ae3);
//	rB = LensegOnSeparateAxis(cross, Be1, Be2);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C21
//	cross = NAe2.Cross(NBe1);
//	rA = LensegOnSeparateAxis(cross, Ae1, Ae3);
//	rB = LensegOnSeparateAxis(cross, Be2, Be3);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C22
//	cross = NAe2.Cross(NBe2);
//	rA = LensegOnSeparateAxis(cross, Ae1, Ae3);
//	rB = LensegOnSeparateAxis(cross, Be1, Be3);
//	l =fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C23
//	cross = NAe2.Cross(NBe3);
//	rA = LensegOnSeparateAxis(cross, Ae1, Ae3);
//	rB = LensegOnSeparateAxis(cross, Be1, Be2);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C31
//	cross = NAe3.Cross(NBe1);
//	rA = LensegOnSeparateAxis(cross, Ae1, Ae2);
//	rB = LensegOnSeparateAxis(cross, Be2, Be3);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C32
//	cross = NAe3.Cross(NBe2);
//	rA = LensegOnSeparateAxis(cross, Ae1, Ae2);
//	rB = LensegOnSeparateAxis(cross, Be1, Be3);
//	l = fabs(interval.Dot(cross));
//
//	if (l > rA + rB)
//		return false;
//
//	// 分離軸 C33
//	cross = NAe3.Cross(NBe3);
//	rA = LensegOnSeparateAxis(cross, Ae1, Ae2);
//	rB = LensegOnSeparateAxis(cross, Be1, Be2);
//	l = fabs(interval.Dot(cross));
//	
//	if (l > rA + rB)
//		return false;	
//
//	return true;
//}

const bool CollisionInterSect::OBBToOBBInterSect(OBBCollisionComponent* obb1, OBBCollisionComponent* obb2, InterSectInfo& interSect)
{
	auto obb1DirectionVec = obb1->GetDirectionVec();
	auto obb2DirectionVec = obb2->GetDirectionVec();

	SimpleMath::Vector3 obb1Size = obb1->GetSize();
	SimpleMath::Vector3 obb2Size = obb2->GetSize();

	SimpleMath::Vector3 NAe1 = obb1DirectionVec[0];

	SimpleMath::Vector3 NAe2 = obb1DirectionVec[1];

	SimpleMath::Vector3 NAe3 = obb1DirectionVec[2];

	SimpleMath::Vector3 NBe1 = obb2DirectionVec[0];

	SimpleMath::Vector3 NBe2 = obb2DirectionVec[1];

	SimpleMath::Vector3 NBe3 = obb2DirectionVec[2];

	// 分離軸 C11
	auto c11 = NAe1.Cross(NBe1);

	// 分離軸 C12
	auto c12 = NAe1.Cross(NBe2);


	// 分離軸 C13
	auto c13 = NAe1.Cross(NBe3);


	// 分離軸 C21
	auto c21 = NAe2.Cross(NBe1);


	// 分離軸 C22
	auto c22 = NAe2.Cross(NBe2);


	// 分離軸 C23
	auto c23 = NAe2.Cross(NBe3);


	// 分離軸 C31
	auto c31 = NAe3.Cross(NBe1);


	// 分離軸 C32
	auto c32 = NAe3.Cross(NBe2);

	// 分離軸 C33
	auto c33 = NAe3.Cross(NBe3);

	SimpleMath::Vector3 test[15] =
	{
		obb1DirectionVec[0],
		obb1DirectionVec[1],
		obb1DirectionVec[2],

		obb2DirectionVec[0],
		obb2DirectionVec[1],
		obb2DirectionVec[2],

		c11,c12,c13,
		c21,c22,c23,
		c31,c32,c33
	};

	//for (int i = 0; i < 3; ++i) { // Fill out rest of axis
	//	test[6 + i * 3 + 0] = test[i].Cross(test[0]);
	//	test[6 + i * 3 + 1] = test[i].Cross(test[1]);
	//	test[6 + i * 3 + 2] = test[i].Cross(test[2]);
	//}
	
	SimpleMath::Vector3* hitNormal = nullptr;
	bool shouldFlip;

	for (int i = 0; i < 15; ++i)
	{
		if (test[i].Dot(test[i]) < 0.001f)
		{
			continue;
		}

		// 分離軸に投影した2つのOBBがどれぐらい重なっているか
		float depth = PenetrationDepth(
			obb1, obb2, &test[i], shouldFlip);

		if (depth <= 0.0f)
		{
			return false;
		}

		if (depth < interSect.depth)
		{
			if (shouldFlip)
			{
				test[i] = test[i] * -1.0f;
			}
		}

		interSect.depth = depth;
		hitNormal = &test[i];
	}

	// 衝突法線が見つからなかった場合は衝突していない
	if (hitNormal == nullptr)
	{
		return false;
	}

	auto axis = *hitNormal;
	axis.Normalize();

	std::vector<SimpleMath::Vector3> c1 = ClipEdgesToOBB(GetEdgs(obb2), obb1);
	std::vector<SimpleMath::Vector3> c2 = ClipEdgesToOBB(GetEdgs(obb1), obb2);

	interSect._InterSectPositions.reserve(c1.size() + c2.size());
	interSect._InterSectPositions.insert(interSect._InterSectPositions.end(),
		c1.begin(), c1.end());
	interSect._InterSectPositions.insert(interSect._InterSectPositions.end(),
		c2.begin(), c2.end());

	InterVal i = GetInterVal(obb1, &axis);
	float distance = (i._max - i._min) * 0.5f -
		interSect.depth * 0.5f;

	auto pointOnPlane = obb1->GetCenter() + axis * distance;

	for (int i = interSect._InterSectPositions.size() - 1; i >= 0; --i)
	{
		auto contact = interSect._InterSectPositions[i];

		interSect._InterSectPositions[i] = contact + (axis *
			axis.Dot(pointOnPlane - contact));

		for (int j = interSect._InterSectPositions.size() - 1; j > i; --j) {

			auto v2 = (interSect._InterSectPositions[j] - interSect._InterSectPositions[i]);

			if (v2.Dot(v2) < 0.0001f) {
				interSect._InterSectPositions.erase(interSect._InterSectPositions.begin() + j);
				break;
			}
		}
	}

	interSect._Normal = axis;
	interSect._PoisitionA = obb1->GetCenter();
	interSect._PoisitionB = obb2->GetCenter();

	return true;
}

const bool CollisionInterSect::SphereToSphereInterSect(SphereCollisionComponent* sphere1,
	SphereCollisionComponent* sphere2, InterSectInfo& interSect)
{
	DirectX::SimpleMath::Vector3 thisPos = sphere1->GetPosition();
	DirectX::SimpleMath::Vector3 otherPos = sphere2->GetPosition();
	float thisRadius = sphere1->GetRadius();
	float otherRadius = sphere2->GetRadius();

	interSect._PoisitionA = thisPos;
	interSect._PoisitionB = otherPos;

	SimpleMath::Vector3 ab = otherPos - thisPos;
	SimpleMath::Vector3 normal = ab;
	normal.Normalize();

	interSect._Normal = normal;
	interSect.depth = fabsf(normal.Length() - (thisRadius + otherRadius));

	float dtp = thisRadius - interSect.depth;
	SimpleMath::Vector3 contact = thisPos + normal * dtp;

	//interSect._InterSectPositionA = thisPos + normal * thisRadius;
	//interSect._InterSectPositionB = otherPos - normal * otherRadius;

	interSect._InterSectPositions.push_back(contact);

	//DirectX::SimpleMath::Vector3 temp = otherPos - thisPos;

	return (ab.x * ab.x) + (ab.y * ab.y) + (ab.z * ab.z) <= ((thisRadius + otherRadius) * (thisRadius + otherRadius));

}

const bool CollisionInterSect::PointToOBB(const SimpleMath::Vector3& point, OBBCollisionComponent* obb)
{
	SimpleMath::Vector3 d = point - obb->GetCenter();

	auto directionVec = obb->GetDirectionVec();
	XMVECTOR size = obb->GetSize();

	float dist;

	for (int i = 0; i < 3; ++i)
	{
		dist = d.Dot(directionVec[i]);

		if (dist > size.m128_f32[i])
		{
			//dist = size.m128_f32[i];
			return false;
		}
		if (dist < -size.m128_f32[i])
		{
			//dist = -size.m128_f32[i];
			return false;
		}
	}

	return true;
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

std::vector<SimpleMath::Vector3> CollisionInterSect::GetVertices(OBBCollisionComponent* obb)
{
	std::vector<SimpleMath::Vector3> v;
	v.resize(8);

	auto c = obb->GetCenter();
	auto  e = obb->GetSize();
	auto a = obb->GetDirectionVec();

	// 中心と軸の方向から頂点を求める
	v[0] = c + a[0] * e.x + a[1] * e.y + a[2] * e.z;
	v[1] = c - a[0] * e.x + a[1] * e.y + a[2] * e.z;

	v[2] = c + a[0] * e.x - a[1] * e.y + a[2] * e.z;
	v[3] = c + a[0] * e.x + a[1] * e.y - a[2] * e.z;


	v[4] = c - a[0] * e.x - a[1] * e.y - a[2] * e.z;
	v[5] = c + a[0] * e.x - a[1] * e.y - a[2] * e.z;

	v[6] = c - a[0] * e.x + a[1] * e.y - a[2] * e.z;
	v[7] = c - a[0] * e.x - a[1] * e.y + a[2] * e.z;

	return v;
}

std::vector<Line> CollisionInterSect::GetEdgs(OBBCollisionComponent* obb)
{
	std::vector<Line> result;
	result.reserve(12);

	auto v = GetVertices(obb);

	int index[][2] = {
		{6,1},{6,3},{6,4},{2,7},{2,5},{2,0},
		{0,1},{0,3},{7,1},{7,4},{4,5},{5,3},
	};

	// 頂点とインデックス情報からLineを算出
	for (int j = 0; j < 12; ++j)
	{
		result.push_back(
			Line(v[index[j][0]], v[index[j][1]]));
	}

	return result;
}

std::vector<Plane> CollisionInterSect::GetPlanes(OBBCollisionComponent* obb)
{
	auto c = obb->GetCenter();
	auto e = obb->GetSize();
	auto a = obb->GetDirectionVec();

	std::vector<Plane> result;
	result.resize(7);

	result[0] = Plane(a[0], a[0].Dot(c + a[0] * e.x));
	result[1] = Plane(a[0] * -1.0f, -(a[0].Dot(c - a[0] * e.x)));

	result[2] = Plane(a[1], a[1].Dot(c + a[1] * e.y));
	result[3] = Plane(a[1] * -1.0f, -(a[1].Dot(c - a[1] * e.y)));

	result[4] = Plane(a[2], a[2].Dot(c + a[2] * e.z));
	result[5] = Plane(a[2] * -1.0f, -(a[2].Dot(c - a[2] * e.z)));

	return result;
}

bool CollisionInterSect::ClipToPlane(const Plane& plane, const Line& line, SimpleMath::Vector3& outPoint)
{
	auto ab = line._end - line._start;

	float nAB = plane._normal.Dot(ab);

	// 線と平面の交差チェック
	if (CMP(nAB, 0))
	{
		return false;
	}

	float nA = plane._normal.Dot(line._start);
	float t = (plane._distance - nA) / nAB;

	if (t >= 0.0f && t <= 1.0f)
	{
		outPoint = line._start + ab * t;

		return true;
	}

	return false;

}

std::vector<SimpleMath::Vector3> CollisionInterSect::ClipEdgesToOBB(const std::vector<Line>& edges,
	OBBCollisionComponent* obb)
{
	std::vector<SimpleMath::Vector3> result;

	result.reserve(edges.size());

	SimpleMath::Vector3 intersection;

	std::vector<Plane> planes = GetPlanes(obb);

	for (int i = 0; i < planes.size(); ++i)
	{
		for (int j = 0; j < edges.size(); ++j)
		{
			if (ClipToPlane(planes[i],
				edges[j], intersection))
			{

				if (PointToOBB(intersection, obb))
				{
					result.push_back(intersection);
				}
			}
		}
	}

	return result;
}

float CollisionInterSect::PenetrationDepth(OBBCollisionComponent* o1, OBBCollisionComponent* o2,
	const SimpleMath::Vector3* axis, bool& outShouldFlip)
{
	auto _axis = SimpleMath::Vector3(axis->x, axis->y, axis->z);
	_axis.Normalize();

	InterVal i1 = GetInterVal(o1, &_axis);
	InterVal i2 = GetInterVal(o2, &_axis);

	// 軸に投影した2つの線が重ならない場合
	if (!((i2._min <= i1._max) && (i1._min <= i2._max)))
	{
		return 0.0f;
	}

	float len1 = i1._max - i1._min;
	float len2 = i2._max - i2._min;

	float min = fminf(i1._min, i2._min);
	float max = fmaxf(i1._max, i2._max);

	float length = max - min;

	// 衝突法線を反転させるかの判定
	if (outShouldFlip != 0)
	{
		outShouldFlip = (i2._min < i1._min);
	}

	return (len1 + len2) - length;
}

void CollisionInterSect::FindCollisionFeatrures(OBBCollisionComponent* obb1, OBBCollisionComponent obb2,
	InterSectInfo& interSect)
{

}

// 軸に投影した長さを取得する
InterVal CollisionInterSect::GetInterVal(OBBCollisionComponent* obb, const SimpleMath::Vector3* axis)
{
	SimpleMath::Vector3 vertex[8];

	auto C = obb->GetCenter();
	auto E = obb->GetSize();

	auto A = obb->GetDirectionVec();

	vertex[0] = C + A[0] * E.x + A[1] * E.y + A[2] * E.z;
	vertex[1] = C - A[0] * E.x + A[1] * E.y + A[2] * E.z;
	vertex[2] = C + A[0] * E.x - A[1] * E.y + A[2] * E.z;
	vertex[3] = C + A[0] * E.x + A[1] * E.y - A[2] * E.z;
	vertex[4] = C - A[0] * E.x - A[1] * E.y - A[2] * E.z;
	vertex[5] = C + A[0] * E.x - A[1] * E.y - A[2] * E.z;
	vertex[6] = C - A[0] * E.x + A[1] * E.y - A[2] * E.z;
	vertex[7] = C - A[0] * E.x - A[1] * E.y + A[2] * E.z;

	InterVal result;
	result._min = result._max = axis->Dot(vertex[0]);

	// 軸に投影した線の長さを取得する？
	for (int i = 1; i < 8; ++i)
	{
		float projection = axis->Dot(vertex[i]);
		result._min = (projection < result._min) ? projection : result._min;
		result._max = (projection > result._max) ? projection : result._max;
	}

	return result;
}
