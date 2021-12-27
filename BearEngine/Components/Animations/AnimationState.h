#pragma once
#include <memory>
#include <string>

class AnimationCommandList;

struct AnimationState
{
public:
	AnimationState(std::shared_ptr<AnimationCommandList> animationCommandList, const std::string& stateName)
		:_animationCommandList(animationCommandList), _stateName(stateName), _nextStete("")
	{

	}

	AnimationState(std::shared_ptr<AnimationCommandList> animationCommandList,const std::string& stateName, const std::string& destState)
		:_animationCommandList(animationCommandList), _nextStete(destState),_stateName(stateName)
	{
		
	}

	void Play()
	{
		_animationCommandList->Play();
	}

	void Playing()
	{
		_animationCommandList->AnimationUpdate();
	}

	bool IsEnd()
	{
		return _animationCommandList->IsEnd();
	}

	const std::string& NextState() { return _nextStete; }
	const std::string& StateName() { return _stateName; }

	~AnimationState() = default;

private:
	std::string _nextStete;
	std::string _stateName;
	std::shared_ptr<AnimationCommandList> _animationCommandList;
};
