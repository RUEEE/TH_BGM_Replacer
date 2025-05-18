#pragma once
#include <string>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <ostream>

enum BGM_Type
{
	WAV_FILE,
	DAT_FILE,
};

struct BGM_Info
{
	WAVEFORMATEX wavHeader;

	BGM_Type type;
	std::wstring BGMFilePath_dat;
	std::wstring BGMFilePath_wav;

	std::string idName;
	std::string bgmName;

	DWORD enterPos;
	DWORD enterLen;
	DWORD enterloopLen;
	DWORD totalLen;
	// bgm:
	// ----------enter-----------------|-------loop---------|---others---
	// enterpos                   loopPos      
	// ---------------enterLen---------|-----loopLen--------
	// -------------------enterloopLen----------------------    
	// ---------------------totalLen-------------------------------------
	DWORD GetEnterPos() { return enterPos; }
	DWORD GetLoopPos() { return enterPos + enterLen; }

	DWORD GetEnterLen() { return enterLen; }
	DWORD GetLoopLen() { return enterloopLen - enterLen; }
	DWORD GetEnterLoopLen() { return enterloopLen; }

	DWORD GetTotalLen() { return totalLen; }
	double GetWavSec(DWORD len) { return (double)len/(double)(wavHeader.nAvgBytesPerSec); }
};

std::ostream& operator<<(std::ostream& os, BGM_Info& info);

class BGM_Dump
{
	std::wstring datPath;
	std::wstring cmtPath;
	std::wstring fmtPath;
public:
	std::vector<BGM_Info> bgmInfos;

	void LoadBGM13(std::wstring dat_path, std::wstring fmt_path, std::wstring cmt_path = L"");
	std::string GetBGM4AllFormat13();
};

std::ostream& operator<<(std::ostream& os, BGM_Dump& dmp);