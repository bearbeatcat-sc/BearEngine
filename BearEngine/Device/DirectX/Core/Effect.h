#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "../../Singleton.h"
#include "../Core/PSOManager.h"

#include <string>

class Effect
{
public:
	Effect();
	~Effect();
	virtual bool Init(const std::string& vertexShaderName, const std::string& pixelShaderName, const std::string& geomtryShaderName) = 0;
	PSO* GetPSO();

protected:
	PSO m_PSO;
};

#endif