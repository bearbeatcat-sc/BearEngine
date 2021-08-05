#include "ParticleSequence.h"

#include "../../Utility/Timer.h"
#include "../../Utility/Random.h"

#include <assert.h>

ParticleSequence::ParticleSequence(const std::string& sequenceName, bool loopFlag, bool randomFlag, const std::string& particleUpdateShaderName, const std::string& particleInitShaderName)
	:m_SequenceName(sequenceName), m_Index(-1), m_DeathFlag(false),m_IsLoop(loopFlag),m_IsRandom(randomFlag)
{
	m_Action = std::make_shared<NormalParticleAction>(particleUpdateShaderName, particleInitShaderName);
	ParticleManager::GetInstance().AddAction(m_Action, sequenceName);

	m_pChangeTimer = new Timer(0.0f);
}

ParticleSequence::~ParticleSequence()
{
	delete m_pChangeTimer;
}

void ParticleSequence::AddCommand(std::shared_ptr<ParticleSequcenCommand::SequenceCommand> command)
{
	m_Command.push_back(command);
}

void ParticleSequence::Update()
{
	m_pChangeTimer->Update();
	if (m_pChangeTimer->IsTime())
	{
		ChangeAction();
	}
}

void ParticleSequence::ShutDown()
{
	m_Command.clear();
}

void ParticleSequence::ChangeAction()
{
	assert(m_Command.size() != 0);

	// これ以上チェンジできない
	if(m_Index != -1 && m_Index >= m_Command.size() - 1)
	{
		// ループするなら初期化
		if (m_IsLoop)
		{
			m_Index = -1;
		}
		else
		{
			Destroy();
			return;
		}
	}

	++m_Index;

	float changeTime = m_Command[m_Index]->m_ChangeTime;
	m_pChangeTimer->Reset();
	m_pChangeTimer->SetTime(changeTime);

	auto command = m_Command[m_Index];


	if (m_IsRandom)
	{
		Random rand;
		float randomParameter = rand.GetRandom(-2.0f, 2.0f);

		// アクションを書き換える
		m_Action->SetPositionRange(command->m_PositionRange * randomParameter);
		m_Action->SetRotateRange(command->m_RotateRange * randomParameter);
		m_Action->SetScaleRange(command->m_ScaleRange * randomParameter);
		m_Action->SetVelocityRange(command->m_VelocityRange * randomParameter);
		m_Action->SetColorRange(command->m_ColorRange * randomParameter);

		m_Action->SetAddColor(command->m_AddColor * randomParameter);
		m_Action->SetAddVelocity(command->m_AddVelocity * randomParameter);
		m_Action->SetAddRotate(command->m_AddRotate * randomParameter);
		m_Action->SetAddScale(command->m_AddScale * randomParameter);

		m_Action->SetDeathTime(command->m_DestroyTime * randomParameter);
		return;
	}

	// アクションを書き換える
	m_Action->SetPositionRange(command->m_PositionRange);
	m_Action->SetRotateRange(command->m_RotateRange);
	m_Action->SetScaleRange(command->m_ScaleRange );
	m_Action->SetVelocityRange(command->m_VelocityRange );
	m_Action->SetColorRange(command->m_ColorRange );

	m_Action->SetAddColor(command->m_AddColor);
	m_Action->SetAddVelocity(command->m_AddVelocity);
	m_Action->SetAddRotate(command->m_AddRotate);
	m_Action->SetAddScale(command->m_AddScale);

	m_Action->SetDeathTime(command->m_DestroyTime);

}

void ParticleSequence::Destroy()
{
	m_DeathFlag = true;
}

bool ParticleSequence::IsDestroy()
{
	return m_DeathFlag;
}
