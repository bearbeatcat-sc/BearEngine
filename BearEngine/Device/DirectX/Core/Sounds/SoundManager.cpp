#include "SoundManager.h"
#include <fstream>
#include <cassert>

#pragma comment(lib,"xaudio2.lib")

XAudio2VoiceCallBack voiceCallBack;

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
	Finalize();
}

void SoundManager::Finalize()
{
	if (m_pMasteringVoice)
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
	}

	if (m_pXAudio)
	{
		m_pXAudio->Release();
		m_pXAudio = nullptr;
	}

	CoUninitialize();
}

bool SoundManager::Init()
{
	if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
	{
		return false;
	}

	if (FAILED(XAudio2Create(&m_pXAudio)))
	{
		return false;
	}

	if (FAILED(m_pXAudio->CreateMasteringVoice(&m_pMasteringVoice)))
	{
		return false;
	}

	return true;
}
void SoundManager::DirectPlay(const std::string& filePath, IXAudio2SourceVoice* pSourceVoice, float volume , bool isLoop)
{
	std::ifstream file;
	file.open(filePath, std::ios_base::binary);

	if (file.fail())
	{
		assert(0);
	}

	// Load Riff Header
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}


	// Load Format Chunk
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	// Load Data chunk
	Chunk data;
	file.read((char*)&data, sizeof(data));


	// Load WaveData is Chunk

	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	file.close();



	WAVEFORMATEX wfex{};

	// Set WaveFormat
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;


	// Craete SourceVoice
	auto result = m_pXAudio->CreateSourceVoice(&pSourceVoice, &wfex,0,2.0f,
		&voiceCallBack);

	if (FAILED(result))
	{
		delete[] pBuffer;
		return;
	}

	// Set WaveData
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = data.size;
	buf.pContext = pBuffer;	
	if (isLoop)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// Play WaveData
	pSourceVoice->SetVolume(volume);
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

	return;
}
void SoundManager::DirectPlay(const std::string& filePath, float volume, bool isLoop)
{
	std::ifstream file;
	file.open(filePath, std::ios_base::binary);

	if (file.fail())
	{
		assert(0);
	}

	// Load Riff Header
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}


	// Load Format Chunk
	FormatChunk format;
	file.read((char*)&format, sizeof(format));

	// Load Data chunk
	Chunk data;
	file.read((char*)&data, sizeof(data));


	// Load WaveData is Chunk

	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	file.close();

	IXAudio2SourceVoice* pSourceVoice = nullptr;

	WAVEFORMATEX wfex{};

	// Set WaveFormat
	memcpy(&wfex, &format.fmt, sizeof(format.fmt));
	wfex.wBitsPerSample = format.fmt.nBlockAlign * 8 / format.fmt.nChannels;


	// Craete SourceVoice
	auto result = m_pXAudio->CreateSourceVoice(&pSourceVoice, &wfex, 0, 2.0f,
		&voiceCallBack);

	if (FAILED(result))
	{
		delete[] pBuffer;
		return;
	}

	// Set WaveData
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (BYTE*)pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = data.size;
	buf.pContext = pBuffer;
	if (isLoop)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// Play WaveData
	pSourceVoice->SetVolume(volume);
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();

	return;
}
