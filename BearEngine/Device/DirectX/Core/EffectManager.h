#ifndef _EFFECT_MANAGER_H_
#define _EFFECT_MANAGER_H_

#include "../../Singleton.h"
#include "Effect.h"
#include <map>

#include <memory>

class EffectManager
	:public Singleton<EffectManager>
{
public:
	friend class Singleton<EffectManager>;
	bool Init();
	bool AddEffect(std::shared_ptr<Effect> effect, std::string effectName);
	std::shared_ptr<Effect> GetEffect(std::string effectName);

protected:
	EffectManager();
	~EffectManager();

private:
	std::map<std::string, std::shared_ptr<Effect>> m_Effects;
};

#endif
