#include "AnimationQue.h"

#include "AnimationCommand.h"

AnimationQue::AnimationQue()
	:mCurrentIndex(0), mIsPlay(false), mIsLoop(false)
{
}

AnimationQue::~AnimationQue()
{
	mAnimationCommands.clear();
}

void AnimationQue::PlayAnimation(bool isLoop)
{	
	mCurrentIndex = 0;
	mIsPlay = true;
	mIsLoop = isLoop;
}

void AnimationQue::AddAnimation(std::shared_ptr<AnimationCommand> command)
{
	mAnimationCommands.push_back(command);
}

bool AnimationQue::IsPlay()
{
	return mIsPlay;
}

void AnimationQue::UpdateAnimation()
{
	mAnimationCommands[mCurrentIndex]->AnimationUpdate();
	
	if(mAnimationCommands[mCurrentIndex]->IsEnd())
	{
		ChangeAnimation();
		return;
	}
}

void AnimationQue::ChangeAnimation()
{
	if(mCurrentIndex == mAnimationCommands.size() -1)
	{
		if(mIsLoop)
		{
			PlayAnimation(true);
			return;
		}
		
		EndAnimation();
		return;
	}

	mCurrentIndex++;
	mAnimationCommands[mCurrentIndex]->Play();
}

void AnimationQue::EndAnimation()
{
	mIsPlay = false;
	mCurrentIndex = 0;
}
