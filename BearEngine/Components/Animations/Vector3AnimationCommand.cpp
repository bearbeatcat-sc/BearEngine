#include "Vector3AnimationCommand.h"


#include "Utility/EasingUtil.h"
#include "Utility/Time.h"
#include "Utility/Math/MathUtility.h"

Vector3AnimationCommand::Vector3AnimationCommand(const SimpleMath::Vector3& start, const SimpleMath::Vector3& target,SimpleMath::Vector3& pRet,float speed, AnimationSpeedType animationSpeedType)
	:AnimationCommand(animationSpeedType),_target(target),_start(start),_lerpTime(0.0f), _ret(pRet), _speed(speed)
{
}

Vector3AnimationCommand::~Vector3AnimationCommand()
{
}

void Vector3AnimationCommand::Play()
{
	_lerpTime = 0.0f;
}

void Vector3AnimationCommand::AnimationUpdate()
{
	_lerpTime = MathUtility::Clamp(_lerpTime + Time::DeltaTime * _speed, 0.0f, 1.0f);

	float x = _lerpTime / 1.0f;

	switch(_animationSpeedType)
	{
	case AnimationSpeedType_None: 
		break;

	case AnimationSpeedType_InCubic:
		x = EasingUtil::ease_in_cubic(_lerpTime);
		break;

	case AnimationSpeedType_InElastic:
		x = EasingUtil::ease_in_elastic(_lerpTime);
		break;
	case AnimationSpeedType_OutSin:
		x = EasingUtil::ease_out_sin(_lerpTime);
		break;

	case AnimationSpeedType_OutElastic:
		x = EasingUtil::ease_in_elastic(_lerpTime);
		break;

	case AnimationSpeedType_InOutCulbic:
		x = EasingUtil::ease_inout_cublic(_lerpTime);
		break;

	case AnimationSpeedType_InOutElastic:
		x = EasingUtil::ease_inout_elastic(_lerpTime);
		break;

	case AnimationSpeedType_Out_Quart:
		x = EasingUtil::ease_out_quart(_lerpTime);
		break;

	default: 
		;
	}

	auto c = _target - _start;
	

	_ret = c * x + _start;

}

bool Vector3AnimationCommand::IsEnd()
{
	return _lerpTime >= 1.0f;
}
