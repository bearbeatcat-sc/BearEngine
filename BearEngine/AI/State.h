#ifndef _STATE_H_
#define _STATE_H_

#include <string>
#include <vector>

class Actor;

class State
{
public:
	State(Actor* user) :m_User(user)
	{

	}

	virtual ~State() 
	{
		m_User = 0;
	}

	virtual void AddNextState(const std::string& nextState)
	{
		nextStates.push_back(nextState);
	}

	virtual void Update() = 0;
	virtual void Init() = 0;
	const std::string& GetNextState()
	{
		return currentNextStateName;
	}


protected:
	Actor* m_User;
	std::vector<std::string> nextStates;
	std::string currentNextStateName;
};

#endif