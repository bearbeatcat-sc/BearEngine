#pragma once

#include <SimpleMath.h>
#include "CollisionComponent.h"

using namespace DirectX;


struct InterSectInfo
{
public:
	SimpleMath::Vector3 _PoisitionA;
	SimpleMath::Vector3 _PoisitionB;
	
	SimpleMath::Vector3 _InterSectPositionA;
	SimpleMath::Vector3 _InterSectPositionB;
	SimpleMath::Vector3 _Normal;

	Actor* _ActorA;
	Actor* _ActorB;
};
