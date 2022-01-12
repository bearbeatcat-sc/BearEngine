#pragma once

#include <SimpleMath.h>


class SphereCollisionComponent;
class OBBCollisionComponent;
class RayCollisionComponent;

struct InterSectInfo;

using namespace DirectX;

struct Line
{
	SimpleMath::Vector3 _start;
	SimpleMath::Vector3 _end;

	inline Line() {};
	inline Line(const SimpleMath::Vector3& s,const SimpleMath::Vector3 e)
		:_start(s), _end(e){}
};

struct Plane
{
	SimpleMath::Vector3 _normal;
	float _distance;

	inline Plane() : _normal(1,0,0){}
	inline Plane(const SimpleMath::Vector3& n,float d):
		_normal(n), _distance(d){}
};

struct InterVal
{
	float _min;
	float _max;
};

class CollisionInterSect
{
public:
	static const bool SphereToOBBInterSect(SphereCollisionComponent* sphere, OBBCollisionComponent* obb,SimpleMath::Vector3& point, InterSectInfo& interSect);
	static const bool OBBToOBBInterSect(OBBCollisionComponent* obb1, OBBCollisionComponent* obb2, InterSectInfo& interSect);
	static const bool OBBToRayInterSect(OBBCollisionComponent* obb1, RayCollisionComponent* ray, InterSectInfo& interSect);
	static const bool SphereToSphereInterSect(SphereCollisionComponent* sphere1, SphereCollisionComponent* sphere2, InterSectInfo& interSect);
	static const bool PointToOBB(const SimpleMath::Vector3& point, OBBCollisionComponent* obb);

private:
	static const void ClosetPtPointOBB(SimpleMath::Vector3& p, OBBCollisionComponent* obb, SimpleMath::Vector3& q);	
	static const float LensegOnSeparateAxis(const SimpleMath::Vector3& sep, const SimpleMath::Vector3& e1, const SimpleMath::Vector3& e2, const SimpleMath::Vector3& e3 = SimpleMath::Vector3(0,0,0));

	static std::vector<SimpleMath::Vector3> GetVertices(OBBCollisionComponent* obb);
	static std::vector<Line> GetEdgs(OBBCollisionComponent* obb);
	static std::vector<Plane> GetPlanes(OBBCollisionComponent* obb);

	static bool ClipToPlane(const Plane& plane,
		const Line& line, SimpleMath::Vector3& outPoint);

	static std::vector<SimpleMath::Vector3> ClipEdgesToOBB(
		const std::vector<Line>& edges, OBBCollisionComponent* obb);

	static float PenetrationDepth(OBBCollisionComponent* o1, OBBCollisionComponent* o2,
		const SimpleMath::Vector3* axis, bool& outShouldFlip);

	static void FindCollisionFeatrures(OBBCollisionComponent* obb1, OBBCollisionComponent obb2, InterSectInfo& interSect);

	static InterVal GetInterVal(OBBCollisionComponent* obb, const SimpleMath::Vector3* axis);
};
