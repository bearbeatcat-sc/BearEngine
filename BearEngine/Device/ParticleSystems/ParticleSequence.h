#ifndef _GPU_PARTICLE_SEQUENCE_H_
#define _GPU_PARTICLE_SEQUENCE_H_

#include "ParticleManager.h"
#include "ParticleSequenceCommand.h"
#include "NormalParticleAction.h"

#include <vector>

class Timer;

class ParticleSequence
{
public:
	ParticleSequence(const std::string& sequenceName,bool loopFlag = false,bool randomFlag = false, const std::string& particleUpdateShaderName = "ParticleUpdateComputeShader", const std::string& particleInitShaderName = "ParticleInitComputeShader");
	~ParticleSequence();
	void AddCommand(std::shared_ptr<ParticleSequcenCommand::SequenceCommand> command);
	void Update();
	void ShutDown();
	void ChangeAction();
	void Destroy();
	bool IsDestroy();

private:
	std::vector<std::shared_ptr<ParticleSequcenCommand::SequenceCommand>> m_Command;
	Timer* m_pChangeTimer;
	int m_Index;
	std::shared_ptr<NormalParticleAction> m_Action;
	std::string m_SequenceName;
	bool m_DeathFlag;
	bool m_IsLoop;
	bool m_IsRandom;
};

#endif