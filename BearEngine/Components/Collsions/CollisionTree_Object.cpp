#include "CollisionTree_Object.h"
#include "CollisionTree_Cell.h"

CollisionTreeObject::CollisionTreeObject(AABBCollisionComponent* coll)
	:m_UserCol(coll)
{
	//m_UserCol->SetTreeObject(this);
}

CollisionTreeObject::~CollisionTreeObject()
{
	Remove();
	m_CollTreeCell = 0;
	m_UserCol = 0;
}

bool CollisionTreeObject::Remove()
{
	if (!m_CollTreeCell)
	{
		return false;
	}

	if (!m_CollTreeCell->OnRemove(this))
	{
		return false;
	}

	// ‘OŒã‚ðŒq‚°‚é
	if (m_Previous != nullptr)
	{
		m_Previous->m_Next = m_Next;
	}
	if (m_Next != nullptr)
	{
		m_Next->m_Previous = m_Previous;
	}

	m_Previous = nullptr;
	m_Next = nullptr;
	m_CollTreeCell = nullptr;
	return true;
}

void CollisionTreeObject::RegisterCell(CollisionTreeCell* cell)
{
	m_CollTreeCell = cell;
}

