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
	DirectX::SimpleMath::Vector3 GetUserPosition();
	CollisionType GetCollisionType();	
	const std::string& GetCollsionTag();
	void SetRigidFlag(bool flag);
	void SetCollisionTag(const std::string& tag);
	int GetCollisionIndex();


	const DirectX::SimpleMath::Vector3& GetVelocity();
	const DirectX::SimpleMath::Vector3& GetAcc();
	const float& GetMass();

	void SetVelocity(DirectX::SimpleMath::Vector3 vec);
	void SetAcc(DirectX::SimpleMath::Vector3 acc);
	void SetMass(float mass);


	Actor* GetUser();
	void UserOnCollision(Actor* other, CollisionComponent* collisionComponent);
	void Delete();
	bool IsDelete();
	//void OnAction(Actor* other, CollisionComponent* collisionComponent);
	virtual bool IsInterSect(CollisionComponent* collisionComponent) = 0;
	CollisionTreeObject* GetCollisionTreeObject();

private:
	void Rebound(CollisionComponent* collisionComponent);
	void RigidUpdate(CollisionComponent* collisionComponent);

protected:
	CollisionTreeObject* m_TreeObject;
	CollisionType m_CollisionType;
	std::string m_CollisonTag;
	bool m_DeleteFlag;
	int m_CollisionIndex;
	bool m_IsRigid;
	
protected:
	float m_m; // 質量
	DirectX::SimpleMath::Vector3 m_Vel; //移動量
	DirectX::SimpleMath::Vector3 m_Acc; // 加速度
};

#endif