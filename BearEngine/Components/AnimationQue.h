#ifndef _ANIMATION_QUE_H_
#define _ANIMATION_QUE_H_

#include <memory>
#include <vector>

class AnimationCommand;

class AnimationQue
{
public:
	AnimationQue();
	~AnimationQue();

	void PlayAnimation(bool isLoop = false);
	void AddAnimation(std::shared_ptr<AnimationCommand> command);
	bool IsPlay();
	void UpdateAnimation();

private:
	void ChangeAnimation();
	void EndAnimation();

private:
	int mMaxSize;
	std::vector<std::shared_ptr<AnimationCommand>> mAnimationCommands;
	int mCurrentIndex;
	bool mIsPlay;
	bool mIsLoop;
};

#endif