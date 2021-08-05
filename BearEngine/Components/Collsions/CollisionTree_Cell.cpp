#include "CollisionTree_Cell.h"
#include "CollisionTree_Object.h"

CollisionTreeCell::CollisionTreeCell()
{
}

CollisionTreeCell::~CollisionTreeCell()
{
}

bool CollisionTreeCell::Push(CollisionTreeObject* obj)
{
	if (obj == nullptr)return false;

	if (obj->m_CollTreeCell == this)
	{
		return false; // 二重登録防止
	}
	
	obj->Remove();

	// 空間に何もないので、そのまま登録
	if (m_LatestTree == nullptr)
	{
		m_LatestTree = obj;
	}
	else
	{
		if (obj == m_LatestTree)
		{
			return false;
		}

		// スタックに登録
		obj->m_Next = m_LatestTree;
		m_LatestTree->m_Previous = obj;

		// スタック、つまり一番上のオブジェクトとして更新
		m_LatestTree = obj;
	}

	// 自分をオブジェクトの配属空間として登録
	obj->RegisterCell(this);

	return true;

}

bool CollisionTreeCell::OnRemove(CollisionTreeObject* obj)
{
	if (m_LatestTree == obj)
	{
		m_LatestTree = m_LatestTree->m_Next;
	}

	return true;
}

CollisionTreeObject* CollisionTreeCell::GetFirstObject()
{
	return m_LatestTree;
}

