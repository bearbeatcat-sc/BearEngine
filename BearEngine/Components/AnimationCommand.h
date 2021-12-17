#ifndef _ANIMATION_COMMAND_H_
#define _ANIMATION_COMMAND_H_

class AnimationCommand
{
public:
	enum AnimationSpeedType
	{
		AnimationSpeedType_None,
		AnimationSpeedType_InCubic,
		AnimationSpeedType_InElastic,
		AnimationSpeedType_OutSin,
		AnimationSpeedType_OutElastic,
		AnimationSpeedType_InOutCulbic,
		AnimationSpeedType_InOutElastic,
		AnimationSpeedType_Out_Quart,
	};

	AnimationCommand(AnimationSpeedType animationSpeedType)
		:_animationSpeedType(animationSpeedType)
	{}



	virtual ~AnimationCommand() = default;

	virtual void Play() = 0;
	virtual void AnimationUpdate() = 0;
	virtual bool IsEnd() = 0;

protected:
	AnimationSpeedType _animationSpeedType;
};

#endif