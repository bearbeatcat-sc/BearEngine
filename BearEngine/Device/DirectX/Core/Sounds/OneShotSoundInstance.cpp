#include "OneShotSoundInstance.h"

#include "Utility/Timer.h"
#include "Utility/Math/MathUtility.h"

#include <xaudio2.h>

OneShotSoundInstance::OneShotSoundInstance( char* pSoundBuffer)
	:AbstractSoundInstance(pSoundBuffer)
{
}

OneShotSoundInstance::~OneShotSoundInstance()
{

}

void OneShotSoundInstance::Play(float volume, float time)
{
	_pSourceVoice->Start();
	_pSourceVoice->SetVolume(volume);
}

void OneShotSoundInstance::Update()
{
	Playing();
}

void OneShotSoundInstance::Playing()
{
	XAUDIO2_VOICE_STATE state;
	_pSourceVoice->GetState(&state);

	if (state.BuffersQueued <= 0)
	{
		_pSourceVoice->Stop();
		Destroy();
	}
}