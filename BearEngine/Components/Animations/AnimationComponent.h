#ifndef _ANIMATION_COMPONENT_H_
#define _ANIMATION_COMPONENT_H_

#include <memory>
#include <string>
#include <Components/Component.h>
#include <Components/Animations/AnimationQue.h>

class AnimationCommandList;
class AnimationQue;

class AnimationComponent
	:public Component
{
public:
	AnimationComponent(Actor* pActor);
	~AnimationComponent();
	const std::string& GetCurrentState();
	void PlayAnimation(const std::string& stateName);
	void AddAnimationState(std::shared_ptr<AnimationCommandList> command, const std::string& stateName, const std::string& nextState);
	void AddAnimationState(std::shared_ptr<AnimationCommandList> command, const std::string& stateName, AnimationQue::StandardAnimationStateType stateType);
	void DrawProperties() override;
private:
	void Update() override;
	void AnimationUpdate();


private:
	std::shared_ptr <AnimationQue> _animationQue;
};

#endif
