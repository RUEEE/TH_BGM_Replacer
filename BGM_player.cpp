#include "BGM_player.h"
#include "utils.h"
#include <iostream>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")


BGM_Player::BGM_Player(HWND hwnd):LoopPos(0),bufferSize(0),playPos(0),pSoundBuffer(nullptr)
{
	if (FAILED(DirectSoundCreate(nullptr, &lpDirectSound, nullptr))){
		return;
	}
	lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
}

bool BGM_Player::LoadBGM(BGM_Info* pBGMInfo, HANDLE loopEvent)
{
	// set buffer
	if (pSoundBuffer)
	{
		pSoundBuffer->Release();
		pSoundBuffer = nullptr;
	}
	bufferSize = pBGMInfo->GetTotalLen();
	DSBUFFERDESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
	desc.dwBufferBytes = bufferSize;
	desc.lpwfxFormat = &pBGMInfo->wavHeader;
	if (FAILED(lpDirectSound->CreateSoundBuffer(&desc, &pSoundBuffer, nullptr)))
		return false;

	// load buffer from file
	LPVOID lpvPtr1;
	DWORD dwBytes1;
	if (FAILED(pSoundBuffer->Lock(0, 0, &lpvPtr1, &dwBytes1, 0, 0, DSBLOCK_ENTIREBUFFER)))
	{
		pSoundBuffer->Release();
		pSoundBuffer = nullptr;
		return false;
	}
	std::fstream fs;
	if(pBGMInfo->type == DAT_FILE)
		fs = std::fstream(pBGMInfo->BGMFilePath_dat, std::ios::in | std::ios::binary);
	else
		fs = std::fstream(pBGMInfo->BGMFilePath_wav, std::ios::in | std::ios::binary);
	
	if (!fs.is_open())
		return false;
	char ch;
	fs.read(&ch, 1);
	fs.seekg(pBGMInfo->GetEnterPos(), std::ios::beg);
	int len = 0;
	int totLen = pBGMInfo->GetTotalLen();

	fs.read((char*)lpvPtr1, dwBytes1);
	if (!fs.good())
	{
		pSoundBuffer->Unlock(lpvPtr1, dwBytes1, 0, 0);
		pSoundBuffer->Release();
		pSoundBuffer = nullptr;
		return false;
	}

	// set event
	DSBPOSITIONNOTIFY PositionNotify;
	LPDIRECTSOUNDNOTIFY8 lpDsNotify;

	if (FAILED(pSoundBuffer->QueryInterface(IID_IDirectSoundNotify,(LPVOID*)&lpDsNotify))){
		pSoundBuffer->Release();
		pSoundBuffer = nullptr;
		return false;
	}
	PositionNotify.dwOffset = pBGMInfo->GetEnterLoopLen() - 1;
	PositionNotify.hEventNotify = loopEvent;
	if (FAILED(lpDsNotify->SetNotificationPositions(1, &PositionNotify))){
		pSoundBuffer->Release();
		pSoundBuffer = nullptr;
		return false;
	}
	lpDsNotify->Release();

	playPos = 0;
	LoopPos = pBGMInfo->GetEnterLen();
	return true;
}


void BGM_Player::Play()
{
	if (playPos > bufferSize)
		playPos = bufferSize;
	pSoundBuffer->SetCurrentPosition(playPos);
	pSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
}

void BGM_Player::Stop()
{
	pSoundBuffer->Stop();
	playPos = 0;
	pSoundBuffer->Release();
}

void BGM_Player::Pause()
{
	pSoundBuffer->Stop();
	DWORD dwWriteCursor;
	pSoundBuffer->GetCurrentPosition(&playPos, &dwWriteCursor);
}

void BGM_Player::JumpLoop()
{
	pSoundBuffer->SetCurrentPosition(LoopPos);
}

BGM_Player::~BGM_Player()
{
	lpDirectSound->Release();
}
