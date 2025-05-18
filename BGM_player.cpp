#include "BGM_player.h"
#include "utils.h"
#include <iostream>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")


BGM_Player::BGM_Player():pxAudio(nullptr),pMasteringVoice(nullptr),pSourceVoice(nullptr),pCallback(nullptr), is_paused(false), volume(0.5f)
{

}

HRESULT BGM_Player::LoadBGM(BGM_Info* pBGMInfo)
{
	if (pSourceVoice){
		Stop();
		pSourceVoice->DestroyVoice();
	}
	is_paused = true;
	curBGM = pBGMInfo;
	// set buffer
	HRESULT hr;
	if (pCallback)
		delete pCallback;
	pCallback = new PlayerCallback();
	pCallback->player = this;
	if (FAILED(hr = pxAudio->CreateSourceVoice(&pSourceVoice,&pBGMInfo->wavHeader,0,2.0f, pCallback,NULL,NULL)))
		return hr;
	samplesPlayedBeforeLastStop = 0;
	volume = SetVolume(volume);
	// load buffer from file
	buffer.clear();
	std::fstream fs;
	if(pBGMInfo->type == DAT_FILE)
		fs = std::fstream(pBGMInfo->BGMFilePath_dat, std::ios::in | std::ios::binary);
	else
		fs = std::fstream(pBGMInfo->BGMFilePath_wav, std::ios::in | std::ios::binary);
	if (!fs.is_open())
		return E_FAIL;
	fs.seekg(pBGMInfo->GetBeginPos(), std::ios::beg);
	int totLen = pBGMInfo->GetTotalLen();
	buffer.resize(totLen);
	fs.read((char*)buffer.data(), totLen);
	if (!fs.good()){
		return E_FAIL;
	}

	// set buffer
	return S_OK;
}


HRESULT BGM_Player::Init()
{
	HRESULT hr=S_OK;
	if (FAILED(hr = XAudio2Create(&pxAudio, 0)))
		return hr;
	if (FAILED(hr = pxAudio->CreateMasteringVoice(&pMasteringVoice)))
		return hr;
}

HRESULT BGM_Player::Play(BGM_Info* pInfo, DWORD pos)
{
	if (!pSourceVoice || curBGM!=pInfo){
		this->LoadBGM(pInfo);
	}
	is_paused = false;
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	if (state.BuffersQueued==0){
		memset(&buf,0,sizeof(buf));
		buf.Flags = 0;
		DWORD sample = pInfo->wavHeader.nChannels * pInfo->wavHeader.wBitsPerSample / 8;
		buf.pAudioData = buffer.data();
		buf.AudioBytes = buffer.size()/sample * sample; //aligned to sample
		if (pos < 0)pos = 0;
		if (pos < pInfo->GetBeginLoopLen()){
			buf.PlayBegin = pos / sample;
			buf.PlayLength = pInfo->GetBeginLoopLen()/ sample - buf.PlayBegin;
			buf.LoopBegin = pInfo->GetBeginLen()/ sample;
			buf.LoopLength = pInfo->GetLoopLen() / sample;
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		}else{
			if (pos > pInfo->GetTotalLen())
				pos = pInfo->GetTotalLen() - 100;
			buf.PlayBegin = pos / sample;
			buf.PlayLength = pInfo->GetTotalLen() / sample - buf.PlayBegin;
			buf.LoopBegin = 0;
			buf.LoopLength = 0;
			buf.LoopCount = 0;
		}
		samplesPlayedBeforeLastStop = state.SamplesPlayed - buf.PlayBegin;
		buf.pContext = NULL;
		HRESULT hr = pSourceVoice->SubmitSourceBuffer(&buf);
		if (FAILED(hr))
			return hr;
	}
	return pSourceVoice->Start(0, XAUDIO2_COMMIT_NOW);
}

HRESULT BGM_Player::Stop()
{
	if (!pSourceVoice)
		return E_FAIL;
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	while (state.BuffersQueued){
		HRESULT hr = pSourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
		if (FAILED(hr))
			return hr;
		if (FAILED(hr = pSourceVoice->FlushSourceBuffers()))
			return hr;
		Sleep(1);
		pSourceVoice->GetState(&state);
	}
	samplesPlayedBeforeLastStop = state.SamplesPlayed;
	is_paused = true;
	return S_OK;
}

HRESULT BGM_Player::Pause()
{
	if (!pSourceVoice)
		return E_FAIL;
	is_paused = true;
	return pSourceVoice->Stop(0, XAUDIO2_COMMIT_NOW);
}

HRESULT BGM_Player::SetPos(BGM_Info* pInfo, DWORD pos)
{
	if (!pSourceVoice)
		return E_FAIL;
	HRESULT hr;
	bool pause2 = is_paused;
	hr = this->Stop();
	if (FAILED(hr))
		return hr;
	if (FAILED(hr = this->Play(pInfo, pos)))
		return hr;
	if(pause2)
		return this->Pause();
	return hr;
}

DWORD BGM_Player::GetCurPos()
{
	if (!pSourceVoice)
		return 0;
	XAUDIO2_VOICE_STATE state;
	pSourceVoice->GetState(&state);
	DWORD sample = curBGM->wavHeader.nChannels * curBGM->wavHeader.wBitsPerSample / 8;
	return (state.SamplesPlayed - samplesPlayedBeforeLastStop) * sample;
}

HRESULT BGM_Player::ResetCurBGM()
{
	auto pos = this->GetCurPos();
	bool pause2 = is_paused;
	HRESULT hr = this->Stop();
	if (FAILED(hr))
		return hr;
	if (FAILED(hr = Play(this->curBGM, pos)))
		return hr;
	if (pause2)
		return this->Pause();
	return hr;
}

float BGM_Player::SetVolume(float vin)
{
	volume = vin * vin /10.0f;
	if (pSourceVoice){
		pSourceVoice->SetVolume(vin * vin / 10.0f);
		volume = GetVolume();
	}
	return volume;
}

float BGM_Player::GetVolume()
{
	if (pSourceVoice) {
		float v = 0.0f;
		pSourceVoice->GetVolume(&v);
		volume = sqrt(v*10.0f);
	}
	return volume;
}


BGM_Player::~BGM_Player()
{
	if(pMasteringVoice)pMasteringVoice->DestroyVoice();
	if (pxAudio)pxAudio->Release();
}
