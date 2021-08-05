#include "ParticleActionManager.h"
#include "NormalParticleAction.h"
#include "ParticleSequence.h"
#include "ParticleAction.h"

ParticleActionManager::ParticleActionManager()
{
}

ParticleActionManager::~ParticleActionManager()
{
	m_Sequences.clear();
	m_Actions.clear();
}

void ParticleActionManager::AddAction(std::shared_ptr<ParticleAction> action, const std::string& actionName)
{
	m_Actions.emplace(actionName, action);
}

void ParticleActionManager::AddSequence(std::shared_ptr<ParticleSequence> sequence)
{
	m_Sequences.push_back(sequence);
}

void ParticleActionManager::Update()
{
	for (auto& action : m_Actions)
	{
		action.second->Update();
	}

	for (auto& sequence : m_Sequences)
	{
		sequence->Update();
	}

	DeleteSequence();
}

void ParticleActionManager::DeleteSequence()
{
	for (auto itr = m_Sequences.begin(); itr != m_Sequences.end();)
	{
		if ((*itr)->IsDestroy())
		{
			itr = m_Sequences.erase(itr);
			continue;
		}

		++itr;
	}
}

void ParticleActionManager::DeleteAction(const std::string& actionName)
{
	auto result = m_Actions.find(actionName);

	if (result == m_Actions.end())return;

	m_Actions.erase(actionName);
}

ID3D12Resource* ParticleActionManager::GetBuffer(const std::string& actionName)
{
	if (m_Actions.find(actionName) != m_Actions.end())
	{
		return m_Actions.at(actionName)->GetBuffer();
	}

	return nullptr;
}

std::shared_ptr<ParticleAction> ParticleActionManager::GetAction(const std::string& actionName)
{
	if (m_Actions.find(actionName) != m_Actions.end())
	{
		return m_Actions.at(actionName);
	}

	return nullptr;
}
