#include "StateMachine.h"
#include "State.h"

StateMachine::StateMachine()
{

}

StateMachine::~StateMachine()
{
	m_States.clear();
}

void StateMachine::AddState(const std::string& name, std::shared_ptr<State> state)
{
	m_States.emplace(name, state);
}

void StateMachine::DeleteState(const std::string& name)
{
	auto find = m_States.find(name);

	if (find != m_States.end())return;

	m_States.erase(name);
}

void StateMachine::ChangeState(const std::string& name)
{
	m_CurrentState = m_States.at(name);
	m_CurrentStateName = name;
}

std::string StateMachine::GetState()
{
	return m_CurrentStateName;
}



void StateMachine::Update()
{
	if (m_CurrentState == nullptr)return;

	m_CurrentState->Update();

}

void StateMachine::ChangeState()
{
	m_CurrentState = m_States.at(m_CurrentState->GetNextState());
}
