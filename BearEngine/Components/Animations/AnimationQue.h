#ifndef _ANIMATION_QUE_H_
#define _ANIMATION_QUE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

class AnimationCommandList;
class AnimationCommand;
class AnimationState;

class AnimationQue
{
public:
	AnimationQue();
	~AnimationQue();

	enum StandardAnimationStateType
	{
		AnimationStateType_None,
		AnimationStateType_End,
	};

	void PlayAnimation(const std::string& stateName);
	void AddAnimation(std::shared_ptr<AnimationCommandList> command, const std::string& stateName ,const std::string& nextState = "");
	void AddAnimation(std::shared_ptr<AnimationCommandList> command, const std::string& stateName , StandardAnimationStateType stateType);
	void UpdateAnimation();
	const std::string& GetCurrentState();

private:
	void ChangeAnimation();
	void ChangeAnimation(const std::string& stateName);
	void EndAnimation();
	std::shared_ptr<AnimationState> FindAnimation(const std::string& stateName);

private:
	int _maxSize;
	std::map<std::string,std::shared_ptr<AnimationState>> _animationStates;
	std::shared_ptr<AnimationState> _currentState;
	std::string _currentStateName;
};

#endif