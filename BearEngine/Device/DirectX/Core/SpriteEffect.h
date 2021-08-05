#ifndef _SPRITE_EFFECT_H_
#define _SPRITE_EFFECT_H_

#include "Effect.h"

class SpriteEffect
	:public Effect
{
public:
	SpriteEffect();
	~SpriteEffect();
	virtual bool Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName) override;
};

#endif