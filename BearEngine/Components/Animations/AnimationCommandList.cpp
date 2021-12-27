#include "AnimationCommandList.h"

#include "AnimationCommand.h"

void AnimationCommandList::AnimationUpdate()
{
	for (auto itr = _animationCommands.begin(); itr != _animationCommands.end(); ++itr)
	{
		if ((*itr)->IsEnd())
		{
			continue;
		}

		(*itr)->AnimationUpdate();
	}
}

void AnimationCommandList::Play()
{
	for (auto itr = _animationCommands.begin(); itr != _animationCommands.end(); ++itr)
	{
		(*itr)->Play();
	}
}

bool AnimationCommandList::IsEnd()
{
	for (auto itr = _animationCommands.begin(); itr != _animationCommands.end(); ++itr)
	{
		if(!(*itr)->IsEnd())
		{
			return false;
		}
	}

	return true;
}

void AnimationCommandList::AddAnimation(std::shared_ptr<AnimationCommand> command)
{
	_animationCommands.push_back(command);
}
