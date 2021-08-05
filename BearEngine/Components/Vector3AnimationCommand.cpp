#include "Vector3AnimationCommand.h"

#include "../Utility/Time.h"
#include "../Utility/Math/MathUtility.h"

Vector3AnimationCommand::Vector3AnimationCommand(const SimpleMath::Vector3& start, const SimpleMath::Vector3& target,SimpleMath::Vector3& pRet)
	:mTarget(target),mStart(start),mLerpTime(0.0f), mRet(pRet)
{
}

Vector3AnimationCommand::~Vector3AnimationCommand()
{
}

void Vector3AnimationCommand::Play()
{
	mLerpTime = 0.0f;
}

void Vector3AnimationCommand::AnimationUpdate()
{
	mLerpTime = MathUtility::Clamp(mLerpTime + Time::DeltaTime, 0.0f, 1.0f);

	mRet = SimpleMath::Vector3::Lerp(mStart, mTarget, mLerpTime);
}

bool Vector3AnimationCommand::IsEnd()
{
	return mLerpTime >= 1.0f;
}
