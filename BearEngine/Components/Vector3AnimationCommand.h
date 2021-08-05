#ifndef _VECTOR3_ANIMATION_COMMAND_H_
#define _VECTOR3_ANIMATION_COMMAND_H_

#include "AnimationCommand.h"

#include <SimpleMath.h>

using namespace DirectX;

class Vector3AnimationCommand
	:public AnimationCommand
{
public:
	Vector3AnimationCommand(const SimpleMath::Vector3& start,const SimpleMath::Vector3& target,SimpleMath::Vector3& pRet);
	~Vector3AnimationCommand() override;

	void Play() override;
	void AnimationUpdate() override;
	bool IsEnd() override;
private:
	SimpleMath::Vector3 mTarget;
	SimpleMath::Vector3 mStart;

	SimpleMath::Vector3 &mRet;

	float mLerpTime;
};

#endif