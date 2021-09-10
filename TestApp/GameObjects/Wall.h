#ifndef _WALL_H_
#define _WALL_H_

#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

using namespace DirectX;

class AABBCollisionComponent;
class InkManager;

class Wall
	:public Actor
{
public:
	Wall(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale);
	~Wall();

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

private:
	AABBCollisionComponent* m_pCollisionComponent;
};

#endif