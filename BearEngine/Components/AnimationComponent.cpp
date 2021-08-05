#include "AnimationComponent.h"

#include "AnimationQue.h"

AnimationComponent::AnimationComponent(Actor* pActor)
	:Component(pActor)
{
}

AnimationComponent::~AnimationComponent()
{
	animationQues.clear();
}

void AnimationComponent::PlayAnimation(const std::string& animationName)
{
	auto find = animationQues.find(animationName);

	if (find == animationQues.end())return;

	(*find).second->PlayAnimation();
}

void AnimationComponent::AddAnimationQue(const std::string& animationName, std::shared_ptr<AnimationQue> que)
{
	animationQues.emplace(animationName, que);
}

void AnimationComponent::Update()
{
	AnimationUpdate();
}

void AnimationComponent::AnimationUpdate()
{
	for(auto que : animationQues)
	{
		if(que.second->IsPlay())
		{
			que.second->UpdateAnimation();
		}
	}
}
