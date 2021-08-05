#ifndef _MESH_NORMAL_EFFECT_H_
#define _MESH_NORMAL_EFFECT_H_

#include "Effect.h"

class MeshEffect
	:public Effect
{
public:
	MeshEffect();
	~MeshEffect();

	virtual bool Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName) override;

};

#endif