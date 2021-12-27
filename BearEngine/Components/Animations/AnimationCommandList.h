#pragma once
#include <memory>
#include <vector>

class AnimationCommand;

class AnimationCommandList
{
public:
	AnimationCommandList() = default;
	~AnimationCommandList() = default;

	void AnimationUpdate();
	void Play();
	bool IsEnd();
	void AddAnimation(std::shared_ptr<AnimationCommand> command);

private:
	std::vector<std::shared_ptr<AnimationCommand>> _animationCommands;

};
