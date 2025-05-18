#pragma once
#include <windows.h>
#include <dsound.h>
#include "BGM_def.h"

#include <xaudio2.h>


class BGM_Player
{
	friend class PlayerCallback;
protected:
	IXAudio2* pxAudio;
	IXAudio2MasteringVoice* pMasteringVoice;
	IXAudio2SourceVoice* pSourceVoice;
	std::vector<BYTE> buffer;
	XAUDIO2_BUFFER buf;

	BGM_Info* curBGM;
	unsigned long long samplesPlayedBeforeLastStop;
	bool is_paused;
	float volume;
public:
	PlayerCallback* pCallback;
	BGM_Player();
	HRESULT Init();
	HRESULT LoadBGM(BGM_Info* pBgm);

	HRESULT Play(BGM_Info* pBgm, DWORD pos = 0);
	HRESULT Stop();
	HRESULT Pause();
	HRESULT SetPos(BGM_Info* pBgm, DWORD pos);
	DWORD GetCurPos();
	HRESULT ResetCurBGM();
	float SetVolume(float vin);
	float GetVolume();

	~BGM_Player();
	
};

class PlayerCallback : public IXAudio2VoiceCallback
{
public:
	PlayerCallback() :player(nullptr){}
	~PlayerCallback() {}
	BGM_Player* player;
	void _stdcall OnVoiceProcessingPassStart(UINT32 BytesRequired) {};
	void _stdcall OnVoiceProcessingPassEnd () {};
	void _stdcall OnStreamEnd() {};
	void _stdcall OnBufferStart(void* pBufferContext) {};
	void _stdcall OnBufferEnd(void* pBufferContext) {};
	void _stdcall OnLoopEnd(void* pBufferContext) { 
		if (player){
			DWORD sample = player->curBGM->wavHeader.nChannels * player->curBGM->wavHeader.wBitsPerSample / 8;
			player->samplesPlayedBeforeLastStop += player->curBGM->GetLoopLen() / sample;
		}
	};
	void _stdcall OnVoiceError(void* pBufferContext, HRESULT Error) {};
};
