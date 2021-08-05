#ifndef _GPU_PARTICLE_SEQUENCE_COMMAND_H_
#define _GPU_PARTICLE_SEQUENCE_COMMAND_H_

#include "ParticleActionManager.h"

namespace ParticleSequcenCommand
{
	struct SequenceCommand
	{

		DirectX::SimpleMath::Vector3 m_PositionRange;
		DirectX::SimpleMath::Vector3 m_VelocityRange;
		DirectX::SimpleMath::Vector3 m_ScaleRange;
		DirectX::SimpleMath::Color m_ColorRange;
		DirectX::SimpleMath::Vector3 m_RotateRange;

		DirectX::SimpleMath::Color m_AddColor;
		DirectX::SimpleMath::Vector3 m_AddRotate;
		DirectX::SimpleMath::Vector3 m_AddVelocity;
		DirectX::SimpleMath::Vector3 m_AddScale;

		float m_DestroyTime = 1.0f;
		float m_ChangeTime = 1.0f;
	};

}



#endif