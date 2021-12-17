#include "Vector3AnimationCommand.h"

#include "../Utility/Time.h"
#include "../Utility/Math/MathUtility.h"
#include "Utility/EasingUtil.h"

Vector3AnimationCommand::Vector3AnimationCommand(const SimpleMath::Vector3& start, const SimpleMath::Vector3& target,SimpleMath::Vector3& pRet,float speed, AnimationSpeedType animationSpeedType)
	:AnimationCommand(animationSpeedType),mTarget(target),mStart(start),mLerpTime(0.0f), mRet(pRet), m_Speed(speed)
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
	mLerpTime = MathUtility::Clamp(mLerpTime + Time::DeltaTime * m_Speed, 0.0f, 1.0f);

	float x = mLerpTime / 1.0f;

	switch(_animationSpeedType)
	{
	case AnimationSpeedType_None: 
		break;

	case AnimationSpeedType_InCubic:
		x = EasingUtil::ease_in_cubic(mLerpTime);
		break;

	case AnimationSpeedType_InElastic:
		x = EasingUtil::ease_in_elastic(mLerpTime);
		break;
	case AnimationSpeedType_OutSin:
		x = EasingUtil::ease_out_sin(mLerpTime);
		break;

	case AnimationSpeedType_OutElastic:
		x = EasingUtil::ease_in_elastic(mLerpTime);
		break;

	case AnimationSpeedType_InOutCulbic:
		x = EasingUtil::ease_inout_cublic(mLerpTime);
		break;

	case AnimationSpeedType_InOutElastic:
		x = EasingUtil::ease_inout_elastic(mLerpTime);
		break;

	case AnimationSpeedType_Out_Quart:
		x = EasingUtil::ease_out_quart(mLerpTime);
		break;

	default: 
		;
	}

	auto c = mTarget - mStart;
	

	mRet = c * x + mStart;

}

bool Vector3AnimationCommand::IsEnd()
{
	return mLerpTime >= 1.0f;
}
