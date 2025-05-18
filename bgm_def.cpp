#include <format>
#include <sstream>
#include <map>
#include <filesystem>
#include "BGM_def.h"
#include "utils.h"

const std::wstring bgm_names06[17] = {
	L"赤より紅い夢",
	L"ほおずきみたいに紅い魂",
	L"妖魔夜行",
	L"ルーネイトエルフ",
	L"おてんば恋娘",
	L"上海紅茶館　～ Chinese Tea",
	L"明治十七年の上海アリス",
	L"ヴワル魔法図書館",
	L"ラクトガール　～ 少女密室",
	L"メイドと血の懐中時計",
	L"月時計　～ ルナ ダイアル",
	L"ツェペシュの幼き末裔",
	L"亡き王女の為のセプテット",
	L"魔法少女達の百年祭",
	L"U.N.オーエンは彼女なのか？",
	L"紅より儚い永遠",
	L"紅楼　～ Eastern Dream..."
};
const std::wstring bgm_wavs06[17] = {
	L"th06_01.wav",
	L"th06_02.wav",
	L"th06_03.wav",
	L"th06_04.wav",
	L"th06_05.wav",
	L"th06_06.wav",
	L"th06_07.wav",
	L"th06_08.wav",
	L"th06_09.wav",
	L"th06_10.wav",
	L"th06_11.wav",
	L"th06_12.wav",
	L"th06_13.wav",
	L"th06_14.wav",
	L"th06_15.wav",
	L"th06_16.wav",
	L"th06_17.wav",
};
const std::wstring bgm_poses06[17] = {
	L"th06_01.pos",
	L"th06_02.pos",
	L"th06_03.pos",
	L"th06_04.pos",
	L"th06_05.pos",
	L"th06_06.pos",
	L"th06_07.pos",
	L"th06_08.pos",
	L"th06_09.pos",
	L"th06_10.pos",
	L"th06_11.pos",
	L"th06_12.pos",
	L"th06_13.pos",
	L"th06_14.pos",
	L"th06_15.pos",
	L"th06_16.pos",
	L"th06_17.pos",
};

void BGM_Dump::LoadBGM(std::wstring dat_path, std::wstring fmt_path, bool is_TH13_or_later1, std::wstring cmt_path)
{
	version = is_TH13_or_later1?TD_AND_LATER:PCB_TO_TD;

	datPath = dat_path;
	fmtPath = fmt_path;
	cmtPath = cmt_path;
	std::fstream fs(fmt_path, std::ios::in | std::ios::binary);
	
	if (fs.is_open())
	{
		bgmInfos.clear();
		std::map<std::string, std::string> map_names;
		if (!cmt_path.empty())
		{
			std::fstream cmtfs(cmt_path, std::ios::in);
			std::string line;
			std::string id;
			std::string name;
			while (std::getline(cmtfs, line)){
				auto trimString = [](const std::string& str)->std::string {
						size_t start = str.find_first_not_of(' ');
						if (start == std::string::npos) {
							return "";
						}
						size_t end = str.find_last_not_of(' ');
						return str.substr(start, end - start + 1);
					};
				line = trimString(line);
				if (line[0] == '#' || line.empty())
					continue;
				if (line[0] == '@'){
					size_t start = line.find_last_of('/');
					id = line.substr(start + 1, line.size() - start - 1);
					start = id.find_last_of('.');
					id = id.substr(0, start);//remove .mid
				}else{
					if (!id.empty()){
						name = line;
						map_names[id+".wav"] = name.c_str();
						id = "";
					}
				}
			}
		}
		
		SingleBGM bgm = { 0 };
		while (!fs.fail()) {
			fs.read((char*)&bgm, sizeof(SingleBGM));
			if (fs.fail())
				break;
			BGM_Info info;
			info.unknown = bgm.unknown;
			info.beginLen = bgm.begin_len;
			info.beginPos = bgm.begin_pos;
			info.idName = std::string(bgm.file_name);
			info.bgmName = map_names.contains(info.idName) ? map_names[info.idName] : "";
			info.beginloopLen = info.totalLen = bgm.total_len;
			info.wavHeader = bgm.wav_header;
			info.BGMFilePath_dat = datPath;
			info.type = DAT_FILE;
			bgmInfos.push_back(info);
		}
		fs.close();
	}
}

void BGM_Dump::LoadBGM(std::wstring wav_folder, std::wstring pos_folder)
{
	version = EOSD;
	datPath = L"";
	fmtPath = L"";
	cmtPath = L"";
	wavFolder = wav_folder;
	posFolder = pos_folder;
	bgmInfos.clear();
	for (int i = 0; i < 17; i++){
		auto& name = bgm_names06[i];
		auto& wav_filename = bgm_wavs06[i];
		auto& wav_posname = bgm_poses06[i];

		std::filesystem::path wav_path = wav_folder;
		wav_path = wav_path / wav_filename;
		std::filesystem::path pos_path = pos_folder;
		pos_path = pos_path / wav_posname;

		// read wav file head
		BGM_Info info = { 0 };
		std::fstream fs(wav_path, std::ios::in | std::ios::binary);
		if (!fs.is_open())
			continue;
		struct {
			char chunkID[4];
			DWORD chunksize;
			char format[4];
			char subchunk1ID[4];
			DWORD Subchunk1Size;
		}header1 = { 0 };
		fs.read((char*)&header1, sizeof(header1));
		fs.read((char*)&info.wavHeader, 0x10);
		info.wavHeader.cbSize = 0;
		info.totalLen = header1.chunksize + 8 - 44;

		// read pos file head
		int samples = info.wavHeader.nChannels * info.wavHeader.wBitsPerSample / 8;
		std::fstream fs_pos(pos_path, std::ios::in | std::ios::binary);
		if (!fs_pos.is_open())
			continue;
		SingleBGM06 bgm06 = { 0 };
		fs_pos.read((char*)&bgm06, sizeof(bgm06));

		
		info.unknown = 0;
		info.beginPos = 44;
		info.beginLen = bgm06.loop_pos_samples * samples - 44;
		info.beginloopLen = bgm06.end_pos_samples * samples - 44;

		info.idName = wstr2str(wav_filename.c_str());
		info.bgmName = wstr2str(name.data(), 932);
		info.BGMFilePath_dat = L"";
		info.BGMFilePath_wav = wav_path.c_str();
		info.type = WAV_FILE_EOSD;
		bgmInfos.push_back(info);
	}

}

std::string BGM_Dump::GetBGM4AllFormat()
{
	std::stringstream ss;
	// PATH = ..\TH15 - GZZ\thbgm.dat
	// SAMPLE = 44100
	// CHANNEL = 2
	// BIT = 16
	if (bgmInfos.empty())
		return "";
	ss << std::format("PATH = {}\n" , (char*)wstr2u8(datPath.c_str()).c_str());
	ss << std::format("SAMPLE = {}\n", bgmInfos[0].wavHeader.nSamplesPerSec);
	ss << std::format("CHANNEL = {}\n", bgmInfos[0].wavHeader.nChannels);
	ss << std::format("BIT = {}\n", bgmInfos[0].wavHeader.wBitsPerSample);
	for (auto& it : bgmInfos) {
		//BGM = 宇宙巫女現る, 0x10, 0x124B00, 0x124B10, 0x17AF050
		if (it.type == DAT_FILE)
		{
			if(it.bgmName.empty())
				ss << std::format("BGM = {}, 0x{:x},0x{:x},0x{:x},0x{:x}\n", it.idName,it.GetBeginPos(),it.GetBeginLen(),it.GetLoopPos(),it.GetLoopLen());
			else
				ss << std::format("BGM = {}, 0x{:x},0x{:x},0x{:x},0x{:x}\n", (char*)str2u8(it.bgmName.c_str(), 932).c_str(), it.GetBeginPos(), it.GetBeginLen(), it.GetLoopPos(), it.GetLoopLen());
		}
	}
	return ss.str();
}

void BGM_Dump::ExportBGM(const WCHAR* folder)
{
	if (version == EOSD)
	{
		for (int i = 0; i < 17; i++)
		{
			auto& it = bgmInfos[i];
			auto& wav_posname = bgm_poses06[i];

			std::filesystem::path pos_path = folder;
			pos_path = pos_path / wav_posname;

			// read wav file head
			int samples = it.wavHeader.nChannels * it.wavHeader.wBitsPerSample / 8;

			SingleBGM06 bgm06 = { 
				(it.beginLen + 44)/samples,
				(it.beginloopLen + 44) / samples };

			std::fstream pos_ofile(pos_path, std::ios::out | std::ios::binary | std::ios::trunc);
			pos_ofile.write((char*)&bgm06, sizeof(bgm06));
			pos_ofile.close();
		}
	}else{
		std::fstream dat_ifile(datPath, std::ios::in | std::ios::binary);
		if (!dat_ifile.is_open())
			return;
		// thbgm.dat header
		char dat_head[0x10];
		dat_ifile.read(dat_head, 0x10);

		// contents
		std::vector<std::vector<BYTE>> wav_bytes;
		std::vector<SingleBGM> fmts;
		wav_bytes.resize(bgmInfos.size());
		fmts.resize(bgmInfos.size());
		DWORD dat_file_ofs = 0x10;
		for (int i = 0; i < bgmInfos.size(); i++) {
			auto& it = bgmInfos[i];
			memset(&fmts[i], 0, sizeof(fmts[i]));
			for (int j = 0; j < 16; j++) {
				fmts[i].file_name[j] = j < it.idName.size() ? it.idName[j] : 0;
			}
			fmts[i].begin_pos = dat_file_ofs;
			fmts[i].total_len = it.GetBeginLoopLen();
			fmts[i].begin_len = it.GetBeginLen();
			fmts[i].wav_header = it.wavHeader;
			fmts[i].unknown = version == TD_AND_LATER ? fmts[i].total_len : it.unknown;

			// bgm reading
			if (it.type == DAT_FILE) {
				dat_ifile.seekg(it.GetBeginPos(), std::ios::beg);
				wav_bytes[i].resize(it.GetBeginLoopLen());
				dat_ifile.read((char*)wav_bytes[i].data(), it.GetBeginLoopLen());
				dat_file_ofs += it.GetBeginLoopLen();
			}
			else {
				std::fstream wav_ifile(it.BGMFilePath_wav, std::ios::in | std::ios::binary);
				wav_ifile.seekg(it.GetBeginPos(), std::ios::beg);
				wav_bytes[i].resize(it.GetBeginLoopLen());
				wav_ifile.read((char*)wav_bytes[i].data(), it.GetBeginLoopLen());
				dat_file_ofs += it.GetBeginLoopLen();
				wav_ifile.close();
			}
		}

		// write to file (dat)
		std::filesystem::path dat_opath = folder;
		dat_opath = dat_opath / "thbgm.dat";
		std::fstream dat_ofile_fs(dat_opath, std::ios::out | std::ios::binary | std::ios::trunc);

		dat_ofile_fs.write(dat_head, 0x10);
		for (auto& it : wav_bytes)
			dat_ofile_fs.write((char*)it.data(), it.size());
		dat_ofile_fs.close();

		// write to file (fmt)
		std::filesystem::path fmt_opath = folder;
		fmt_opath = fmt_opath / "thbgm.fmt";
		std::fstream fmt_ofile_fs(fmt_opath, std::ios::out | std::ios::binary | std::ios::trunc);
		for (int i = 0; i < fmts.size(); i++)
			fmt_ofile_fs.write((char*)(&(fmts[i])), sizeof(fmts[i]));
		char fmt_end[0x11] = { 0 };
		fmt_ofile_fs.write(fmt_end, 0x11);
		fmt_ofile_fs.close();
	}
}

std::ostream& operator<<(std::ostream& os, BGM_Info& info)
{
	return os << std::format("{}:{};({},{},{})\n", info.bgmName, info.idName, info.beginPos, info.beginLen, info.beginloopLen);
}

std::ostream& operator<<(std::ostream& os, BGM_Dump& dmp)
{
	for (auto &it : dmp.bgmInfos){
		os << it;
	}
	return os;
}

bool BGM_Info::SetToWav(WCHAR* path)
{
	this->type = WAV_FILE;
	std::fstream fs(path, std::ios::in | std::ios::binary);
	struct {
		char chunkID[4];
		DWORD chunksize;
		char format[4];
		char subchunk1ID[4];
		DWORD Subchunk1Size;
	}header1 = { 0 };
	fs.read((char*)&header1, sizeof(header1));
	if (header1.chunkID[0] != 'R' || header1.chunkID[1] != 'I' || header1.chunkID[2] != 'F' || header1.chunkID[3] != 'F')
		return false;
	if (header1.format[0] != 'W' || header1.format[1] != 'A' || header1.format[2] != 'V' || header1.format[3] != 'E')
		return false;
	if (header1.subchunk1ID[0] != 'f' || header1.subchunk1ID[1] != 'm' || header1.subchunk1ID[2] != 't' || header1.subchunk1ID[3] != ' ')
		return false;

	fs.read((char*)&this->wavHeader, 0x10);
	this->wavHeader.cbSize = 0;

	this->beginPos = 44; // wav begin
	this->beginLen = 0;
	this->beginloopLen = header1.chunksize + 8 - 44;
	this->totalLen = header1.chunksize + 8 - 44;

	BGMFilePath_wav = path;
	return true;
}
