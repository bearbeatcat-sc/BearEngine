#ifndef _PLAYER_H_
#define _PLAYER_H_


#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>

using namespace DirectX;

class InkManager;
class Timer;

class Player
	:public Actor
{
public:
	Player(InkManager* pInkManager);
	~Player();

private:
	virtual void UpdateActor() override;
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnCollsion(Actor* other) override;

	void Move();
	void Throw();
	void ThrowVecControl();
	bool IsThrow();

private:
	SimpleMath::Vector3 mThrowVec;
	SimpleMath::Vector3 mCenterPosition;

	std::shared_ptr<Timer> m_pTimer;

	InkManager* m_pInkManager;
};
#endif