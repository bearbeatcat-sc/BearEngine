#pragma once
#include "Game.h"
#include <SimpleMath.h>
#include <memory>
#include <list>


class Camera;
class Actor;
class CameraAsistant;
class FluidParticleAction;
class Sphere;
class Timer;
class ParticleEmitter;

class CollisionTreeManager;
class InkManager;

class MainGame
	:public Game
{
public:
	MainGame();
	~MainGame();

	virtual void Init() override;
	virtual void Update() override;
	virtual void Destroy() override;

private:
	CameraAsistant* m_CameraAsistant;
	// DEBUG
	std::shared_ptr<Timer> _AddTimer;
	std::shared_ptr<Timer> _GenerateTimer;
	const int _MaxGenerateCount = 24;
	int _GenerateCount;

	bool _IsGenerate;
};