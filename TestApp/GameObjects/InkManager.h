#ifndef _INK_MANAGER_H_
#define _INK_MANAGER_H_

#include <Game_Object/Actor.h>
#include <SimpleMath.h>
#include <memory>
#include <Device/ParticleSystems/ParticleEmitter.h>

using namespace DirectX;

class ParticleEmitter;

class InkManager
	:public Actor
{
public:
	InkManager(int maxParticleCount);
	~InkManager();

	void PushWallData(ParticleEmitter::WallPalam wallParam);
	void AddParticle(int inkParticleCount);
	void GenerateEmitter();

private:
	void UpdateActor() override;
	void Init() override;
	void Shutdown() override;
	void OnCollsion(Actor* other) override;

private:
	std::shared_ptr<ParticleEmitter> mParticleEmitter;
	std::shared_ptr<FluidParticleAction> m_action;

	std::vector<ParticleEmitter::WallPalam> m_WallParameters;

	bool m_isPushWallParameters;
	
	int mParticleMaxCount;
	int mCurrentParticleCount;
};

#endif