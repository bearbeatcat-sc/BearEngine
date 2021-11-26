#ifndef _COLLISION_COMPONENT_H_
#define _COLLISION_COMPONENT_H_

#include "../Component.h"
#include "CollisionType.h"
#include <string>
#include "SimpleMath.h"

class CollisionTreeObject;

class CollisionComponent
	:public Component
{
public:
	CollisionComponent(Actor* user,CollisionType collType, const std::string& collisonTag, int upadeteOredr = 100);
	virtual ~CollisionComponent();
	
	virtual void Update() override;
	
	void OnDrawDebug();
	void OffDrawDebug();

	const DirectX::SimpleMath::Vector3& GetUserPosition();

	CollisionType GetCollisionType();
	const std::string& GetCollsionTag();
	int GetCollisionIndex();
	Actor* GetUser();

	void SetCollisionTag(const std::string& tag);
	void SetTreeObject(CollisionTreeObject* treeobj);

	virtual const DirectX::SimpleMath::Vector3 GetMin() = 0;
	virtual const DirectX::SimpleMath::Vector3 GetMax() = 0;

	void UserOnCollision(Actor* other, CollisionComponent* collisionComponent);
	void Delete();
	bool IsDelete();
	//void OnAction(Actor* other, CollisionComponent* collisionComponent);
	virtual bool IsInterSect(CollisionComponent* collisionComponent) = 0;
	CollisionTreeObject* GetCollisionTreeObject();


protected:
	bool _isDrawDebug;
	CollisionTreeObject* _treeObject;
	CollisionType _collisionType;
	std::string _collisionTag;
	bool _deleteFlag;
	int _collisionIndex;
	
};

#endif