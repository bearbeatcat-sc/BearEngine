#ifndef _FLOR_H_
#define _FLOR_H_


#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

using namespace DirectX;

class CollisionComponent;
class RigidBodyComponent;

class Flor
	:public Actor
{
public:
	Flor(const SimpleMath::Vector3& pos, const SimpleMath::Vector3& scale);
	~Flor();

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;


};

#endif