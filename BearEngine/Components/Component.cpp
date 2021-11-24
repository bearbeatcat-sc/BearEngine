#include "Component.h"
#include "../Game_Object/Actor.h"

Component::Component(Actor* user, int updateOrder)
	:_user(user), _upderOrder(updateOrder)
{
	//m_User->AddComponent(this);
}

Component::~Component()
{
	//m_User->RemoveComponent(this);
}

int Component::GetUpdateOrder() const
{
	return _upderOrder;
}
