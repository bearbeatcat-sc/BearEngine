#include "CollisionTree_Manager.h"
#include "CollisionTree_Object.h"
#include "CollisionTagManager.h"

CollisionTreeManager::CollisionTreeManager()
{
	m_iPow.resize(m_MaxLevel + 2);
}

CollisionTreeManager::~CollisionTreeManager()
{
	for (auto itr = m_Cells.begin(); itr != m_Cells.end(); itr++)
	{
		delete (*itr);
	}

	m_Cells.clear();
	m_Cells.shrink_to_fit();

	m_iPow.clear();
	m_iPow.shrink_to_fit();
}

bool CollisionTreeManager::Init(int level,const SimpleMath::Vector3& min,const SimpleMath::Vector3& max)
{
	// �ő僌�x���ȏ�͐����ł��Ȃ�
	if (level >= m_MaxLevel) { return false; }


	m_iPow[0] = 1;
	// �e���x���ł̋�Ԑ����Z�o����
	for (int i = 1; i <= m_MaxLevel + 1; i++)
	{
		// �ׂ���ŎZ�o���Ă���
		m_iPow[i] = m_iPow[i - 1] * 8;
	}

	// �z��̍쐬
	// ��Ԑ����Z�o
	m_CellCount = (m_iPow[level + 1] - 1) / 7;
	m_Cells.resize(m_CellCount);


	// �̈�
	m_RangeMax = max;
	m_RangeMin = min;
	m_W = m_RangeMax - m_RangeMin;

	int unit = 1 << level;
	m_Unit = m_W / unit;


	m_uiLevel = level;

	return true;

}

void CollisionTreeManager::SetTreeProperties(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max)
{
	// �̈�
	m_RangeMax = max;
	m_RangeMin = min;
	m_W = m_RangeMax - m_RangeMin;

	// ���ɐݒ肵�Ă��郌�x������ύX���Ȃ�
	int unit = 1 << m_uiLevel;
	m_Unit = m_W / unit;


}

bool CollisionTreeManager::Regist(AABBCollisionComponent* coll, CollisionTreeObject* obj)
{
	int elem = GetMortonNumber(coll->GetMin(), coll->GetMax());

	if (elem == -1)return false;
	if (elem < m_CellCount)
	{
		if (!m_Cells[elem])
			CreateNewCell(elem);

		return m_Cells[elem]->Push(obj);
	}

	return false;
}

int CollisionTreeManager::GetAllCollisionList(std::vector<AABBCollisionComponent*>& colVector)
{
	colVector.clear();

	if (m_Cells[0] == nullptr)
		return 0;

	std::list<AABBCollisionComponent*> colStac;
	GetCollisionList(0, colVector, colStac);

	return colVector.size();
}



int CollisionTreeManager::GetMortonNumber(const SimpleMath::Vector3& minPos,const SimpleMath::Vector3& maxPos)
{

	// 2�̒��_�̈ʒu���Z�o
	int lt = GetPointElem(minPos);
	int rb = GetPointElem(maxPos);

	int def = rb ^ lt;
	int hiLevel = 1;

	//�������Ă����Ԃ̃��x�����Z�o
	for (int i = 0; i < m_uiLevel; i++)
	{
		int check = (def >> (i * 3)) & 0x7;

		if (check != 0)
			hiLevel = i + 1;
	}

	int spaceNum = rb >> (hiLevel * 3);
	int addNum = (m_iPow[m_uiLevel - hiLevel] - 1) / 7;
	spaceNum += addNum;

	if (spaceNum > m_CellCount)
		return 0xffffffff;

	return spaceNum;
}

bool CollisionTreeManager::CreateNewCell(int elem)
{
	// �w�肳�ꂽ��Ԃ����݂���Ȃ��Ȃ�܂ō쐬����
	while (!m_Cells[elem])
	{
		m_Cells[elem] = new CollisionTreeCell();

		elem = (elem - 1) >> 3;

		if (elem >= m_CellCount) break;
		if (elem < 0)break;
	}

	return true;
}

int CollisionTreeManager::BitSeparete3D(int n)
{
	int s = n;
	s = (s | s << 8) & 0x0000f00f;
	s = (s | s << 4) & 0x000c30c3;
	s = (s | s << 2) & 0x00249249;

	return s;
}

int CollisionTreeManager::Get3DMortonNumber(int x, int y, int z)
{
	return BitSeparete3D(x) | (BitSeparete3D(y) << 1) | (BitSeparete3D(z) << 2);
}

int CollisionTreeManager::GetPointElem(const SimpleMath::Vector3& p)
{
	int x = (int)((p.x - m_RangeMin.x) / m_Unit.x);
	int y = (int)((p.y - m_RangeMin.y) / m_Unit.y);
	int z = (int)((p.z - m_RangeMin.z) / m_Unit.z);

	//int x = (int)(p.x / m_Unit.x);
	//int y = (int)(p.y / m_Unit.y);
	//int z = (int)(p.z / m_Unit.z);

	return Get3DMortonNumber(x, y, z);
}

bool CollisionTreeManager::GetCollisionList(int elem, std::vector<AABBCollisionComponent*>& cols, std::list<AABBCollisionComponent*>& colStac)
{
	std::list<AABBCollisionComponent*>::iterator it;

	CollisionTreeObject* to = m_Cells[elem]->GetFirstObject();


	while (to != nullptr)
	{
		CollisionTreeObject* to2 = to->m_Next;

		// ������ԏ�̃I�u�W�F�N�g�Ƃ̏Փ˔�������X�g�ɒǉ����Ă���
		while (to2 != nullptr)
		{
			//if (!collisionTable[to->m_UserCol->GetCollisionIndex()][to2->m_UserCol->GetCollisionIndex()])
			//{
			//	to2 = to2->m_Next;
			//	continue;
			//}

			// ������I�u�W�F�N�g���m����`�ɓo�^���Ă���
			cols.emplace_back(to->m_UserCol);
			cols.emplace_back(to2->m_UserCol);

			to2 = to2->m_Next;
		}

		// �X�^�b�N���m�Ƃ̏Փ˔�������X�g�ɒǉ����Ă���
		for (it = colStac.begin(); it != colStac.end(); ++it)
		{
			//if (!collisionTable[to->m_UserCol->GetCollisionIndex()][(*it)->GetCollisionIndex()])
			//{
			//	continue;
			//}

			cols.emplace_back(to->m_UserCol);
			cols.emplace_back(*it);
		}

		to = to->m_Next;
	}

	bool childFlag = false;
	int objNum = 0;
	int nextElem;

	for (int i = 0; i < 8; i++)
	{
		// �ŏ��̃��[�g��ԕ��A1�𑫂��Ă�
		nextElem = elem * 8 + 1 + i;

		if (nextElem < m_CellCount && m_Cells[elem * 8 + 1 + i])
		{
			if (!childFlag)
			{
				// �o�^����Ă���I�u�W�F�N�g���X�^�b�N�ɒǉ�
				to = m_Cells[elem]->GetFirstObject();
				while (to)
				{
					colStac.emplace_back(to->m_UserCol);
					objNum++;
					to = to->m_Next;
				}
			}
			childFlag = true;

			// ����ԂɈړ�
			GetCollisionList(elem * 8 + 1 + i, cols, colStac);
		}
	}

	if (childFlag)
	{
		for (int i = 0; i < objNum; ++i)
		{
			colStac.pop_back();
		}
	}

	return true;
}
