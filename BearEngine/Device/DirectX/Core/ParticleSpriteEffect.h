#ifndef _PARTICLE_SPRITE_EFFECT_H_
#define _PARTICLE_SPRITE_EFFECT_H_

#include "Effect.h"

class ParticleSpriteEffect
	:public Effect
{
public:
	ParticleSpriteEffect();
	~ParticleSpriteEffect();
	virtual bool Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName) override;
};


#endif