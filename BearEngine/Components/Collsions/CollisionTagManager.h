#ifndef _COLLISION_TAG_MANAGER_
#define _COLLISION_TAG_MANAGER_

#include "../../Device/Singleton.h"
#include <string>
#include <vector>

class CollisionTagManagaer
	:public Singleton<CollisionTagManagaer>
{
public:
	friend class Singleton<CollisionTagManagaer>;
	void AddTag(std::string tagName,std::vector<bool> collTable);
	std::vector<std::vector<bool>> GetCollisionTable();
	int GetTagIndex(std::string tagName);

protected:
	CollisionTagManagaer();
	~CollisionTagManagaer();

private:
	std::vector<std::string> m_Tags;
	std::vector<std::vector<bool>> m_CollisionTable;
};

#endif