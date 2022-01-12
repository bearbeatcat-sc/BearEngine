#ifndef _VECTOR3_ANIMATION_COMMAND_H_
#define _VECTOR3_ANIMATION_COMMAND_H_

#include "AnimationCommand.h"

#include <SimpleMath.h>

using namespace DirectX;

class Vector3AnimationCommand
	:public AnimationCommand
{
public:
	Vector3AnimationCommand(const SimpleMath::Vector3& start,const SimpleMath::Vector3& target,SimpleMath::Vector3& pRet,const float speed = 1.0f,AnimationSpeedType animationspeedType = AnimationSpeedType::AnimationSpeedType_None);
	~Vector3AnimationCommand() override;

	void Play() override;
	void AnimationUpdate() override;
	bool IsEnd() override;

	SimpleMath::Vector3 _target;
	SimpleMath::Vector3 _start;
private:

	SimpleMath::Vector3 &_ret;

	float _lerpTime;
	float _speed;
};

#endif