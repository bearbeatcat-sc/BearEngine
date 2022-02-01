#include "SoundInstance.h"

#include "Utility/Timer.h"
#include "Utility/Math/MathUtility.h"


SoundInstance::SoundInstance(char* pSoundBuffer)
	:AbstractSoundInstance(pSoundBuffer),_currentVolume(0.0f), _targetVolume(0.0f), _previousVolume(0.0f)
{
	_pFadeTimer = std::make_shared<Timer>(1.0f);
}

SoundInstance::~SoundInstance()
{

}

void SoundInstance::Play(float volume,float time)
{
	ChangeState(SoundInstanceState_FadeIn);
	_pSourceVoice->Start();

	_targetVolume = volume;
	_previousVolume = _currentVolume;

	_pFadeTimer->Reset();
	_pFadeTimer->SetTime(time);
}

void SoundInstance::Stop(float time)
{
	ChangeState(SoundInstanceState_FadeOut);

	_targetVolume = 0.0f;
	_previousVolume = _currentVolume;

	_pFadeTimer->Reset();
	_pFadeTimer->SetTime(time);
}

void SoundInstance::ChangeState(SoundInstanceState state)
{
	_soundInstanceState = state;
}

void SoundInstance::Update()
{
	switch (_soundInstanceState)
	{
	case SoundInstanceState::SoundInstanceState_Playing: 
		break;

	case SoundInstanceState::SoundInstanceState_Stop: 
		break;

	case SoundInstanceState::SoundInstanceState_FadeIn:
		FadeIn();
		break;

	case SoundInstanceState::SoundInstanceState_FadeOut:
		FadeOut();
		break;

	default: ;
	}
	
}

void SoundInstance::FadeIn()
{
	_pFadeTimer->Update();
	float ratio = _pFadeTimer->GetRatio();

	_currentVolume = MathUtility::Lerp(_previousVolume, _targetVolume, ratio);

	_pSourceVoice->SetVolume(_currentVolume);

	if(_pFadeTimer->IsTime())
	{
		ChangeState(SoundInstanceState_Playing);
		_pFadeTimer->Reset();
	}
}

void SoundInstance::FadeOut()
{
	_pFadeTimer->Update();
	float ratio = _pFadeTimer->GetRatio();

	_currentVolume = MathUtility::Lerp(_previousVolume, _targetVolume, ratio);

	_pSourceVoice->SetVolume(_currentVolume);

	if (_pFadeTimer->IsTime())
	{
		ChangeState(SoundInstanceState_Stop);
		_pFadeTimer->Reset();
		_pSourceVoice->Stop();
	}
}
