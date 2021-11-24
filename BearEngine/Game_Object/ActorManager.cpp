#include "ActorManager.h"
#include "Actor.h"

#include <imgui/imgui.h>

ActorManager::ActorManager()
{
}

ActorManager::~ActorManager()
{
	Shutdown();
}

void ActorManager::AddActor()
{
	for (int i = 0; i < m_AddActors.size(); ++i)
	{
		if (m_AddActors[i] == nullptr)continue;

		m_Actos.push_back(m_AddActors[i]);
	}

	m_AddActors.clear();

}

void ActorManager::Init()
{
	m_Actos.clear();
}

void ActorManager::Update()
{
	AddActor();

	for (auto actor : m_Actos)
	{
		actor->Update();
	}

	DeleteActor();

}

void ActorManager::Shutdown()
{
	for (auto itr = m_Actos.begin(); itr != m_Actos.end();)
	{		
		if ((*itr) == nullptr)continue;

		(*itr)->Clean();
		delete (*itr);
		(*itr) = nullptr;
		itr = m_Actos.erase(itr);
	}
}

void ActorManager::AddActor(Actor* actor)
{
	m_AddActors.push_back(actor);
}

//#ifdef _DEBUG
void ActorManager::RenderDebug()
{
	ImGui::Begin("Actors", nullptr);
	ImGui::Text("ActorCount : %i", m_Actos.size());
	ImGui::AlignTextToFramePadding();
	ImGui::BeginChild("Actors");

	static int selected = -1;
	int index = 0;
	
	for(int i = 0; i < m_Actos.size(); ++i)
	{
		m_Actos[i]->RenderDebug(index,selected);
		
	}
	ImGui::EndChild();

	ImGui::End();
}
//#endif

void ActorManager::DeleteActor(Actor* actor)
{
	auto iter = std::find(m_Actos.begin(), m_Actos.end(), actor);

	if (iter != m_Actos.end())
	{
		(*iter)->Clean();
		delete (*iter);
		(*iter) = nullptr;
		m_Actos.erase(iter);
	}

}

void ActorManager::DeleteActor()
{
	for (auto iter = m_Actos.begin(); iter != m_Actos.end();)
	{
		if ((*iter)->GetDestroyFlag())
		{
			(*iter)->Clean();
			delete (*iter);
			(*iter) = nullptr;
			iter = m_Actos.erase(iter);
			continue;
		}

		iter++;
	}
}

std::vector<Actor*>& ActorManager::GetActors()
{
	return m_Actos;
}


