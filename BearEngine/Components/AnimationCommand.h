#ifndef _ANIMATION_COMMAND_H_
#define _ANIMATION_COMMAND_H_

class AnimationCommand
{
public:
	AnimationCommand() {}
	virtual ~AnimationCommand() = default;

	virtual void Play() = 0;
	virtual void AnimationUpdate() = 0;
	virtual bool IsEnd() = 0;	
};

#endif