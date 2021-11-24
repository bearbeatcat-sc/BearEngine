#ifndef _COLLISION_TREE_MANAGER_H_
#define _COLLISION_TREE_MANAGER_H_

#include <vector>
#include <list>

#include "CollisionTree_Cell.h"
#include "SimpleMath.h"
#include "AABBCollsionComponent.h"

using namespace DirectX;

class Actor;

class CollisionTreeManager
{
public:
	CollisionTreeManager();
	~CollisionTreeManager();
	bool Init(int level, const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);
	void SetTreeProperties(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);
	bool Regist(CollisionComponent* coll, CollisionTreeObject* obj);
	int GetAllCollisionList(std::vector<CollisionComponent*>& colVector);

private:
	int GetMortonNumber(const SimpleMath::Vector3& minPos, const  SimpleMath::Vector3& maxPos);
	bool CreateNewCell(int elem);
	int BitSeparete3D(int n);
	int Get3DMortonNumber(int x, int y, int z);
	int GetPointElem(const SimpleMath::Vector3& p);
	bool GetCollisionList(int elem, std::vector<CollisionComponent*>& cols, std::list<CollisionComponent*>& colStac);

private:
	SimpleMath::Vector3 m_RangeMin; // 領域の最小値
	SimpleMath::Vector3 m_RangeMax;//　領域の最大

	SimpleMath::Vector3 m_W; // 領域の幅
	SimpleMath::Vector3 m_Unit; //最小Cellの範囲；



	int m_CellCount; // 空間の数;
	std::vector<CollisionTreeCell*> m_Cells;
	int m_MaxLevel = 7;
	int m_uiLevel; // 最下位レベル
	std::vector<int> m_iPow;
};

#endif 