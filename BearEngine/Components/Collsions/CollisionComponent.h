#ifndef _COLLISION_COMPONENT_H_
#define _COLLISION_COMPONENT_H_

#include "../Component.h"
#include "CollisionType.h"
#include <string>
#include "SimpleMath.h"

class CollisionTreeObject;
class RigidBodyComponent;
struct InterSectInfo;

class CollisionComponent
	:public Component
{
public:
	CollisionComponent(Actor* user,CollisionType collType, const std::string& collisonTag, int upadeteOredr = 100);
	virtual ~CollisionComponent();
	
	virtual void Update() override;
	
	void OnDrawDebug();
	void OffDrawDebug();
	//void RegistRigidBody(std::shared_ptr<RigidBodyComponent> rigid_body_component);
	//void OnResolveContact(Actor* other, CollisionComponent* otherCollisionComponent,InterSectInfo& inter_sect_info);
	
	const DirectX::SimpleMath::Vector3& GetUserPosition();

	CollisionType GetCollisionType();
	const std::string& GetCollsionTag();
	int GetCollisionIndex();
	Actor* GetUser();
	//std::shared_ptr<RigidBodyComponent> GetRigidBody();

	void SetCollisionTag(const std::string& tag);
	void SetTreeObject(CollisionTreeObject* treeobj);

	virtual const DirectX::SimpleMath::Vector3 GetMin() = 0;
	virtual const DirectX::SimpleMath::Vector3 GetMax() = 0;

	void UserOnCollision(Actor* other, CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info);
	void Delete();
	bool IsDelete();
	//void OnAction(Actor* other, CollisionComponent* collisionComponent);
	virtual bool IsInterSect(CollisionComponent* collisionComponent, InterSectInfo& inter_sect_info) = 0;
	CollisionTreeObject* GetCollisionTreeObject();


	void DrawProperties() override;
protected:
	bool _isDrawDebug;
	CollisionTreeObject* _treeObject;
	CollisionType _collisionType;
	std::string _collisionTag;
	bool _deleteFlag;
	int _collisionIndex;

	// 今回はとりあえず、ポインタをもたせる。
	//std::shared_ptr<RigidBodyComponent> _rigidBodyComponent;
	
};

#endif