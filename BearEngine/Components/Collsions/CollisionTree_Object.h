#ifndef _COLLISION_TREE_OBJECT_H_
#define _COLLISION_TREE_OBJECT_H_

#include "AABBCollsionComponent.h"
#include <memory>

class CollisionTreeCell;

class CollisionTreeObject
{
public:
	CollisionTreeObject(CollisionComponent* coll);
	~CollisionTreeObject();
	bool Remove();
	void RegisterCell(CollisionTreeCell* cell);

public:
	CollisionComponent* m_UserCol;

	CollisionTreeObject* m_Previous;
	CollisionTreeObject* m_Next;
	
	CollisionTreeCell* m_CollTreeCell; // 登録されている空間
};

#endif