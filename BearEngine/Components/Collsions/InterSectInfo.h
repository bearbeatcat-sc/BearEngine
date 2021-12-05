#pragma once

#include <SimpleMath.h>
#include "CollisionComponent.h"

using namespace DirectX;


struct InterSectInfo
{
public:
	SimpleMath::Vector3 _PoisitionA;
	SimpleMath::Vector3 _PoisitionB;
	

	SimpleMath::Vector3 _Normal;


	// 現状はOBB同士の交点の計算結果のみ扱う
	std::vector<SimpleMath::Vector3> _InterSectPositions;

	// OBBtoOBB以外での交点
	SimpleMath::Vector3 _InterSectPositionA;
	SimpleMath::Vector3 _InterSectPositionB;

	float depth = 0.0f;

	Actor* _ActorA;
	Actor* _ActorB;
};
