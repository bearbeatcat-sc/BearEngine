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
		return false; // ��d�o�^�h�~
	}
	
	obj->Remove();

	// ��Ԃɉ����Ȃ��̂ŁA���̂܂ܓo�^
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

		// �X�^�b�N�ɓo�^
		obj->m_Next = m_LatestTree;
		m_LatestTree->m_Previous = obj;

		// �X�^�b�N�A�܂��ԏ�̃I�u�W�F�N�g�Ƃ��čX�V
		m_LatestTree = obj;
	}

	// �������I�u�W�F�N�g�̔z����ԂƂ��ēo�^
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

