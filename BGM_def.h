#pragma once
#include <string>
#define NOMINMAX
#include <Windows.h>
#include <fstream>
#include <vector>
#include <ostream>

enum BGM_Type
{
	WAV_FILE,
	DAT_FILE,
	WAV_FILE_EOSD,
};

struct BGM_Info
{
	WAVEFORMATEX wavHeader;

	BGM_Type type;
	std::wstring BGMFilePath_dat;
	std::wstring BGMFilePath_wav;

	std::string idName;
	std::string bgmName;

	DWORD unknown;

	DWORD beginPos;
	DWORD beginLen;
	DWORD beginloopLen;
	DWORD totalLen;
	// bgm:
	// ----------begin-----------------|-------loop---------|---others---
	//beginpos                   loopPos      
	// --------------beginrLen---------|-----loopLen--------
	// -------------------beginloopLen----------------------    
	// ---------------------totalLen-------------------------------------
	DWORD GetBeginPos() { return beginPos; }
	DWORD GetLoopPos() { return beginPos + beginLen; }

	DWORD GetBeginLen() { return beginLen; }
	DWORD GetLoopLen() { return beginloopLen > beginLen? beginloopLen - beginLen : 0; }
	DWORD GetBeginLoopLen() { return beginloopLen; }

	DWORD GetTotalLen() { return totalLen; }

	float GetBeginLen_s() { return (float)GetBeginLen() / wavHeader.nAvgBytesPerSec; }
	float GetLoopLen_s() { return (float)GetLoopLen() / wavHeader.nAvgBytesPerSec; }
	float GetBeginLoopLen_s() { return (float)GetBeginLoopLen() / wavHeader.nAvgBytesPerSec; }
	float GetTotalLen_s() { return (float)GetTotalLen() / wavHeader.nAvgBytesPerSec; }
	DWORD GetSampleByteSize() { return wavHeader.nChannels * wavHeader.wBitsPerSample / 8; }

	void SetBeginLen(int len) {
		// not use DWORD because need to compare to 0
		if (len < 0)len = 0; 
		if (len >= GetBeginLoopLen())len = beginloopLen - wavHeader.nChannels * wavHeader.wBitsPerSample;
		int ssz = GetSampleByteSize();
		len = len / ssz * ssz;
		// align
		beginLen = len; 
	}
	void SetBeginLoopLen(int len) {
		if (len < GetBeginLen())len = GetBeginLen() + wavHeader.nChannels*wavHeader.wBitsPerSample;
		if (len > totalLen)len = totalLen;
		int ssz = GetSampleByteSize();
		len = len / ssz * ssz;
		// align
		beginloopLen = len; 
	}
	void SetBeginLen(float ratio) { 
		SetBeginLen((int)(ratio * GetTotalLen()));
	}
	void SetBeginLoopLen(float ratio) { 
		SetBeginLoopLen((int)(ratio * GetTotalLen()));
	}

	bool SetToWav(WCHAR* path);

	double GetWavSec(DWORD len) { return (double)len/(double)(wavHeader.nAvgBytesPerSec); }
};

std::ostream& operator<<(std::ostream& os, BGM_Info& info);

struct SingleBGM {
	char file_name[16];
	DWORD begin_pos;
	DWORD unknown;
	DWORD begin_len;
	DWORD total_len;
	WAVEFORMATEX wav_header;
};

struct SingleBGM06
{
	DWORD loop_pos_samples;
	DWORD end_pos_samples;
};

enum Version
{
	EOSD,
	PCB_TO_TD,
	TD_AND_LATER
};

struct BGMWavExportOption
{
	bool enable_ending;
	int loop_time; // >=1
	float sec_for_fade_in;
	float sec_for_fade_out;
};

class BGM_Dump
{
	std::wstring datPath;
	std::wstring cmtPath;
	std::wstring fmtPath;
	std::wstring wavFolder;
	std::wstring posFolder;
public:
	std::vector<BGM_Info> bgmInfos;
	Version version;
	void LoadBGM(std::wstring dat_path, std::wstring fmt_path,bool is_TH13_or_later, std::wstring cmt_path = L"");
	void LoadBGM(std::wstring wav_folder, std::wstring pos_folder);//EoSD
	std::string GetBGM4AllFormat();
	void ExportBGM(const WCHAR* folder);
	void GetWavFile(BGM_Info * info, std::vector<BYTE> &begin,std::vector<BYTE> &loop,std::vector<BYTE> &end);
	void ExportBGMWavs(const WCHAR* folder, BGMWavExportOption* opt);
};

std::ostream& operator<<(std::ostream& os, BGM_Dump& dmp);