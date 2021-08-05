#ifndef _INK_BALL_H_
#define _INK_BALL_H_

#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

using namespace DirectX::SimpleMath;

class MeshComponent;
class AABBCollisionComponent;
class InkManager;

class InkBall
	:public Actor
{
public:
	InkBall(const Vector3& pos, const Vector3& acc, const Vector3& vel, InkManager* pInkManager);
	~InkBall();

private:
	void Move();
	
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

private:
	InkManager* m_pInkManager;
	
	std::shared_ptr<MeshComponent> m_pMeshComponent;
	AABBCollisionComponent* m_pCollisionComponent;

	Vector3 m_Velocity;
	Vector3 m_Acc;
};

#endif