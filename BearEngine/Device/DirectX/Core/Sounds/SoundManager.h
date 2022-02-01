#pragma once

#include "../../../Singleton.h"
#include "AbstractSoundInstance.h"

#include <xaudio2.h>
#include <string>
#include <map>
#include <memory>
#include <vector>


class OneShotSoundInstance;
class SoundInstance;




class SoundManager
	:public Singleton<SoundManager>
{
public:
	friend class Singleton<SoundManager>;
	bool Init();
	void Update();
	void AddInstance(std::shared_ptr<AbstractSoundInstance> instance);

	std::shared_ptr<SoundInstance> CreateSoundInstance(const std::string& filePath,bool isLoop = false);
	void OneShot(const std::string& filePath, float volume = 0.5f);

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


private:
	IXAudio2* m_pXAudio;
	IXAudio2MasteringVoice* m_pMasteringVoice;
	std::vector<std::shared_ptr<AbstractSoundInstance>> _soundInstnaces;

protected:
	SoundManager();
	~SoundManager();
	void Finalize();


};
