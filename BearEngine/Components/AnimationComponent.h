#ifndef _ANIMATION_COMPONENT_H_
#define _ANIMATION_COMPONENT_H_

#include <map>

#include "Component.h"
#include <memory>
#include <vector>

class AnimationQue;

class AnimationComponent
	:public Component
{
public:
	AnimationComponent(Actor* pActor);
	~AnimationComponent();
	void PlayAnimation(const std::string& animationName);
	void AddAnimationQue(const std::string& animationName,std::shared_ptr<AnimationQue> que);

private:
	void Update() override;
	void AnimationUpdate();

private:
	std::map<std::string,std::shared_ptr<AnimationQue>> animationQues;
};

#endif
