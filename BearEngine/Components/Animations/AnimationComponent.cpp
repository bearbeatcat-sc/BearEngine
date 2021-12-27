#include "AnimationComponent.h"

#include "AnimationQue.h"

AnimationComponent::AnimationComponent(Actor* pActor)
	:Component(pActor)
{
	_animationQue = std::make_shared<AnimationQue>();
}

AnimationComponent::~AnimationComponent()
{
	
}

const std::string& AnimationComponent::GetCurrentState()
{
	return _animationQue->GetCurrentState();
}

void AnimationComponent::PlayAnimation(const std::string& animationName)
{
	_animationQue->PlayAnimation(animationName);
}

void AnimationComponent::AddAnimationState(std::shared_ptr<AnimationCommandList> command, const std::string& stateName,
	const std::string& nextState)
{
	_animationQue->AddAnimation(command, stateName, nextState);
}

void AnimationComponent::AddAnimationState(std::shared_ptr<AnimationCommandList> command, const std::string& stateName,
	AnimationQue::StandardAnimationStateType stateType)
{
	_animationQue->AddAnimation(command, stateName, stateType);

}

void AnimationComponent::DrawProperties()
{
}

void AnimationComponent::Update()
{
	AnimationUpdate();
}

void AnimationComponent::AnimationUpdate()
{
	_animationQue->UpdateAnimation();
}
