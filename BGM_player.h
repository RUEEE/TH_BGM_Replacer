#pragma once
#include <windows.h>
#include <dsound.h>
#include "BGM_def.h"

class BGM_Player
{
	LPDIRECTSOUND lpDirectSound;
	
	LPDIRECTSOUNDBUFFER pSoundBuffer;
	int bufferSize;

	DWORD playPos;
	DWORD LoopPos;
public:
	BGM_Player(HWND hwnd);
	bool LoadBGM(BGM_Info* pBgm, HANDLE loopEvent);

	void Play();
	void Stop();
	void Pause();
	void JumpLoop();

	~BGM_Player();
};