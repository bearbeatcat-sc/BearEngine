#ifndef _COMPONENT_H_
#define _COMPONENT_H_

class Actor;

class Component
{
public:
	Component(Actor* user, int updateOrder = 100);
	virtual ~Component();
	virtual void Update() = 0;
	virtual void DrawProperties() = 0;

	int GetUpdateOrder() const;

protected:
	Actor* _user;
	int _upderOrder;	
};

#endif