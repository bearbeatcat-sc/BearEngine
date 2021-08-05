#include "CollisionManager.h"
#include "CollisionComponent.h"
#include "./CollisionManager.h"
#include "./CollisionTagManager.h"
#include "CollisionTree_Object.h"
#include <chrono>
#include "../../imgui/imgui.h"
#include "../../Utility/Time.h"
#include "../../Game_Object/Actor.h"
#include "../../Utility/LogSystem.h"
#include <string>

CollisionManager::CollisionManager()
	:m_isDebugMode(true)
{
}

CollisionManager::~CollisionManager()
{
	Reset();
	delete m_CollisionTreeManager;
}

void CollisionManager::AddComponent(CollisionComponent* component)
{
	m_components.push_back(component);
}

void CollisionManager::AddRegistTree(AABBCollisionComponent* component)
{
	CollisionTreeObject* m_Obj = new CollisionTreeObject(component);

	auto result = m_CollisionTreeManager->Regist(component, m_Obj);

	// モートン番号が取得できなかった場合、エラーログを残す
	if (!result)
	{
		auto point = component->GetUserPosition();
		std::string message = "Point X:" + std::to_string(point.x) + " Y:" + std::to_string(point.y) + " Z:" + std::to_string(point.z);
		LogSystem::AddLog("CollisionError : Add Failed. " + message);
		delete m_Obj;
		return;
	}

	component->SetTreeObject(m_Obj);
}

void CollisionManager::UpdateRegistTree(AABBCollisionComponent* component, CollisionTreeObject* object)
{
	if (object == nullptr)
	{
		AddRegistTree(component);
		return;
	}

	auto result = m_CollisionTreeManager->Regist(object->m_UserCol, object);
}

void CollisionManager::Update()
{

	int componentSize = m_components.size();

	for (size_t i = 0; i < componentSize; ++i)
	{
		//if (m_components[i]->IsDelete()) { continue; }

		m_components[i]->Update();
		UpdateRegistTree(static_cast<AABBCollisionComponent*>(m_components[i]), m_components[i]->GetCollisionTreeObject());
	}

	std::chrono::system_clock::time_point start, end;

	start = std::chrono::system_clock::now();

	InterSect();

	end = std::chrono::system_clock::now();

	m_CurrentCollisitonTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);

	Delete();

}

void CollisionManager::SetDebugMode(bool flag)
{
	m_isDebugMode = flag;
}

void CollisionManager::InterSect()
{
	auto collisionTable = CollisionTagManagaer::GetInstance().GetCollisionTable();
	std::vector<AABBCollisionComponent*> collisonList;

	// 当たり判定のツリーからリストを取得
	int count = m_CollisionTreeManager->GetAllCollisionList(collisonList);



	for (int i = 0; i < count - 1; i += 2)
	{
		if (collisonList[i]->IsDelete() || collisonList[i + 1]->IsDelete()) continue;
		// 当たり判定のテーブルによって、無駄な処理をスキップ
		if (!collisionTable[collisonList[i]->GetCollisionIndex()][collisonList[i + 1]->GetCollisionIndex()]) continue;


		if (collisonList[i]->IsInterSect(collisonList[i + 1]))
		{
			collisonList[i]->UserOnCollision(collisonList[i + 1]->GetUser(), collisonList[i + 1]);
			collisonList[i + 1]->UserOnCollision(collisonList[i]->GetUser(), collisonList[i]);
		}
	}




}

void CollisionManager::Delete()
{
	for (auto iter = m_components.begin(); iter != m_components.end();)
	{
		if ((*iter)->IsDelete())
		{
			delete (*iter);
			iter = m_components.erase(iter);
			continue;
		}

		++iter;
	}
}

void CollisionManager::Delete(AABBCollisionComponent* component)
{
	auto result = std::find(m_components.begin(), m_components.end(), component);

	if (result == m_components.end())
	{
		return;
	}

	m_components.erase(result);
	delete (*result);
}



void CollisionManager::Reset()
{
	for (auto iter = m_components.begin(); iter != m_components.end(); ++iter)
	{
		delete (*iter);
	}

	m_components.clear();
}

void CollisionManager::Draw()
{

	if (m_isDebugMode)
	{
		ImGui::Text("Application Properties");
		ImGui::Text("FPS:%i", Time::FPS);
		ImGui::Text("DeltaTime:%f", Time::DeltaTime);

		ImGui::Separator();

		ImGui::Text("CollisionTime:%f", time);
		ImGui::Text("ObjectCount:%i", m_components.size());
	}

}

void CollisionManager::Init(int level, const SimpleMath::Vector3& min, const SimpleMath::Vector3& max)
{
	m_CollisionTreeManager = new CollisionTreeManager();
	m_CollisionTreeManager->Init(level, min, max);
}

void CollisionManager::SetTreeProperties(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max)
{
	m_CollisionTreeManager->SetTreeProperties(min, max);
}

