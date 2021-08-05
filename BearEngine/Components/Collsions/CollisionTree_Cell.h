#ifndef _COLLISION_TREE_CELL_H_
#define _COLLISION_TREE_CELL_H_

#include <vector>
#include <memory>

class CollisionTreeObject;

class CollisionTreeCell
{
public:
	CollisionTreeCell();
	~CollisionTreeCell();
	bool Push(CollisionTreeObject* obj);
	bool OnRemove(CollisionTreeObject* obj);
	CollisionTreeObject* GetFirstObject();

private:
	CollisionTreeObject* m_LatestTree; // リストの最新オブジェクトを持つ
};

#endif