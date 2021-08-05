#include "EffectManager.h"

bool EffectManager::Init()
{
    m_Effects.clear();

    return true;
}

bool EffectManager::AddEffect(std::shared_ptr<Effect> effect,std::string effectName)
{
    if (m_Effects.find(effectName) != m_Effects.end()) return false;

    m_Effects.emplace(effectName, effect);
    return true;
}

std::shared_ptr<Effect> EffectManager::GetEffect(std::string effectName)
{
    if (m_Effects.find(effectName) == m_Effects.end()) return nullptr;

    return m_Effects.at(effectName);
}

EffectManager::EffectManager()
{
    m_Effects.clear();
}

EffectManager::~EffectManager()
{
    m_Effects.clear();
}
