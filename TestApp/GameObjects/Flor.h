#ifndef _FLOR_H_
#define _FLOR_H_


#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

using namespace DirectX;

class AABBCollisionComponent;
class InkManager;

class Flor
	:public Actor
{
public:
	Flor(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale, InkManager* pInkManager);
	~Flor();

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

private:
	AABBCollisionComponent* m_pCollisionComponent;
	InkManager* m_pInkManager;
};

#endif