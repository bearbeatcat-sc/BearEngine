#include "AnimationQue.h"

#include <stdexcept>

#include "AnimationCommandList.h"
#include "AnimationState.h"

AnimationQue::AnimationQue()
{
}

AnimationQue::~AnimationQue()
{
	_animationStates.clear();
}

void AnimationQue::PlayAnimation(const std::string& stateName)
{
	ChangeAnimation(stateName);
}

void AnimationQue::AddAnimation(std::shared_ptr<AnimationCommandList> command, const std::string& stateName, const std::string& nextState)
{
	_animationStates.emplace(stateName, std::make_shared<AnimationState>(command, stateName, nextState));
}

void AnimationQue::AddAnimation(std::shared_ptr<AnimationCommandList> command, const std::string& stateName,
	StandardAnimationStateType stateType)
{
	auto nextState = "";

	switch (stateType)
	{
	case AnimationStateType_None:
		nextState = "None";
		break;

	case AnimationStateType_End:
		nextState = "End";
		break;

	default:
		nextState = "None";
	}

	_animationStates.emplace(stateName, std::make_shared<AnimationState>(command, stateName, nextState));
}

void AnimationQue::UpdateAnimation()
{

	// NoneやEndなど、アニメーションを行わない場合があるため
	if (_currentState == nullptr)return;

	_currentState->Playing();

	if (_currentState->IsEnd())
	{
		ChangeAnimation();
		return;
	}
}

const std::string& AnimationQue::GetCurrentState()
{
	return _currentStateName;
}

void AnimationQue::ChangeAnimation()
{
	if (_currentState == nullptr)
	{
		throw std::runtime_error("Not Play AnimationState.");
	}

	auto nextStateName = _currentState->NextState();

	if(nextStateName == "None" || nextStateName == "End")
	{
		_currentStateName = nextStateName;
		_currentState = nullptr;
		return;
	}

	auto find = FindAnimation(nextStateName);

	if (find == nullptr)
	{
		throw std::runtime_error("Not Regist AnimationState.");
	}

	_currentState = find;
	_currentStateName = find->StateName();
	_currentState->Play();
}

void AnimationQue::ChangeAnimation(const std::string& stateName)
{
	if (stateName == "None" || stateName == "End")
	{
		_currentStateName = stateName;
		_currentState = nullptr;
		return;
	}

	auto find = FindAnimation(stateName);

	if (find == nullptr)
	{
		throw std::runtime_error("Not Regist AnimationState.");
	}

	_currentState = find;
	_currentStateName = find->StateName();
	_currentState->Play();
}

std::shared_ptr<AnimationState> AnimationQue::FindAnimation(const std::string& stateName)
{
	auto result = _animationStates.find(stateName);

	if (result != _animationStates.end())
	{
		return result->second;
	}

	return nullptr;
}

