#include "CollisionTagManager.h"

void CollisionTagManagaer::AddTag(std::string tagName,std::vector<bool> collTable)
{
    for (int i = 0; i < m_Tags.size(); i++)
    {
        if (m_Tags[i] == tagName)
        {
            return;
        }
    }

    m_Tags.push_back(tagName);
    m_CollisionTable.push_back(collTable);
}

std::vector<std::vector<bool>> CollisionTagManagaer::GetCollisionTable()
{
    return m_CollisionTable;
}

int CollisionTagManagaer::GetTagIndex(std::string tagName)
{

    for (int i = 0; i < m_Tags.size(); i++)
    {
        if (m_Tags[i] == tagName)
        {
            return i;
        }
    }

    return -1;
}

CollisionTagManagaer::CollisionTagManagaer()
{
}

CollisionTagManagaer::~CollisionTagManagaer()
{
    m_CollisionTable.clear();
}

