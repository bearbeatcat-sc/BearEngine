#pragma once

#include "AbstractSoundInstance.h"

class IXAudio2SourceVoice;

class OneShotSoundInstance
	:public AbstractSoundInstance
{
public:
	OneShotSoundInstance(char* pSoundBuffer);
	~OneShotSoundInstance();

	void Play(float volume, float time);
	void Update() override;

private:
	void Playing();


};
