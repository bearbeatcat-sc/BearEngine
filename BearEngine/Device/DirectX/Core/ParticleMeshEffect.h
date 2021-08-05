#ifndef _PARTICLE_MESH_EFFECT_H_
#define _PARTICLE_MESH_EFFECT_H_

#include "Effect.h"

class ParticleMeshEffect
	:public Effect
{
public:
	ParticleMeshEffect();
	~ParticleMeshEffect();
	virtual bool Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName) override;

};

#endif