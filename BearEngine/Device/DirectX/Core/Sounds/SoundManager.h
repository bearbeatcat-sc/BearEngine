#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include <xaudio2.h>
#include "../../../Singleton.h"
#include <string>
#include <map>

class XAudio2VoiceCallBack : public IXAudio2VoiceCallback
{
public:
	STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) {};
	STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) {};
	STDMETHOD_(void, OnStreamEnd) (THIS) {};
	STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) {};
	STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext)
	{
		delete[] pBufferContext;
	};
	STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) {};
	STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) {};
};


class SoundManager
	:public Singleton<SoundManager>
{
public:
	friend class Singleton<SoundManager>;
	bool Init();
	void DirectPlay(const std::string& filePath, IXAudio2SourceVoice* pSourceVoice, float volume = 0.5f,bool isLoop = false);
	void DirectPlay(const std::string& filePath, float volume = 0.5f,bool isLoop = false);

	struct Chunk
	{
		char id[4]; // チャンク毎のID
		int32_t size; // チャンクサイズ
	};

	struct RiffHeader
	{
		Chunk chunk; // RIFF
		char type[4];  // WAVE
	};

	struct FormatChunk
	{
		Chunk chunk;
		WAVEFORMAT fmt;
	};


protected:
	SoundManager();
	~SoundManager();
	void Finalize();

	IXAudio2* m_pXAudio;
	IXAudio2MasteringVoice* m_pMasteringVoice;
};


#endif