#include <format>
#include <sstream>
#include <map>
#include "BGM_def.h"
#include "utils.h"

void BGM_Dump::LoadBGM13(std::wstring dat_path, std::wstring fmt_path, std::wstring cmt_path)
{
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
				}else{
					if (!id.empty()){
						name = line;
						map_names[id+".wav"] = name.c_str();
						id = "";
					}
				}
			}
		}
		struct SingleBGM13 {
			char file_name[16];
			DWORD enter_pos;
			DWORD total_len;
			DWORD enter_len;
			DWORD total_len2;
			WAVEFORMATEX wav_header;
		};
		SingleBGM13 bgm = { 0 };
		while (!fs.fail()) {
			fs.read((char*)&bgm, sizeof(SingleBGM13));
			if (fs.fail())
				break;
			BGM_Info info;
			info.enterLen = bgm.enter_len;
			info.enterPos = bgm.enter_pos;
			info.idName = std::string(bgm.file_name);
			info.bgmName = map_names.contains(info.idName)?map_names[info.idName]:"";
			info.enterloopLen = info.totalLen = bgm.total_len;
			info.wavHeader = bgm.wav_header;
			info.BGMFilePath_dat = datPath;
			info.type = DAT_FILE;
			bgmInfos.push_back(info);
		}
		fs.close();
	}
}

std::string BGM_Dump::GetBGM4AllFormat13()
{
	std::stringstream ss;
	// PATH = ..\TH15 - GZZ\thbgm.dat
	// SAMPLE = 44100
	// CHANNEL = 2
	// BIT = 16
	if (bgmInfos.empty())
		return "";
	ss << std::format("PATH = {}\n", wstr2str(datPath.c_str()));
	ss << std::format("SAMPLE = {}\n", bgmInfos[0].wavHeader.nSamplesPerSec);
	ss << std::format("CHANNEL = {}\n", bgmInfos[0].wavHeader.nChannels);
	ss << std::format("BIT = {}\n", bgmInfos[0].wavHeader.wBitsPerSample);
	for (auto& it : bgmInfos) {
		//BGM = ÓîÖæÎ×Å®¬F¤ë, 0x10, 0x124B00, 0x124B10, 0x17AF050
		ss << std::format("BGM = {}, 0x{:x},0x{:x},0x{:x},0x{:x}\n", it.bgmName.empty()?it.idName: it.bgmName,it.GetEnterPos(),it.GetEnterLen(),it.GetLoopPos(),it.GetLoopLen());
		
	}
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, BGM_Info& info)
{
	return os << std::format("{}:{};({},{},{})\n", info.bgmName, info.idName, info.enterPos, info.enterLen, info.enterloopLen);
}

std::ostream& operator<<(std::ostream& os, BGM_Dump& dmp)
{
	for (auto &it : dmp.bgmInfos){
		os << it;
	}
	return os;
}
