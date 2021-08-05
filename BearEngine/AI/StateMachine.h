#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include <string>
#include <map>
#include <memory>

class TransitionFlag;
class State;

class StateMachine
{

public:
	StateMachine();
	~StateMachine();
	void AddState(const std::string& name, std::shared_ptr<State> state);
	void DeleteState(const std::string& name);
	void ChangeState(const std::string& name);
	std::string GetState();
	//TransitionFlag* GetTransitonFlag();

	void Update();

private:
	void ChangeState();

private:
	std::map<std::string, std::shared_ptr<State>> m_States;
	std::shared_ptr<State> m_CurrentState;
	std::string m_CurrentStateName;
	//TransitionFlag* m_TransitionFlag;
};

#endif