#pragma once

#include <xaudio2.h>

class XAudio2VoiceCallBack : public IXAudio2VoiceCallback
{
public:
	STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) {};
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) {};
	STDMETHOD_(void, OnStreamEnd) (THIS) {};
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) {};
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) {};
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) {};
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) {};
};


class AbstractSoundInstance
{
public:

	AbstractSoundInstance(char* pSoundBuffer)
		:_destroyFlag(false), _pSoundBuffer(pSoundBuffer)
	{

	}

	virtual ~AbstractSoundInstance()
	{
		if (_pSourceVoice != nullptr)
		{
			_pSourceVoice->Stop();
			_pSourceVoice->DestroyVoice();
			_pSourceVoice = nullptr;
		}

		delete[] _pSoundBuffer;
		_pSoundBuffer = nullptr;
	}

	void CreateSoundVoice(IXAudio2* pDevice, XAUDIO2_BUFFER& pBuffer, WAVEFORMATEX& wfx)
	{
		auto result = pDevice->CreateSourceVoice(&_pSourceVoice, &wfx, 0, 2.0f,
			&_pVoiceCallBack);

		if (FAILED(result))
		{
			delete[] _pSoundBuffer;
			return;
		}

		result = _pSourceVoice->SubmitSourceBuffer(&pBuffer);
	}


	virtual void Update() = 0;

	void Destroy()
	{
		_destroyFlag = true;
	}

	bool IsDestroy() const
	{
		return _destroyFlag;
	}

protected:
	IXAudio2SourceVoice* _pSourceVoice;
	XAudio2VoiceCallBack _pVoiceCallBack;

	char* _pSoundBuffer;
	bool _destroyFlag;
};