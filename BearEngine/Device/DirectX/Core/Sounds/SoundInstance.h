#pragma once

#include <memory>

#include "AbstractSoundInstance.h"

class Timer;

class SoundInstance
	:public AbstractSoundInstance
{
public:
	SoundInstance(char* pSoundBuffer);
	 ~SoundInstance();

	enum SoundInstanceState
	{
		SoundInstanceState_Playing,
		SoundInstanceState_Stop,
		SoundInstanceState_FadeIn,
		SoundInstanceState_FadeOut
	};

	void Play(float volume,float time);
	void Stop(float time);
	void ChangeState(SoundInstanceState state);
	virtual void Update() override;

private:

	void FadeIn();
	void FadeOut();

protected:
	SoundInstanceState _soundInstanceState;

	std::shared_ptr<Timer> _pFadeTimer;

	float _currentVolume;
	float _previousVolume;
	float _targetVolume;
};
