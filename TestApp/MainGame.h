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

	int m_partcile_count;
	float m_particle_size;
	DirectX::SimpleMath::Vector3 m_generate_position;
	DirectX::SimpleMath::Vector3 m_add_velocity;
	DirectX::SimpleMath::Vector3 m_generate_range;
	DirectX::SimpleMath::Vector3 m_velocity;

	DirectX::SimpleMath::Vector3 mSpeherePosition;
	Sphere* mSpehere;

	const int total_particle_count = 20000;
	int mCurrentParticleCount;

	std::shared_ptr<ParticleEmitter> m_GPUParticleEmitter;	
	std::list<std::shared_ptr<ParticleEmitter>> m_particles;


	std::shared_ptr<Timer> mParticleTimer;
	const float mParticleTime = 0.01f;
};