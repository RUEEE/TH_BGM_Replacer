#include "window.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <format>
#include <imgui.h>
#include <memory>
#include "BGM_def.h"
#include "BGM_player.h"
#include <algorithm>

std::unique_ptr<BGM_Player> g_player;

bool WindowUpdateFunc(ImGuiWindow* pWind)
{
	static BGM_Dump dump;
	static int cur_select = -1;
	// static WCHAR dat_file[MAX_PATH] = L"C:\\disk\\touhou\\TH15-GZZ\\thbgm.dat";
	// static WCHAR fmt_file[MAX_PATH] = L"C:\\disk\\touhou\\TH15-GZZ\\data\\thbgm.fmt";
	// static WCHAR cmt_file[MAX_PATH] = L"C:\\disk\\touhou\\TH15-GZZ\\data\\musiccmt.txt";
	// 
	// static std::wstring wav_folder = L"C:\\disk\\touhou\\TH06-HMX\\bgm";
	// static std::wstring pos_folder = L"C:\\disk\\touhou\\TH06-HMX\\data";

	static WCHAR dat_file[MAX_PATH] = L"";
	static WCHAR fmt_file[MAX_PATH] = L"";
	static WCHAR cmt_file[MAX_PATH] = L"";

	static std::wstring wav_folder = L"";
	static std::wstring pos_folder = L"";

	// load file
	if (ImGui::Button("Load")) {
		ImGui::OpenPopup("load files");
	}
	ImVec2 wndSize = ImGui::GetWindowSize();
	ImVec2 nextWindSz = { ImGui::CalcTextSize("  Select musiccmt.txt(optional)  |AAAAAAAAAAAAAAAAAAAAAAAAAAAA").x,ImGui::GetFrameHeight() * 9.0f };
	ImGui::SetNextWindowSize(nextWindSz);
	ImGui::SetNextWindowPos({ wndSize.x * 0.5f - nextWindSz.x*0.5f,wndSize.y * 0.5f - nextWindSz.y * 0.5f });
	if (ImGui::BeginPopupModal("load files",0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetCursorPosY(ImGui::GetWindowHeight()*0.5f-ImGui::GetFrameHeight() * 2.5f);
		static int ver = 2;
		ImGui::Combo("version", &ver, "TH06\0TH07-TH13\0TH13+\0\0");
		if (ver == 0){
			//EOSD load
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, ImGui::CalcTextSize("  Select musiccmt.txt(optional)  ").x);
			if (ImGui::Button("Select wav folder")){
				wav_folder = LauncherWndFolderSelect(L"select wav folder");
				if (pos_folder.empty()){
					for (int i = wav_folder.size() - 1; i >= 0; i--)
						if (wav_folder[i] == L'\\' || wav_folder[i] == L'/') {
							pos_folder = wav_folder.substr(0, i);
							pos_folder += L"\\data";
							break;
						}
				}
			}
			ImGui::NextColumn();
			ImGui::Text("%s", wstr2u8(wav_folder.c_str()).c_str());
			ImGui::NextColumn();
			if (ImGui::Button("Select pos folder")){
				pos_folder = LauncherWndFolderSelect(L"select wav folder");
			}
			ImGui::NextColumn();
			ImGui::Text("%s", wstr2u8(pos_folder.c_str()).c_str());
			ImGui::Columns(1);
		}else{
			//other load
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, ImGui::CalcTextSize("  Select musiccmt.txt(optional)  ").x);
			if (ImGui::Button("Select thbgm.dat"))
			{
				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFilter = L"dat(*.dat)\0*.dat\0*.*\0\0";
				ofn.lpstrFile = dat_file;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&ofn)) {
					memset(dat_file, 0, sizeof(dat_file));
				}
				if (fmt_file[0] == '\0')
				{
					wcscpy_s(fmt_file, dat_file);
					for (int i = wcslen(fmt_file) - 1; i >= 0; i--)
						if (fmt_file[i] == L'\\' || fmt_file[i] == L'/'){
							wcscpy_s(&(fmt_file[i]),MAX_PATH - wcslen(dat_file), L"\\data\\thbgm.fmt");
							break;
						}
				}
				if (cmt_file[0] == '\0') {
					wcscpy_s(cmt_file, fmt_file);
					for (int i = wcslen(cmt_file) - 1; i >= 0; i--)
						if (cmt_file[i] == L'\\' || cmt_file[i] == L'/') {
							wcscpy_s(&(cmt_file[i]), MAX_PATH - wcslen(cmt_file), L"\\musiccmt.txt");
							break;
						}
				}
			}
			ImGui::NextColumn();
			ImGui::Text("%s", wstr2u8(dat_file).c_str());
			ImGui::NextColumn();
			if (ImGui::Button("Select thbgm.fmt"))
			{
				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFilter = L"fmt(*.fmt)\0*.fmt\0*.*\0\0";
				ofn.lpstrFile = fmt_file;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&ofn)) {
					memset(fmt_file, 0, sizeof(fmt_file));
				}
				if (cmt_file[0] == '\0'){
					wcscpy_s(cmt_file, fmt_file);
					for (int i = wcslen(cmt_file) - 1; i >= 0; i--)
						if (cmt_file[i] == L'\\' || cmt_file[i] == L'/'){
							wcscpy_s(&(cmt_file[i]), MAX_PATH - wcslen(cmt_file), L"\\musiccmt.txt");
							break;
						}
				}
			}
			ImGui::NextColumn();
			ImGui::Text("%s", wstr2u8(fmt_file).c_str());
			ImGui::NextColumn();
			if (ImGui::Button("Select musiccmt.txt(optional)"))
			{
				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFilter = L"txt(*.txt)\0*.txt\0*.*\0\0";
				ofn.lpstrFile = cmt_file;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&ofn)) {
					memset(cmt_file, 0, sizeof(cmt_file));
				}
			}
			ImGui::NextColumn();
			ImGui::Text("%s", wstr2u8(cmt_file).c_str());
			ImGui::NextColumn();
			ImGui::Columns(1);
		}
		if (ImGui::Button("OK",{ 100.0f,ImGui::GetFrameHeight() })) {
			g_player->Stop();
			ImGui::CloseCurrentPopup();
			if (ver != 0)
				dump.LoadBGM(dat_file, fmt_file, ver == 1, cmt_file);
			else
				dump.LoadBGM(wav_folder, pos_folder);
			cur_select = -1;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", { 100.0f,ImGui::GetFrameHeight() })) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// bgm lists
	// id (name) begin loop end total WAVfile
	if (ImGui::BeginTable("bgm list", 6, ImGuiTableFlags_::ImGuiTableFlags_Borders | ImGuiTableFlags_::ImGuiTableFlags_Resizable | ImGuiTableFlags_::ImGuiTableFlags_ScrollY, 
		{ ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y*0.6f}))
	{
		float sz = ImGui::GetWindowSize().x;
		float size_text = ImGui::CalcTextSize("AAA").x;
		DWORD flag = ImGuiTableColumnFlags_WidthStretch;
		DWORD flag2 = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
		ImGui::TableSetupColumn("id", flag2, size_text);
		size_text = ImGui::CalcTextSize("NAMENAMENAMENAME").x;
		ImGui::TableSetupColumn("name", flag, size_text);
		size_text = ImGui::CalcTextSize("endloop len").x;
		ImGui::TableSetupColumn("loop pos", flag2, size_text);
		ImGui::TableSetupColumn("end pos", flag2, size_text);
		ImGui::TableSetupColumn("total len", flag2, size_text);
		size_text = ImGui::CalcTextSize("C:/ABCD/EFGH/IJKL").x;
		ImGui::TableSetupColumn("file", flag, size_text);
		ImGui::TableHeadersRow();

		for (int i = 0; i < dump.bgmInfos.size(); i++)
		{
			auto& it = dump.bgmInfos[i];
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (ImGui::Selectable(std::format("{}{}", i, it.type == WAV_FILE ? '*' : ' ').c_str(), cur_select == i, ImGuiSelectableFlags_::ImGuiSelectableFlags_SpanAllColumns))
				cur_select = i;
			ImGui::TableNextColumn();
			if(it.bgmName.empty())
				ImGui::Text("%s", it.idName.c_str());
			else
				ImGui::Text("%s",str2u8(it.bgmName.c_str(), 932).c_str());
			// begin len
			ImGui::TableNextColumn();
			ImGui::Text("%.2lfs", it.GetWavSec(it.GetBeginLen()));
			//endloop
			ImGui::TableNextColumn();
			ImGui::Text("%.2lfs",  it.GetWavSec(it.GetBeginLoopLen()));
			//total
			ImGui::TableNextColumn();
			ImGui::Text("%.2lfs",  it.GetWavSec(it.GetTotalLen()));
			//file
			ImGui::TableNextColumn();
			ImGui::Text("%s", it.type == DAT_FILE ?  wstr2u8(it.BGMFilePath_dat.c_str()).c_str() : wstr2u8(it.BGMFilePath_wav.c_str()).c_str());
		}
		ImGui::EndTable();
	}

	ImGui::Separator();
	static BGM_Info* p_cur_bgm = nullptr;

	BGM_Info* p_cur_selected = (cur_select >= 0 && cur_select <= dump.bgmInfos.size()) ? (&dump.bgmInfos[cur_select]) : nullptr;
	bool enable_play = p_cur_bgm!=nullptr || p_cur_selected!=nullptr;
	bool disabled_export = dump.bgmInfos.size() == 0;

	static double cur_bgm_played_pos = 0;
	ImGui::BeginDisabled(!enable_play);
	if (ImGui::Button("Play")){
		p_cur_bgm = p_cur_selected;
		g_player->Play(p_cur_bgm);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")){
		g_player->Stop();
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause")){
		g_player->Pause();
	}
	ImGui::SameLine();
	ImGui::InvisibleButton("sep", {20.0f,1.0f});

	static WCHAR wav_changed[MAX_PATH];
	ImGui::SameLine();
	if (ImGui::Button("Change BGM") && (p_cur_bgm || p_cur_selected))
	{
		BGM_Info* bgm_to_change = p_cur_selected ? p_cur_selected : p_cur_bgm;
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = L"wav(*.wav)\0*.wav\0*.*\0\0";
		ofn.lpstrFile = wav_changed;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST;
		if (!GetOpenFileName(&ofn)) {
			memset(wav_changed, 0, sizeof(wav_changed));
		}
		bgm_to_change->SetToWav(wav_changed);
		if(p_cur_bgm == bgm_to_change)
			g_player->LoadBGM(bgm_to_change);
	}

	ImGui::SameLine();
	ImGui::InvisibleButton("sep2", { 30.0f,1.0f });
	static float volume = 0.5f;
	volume = g_player->GetVolume();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.0f);
	if (ImGui::SliderFloat("volume", &volume, 0.0f, 1.0f))
		volume = g_player->SetVolume(volume);
	// add BGM
	{
		ImGui::EndDisabled(!enable_play);
		ImGui::BeginDisabled(disabled_export);
		ImGui::SameLine();
		ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(),ImGui::GetWindowWidth() - 100.0f));
		if (ImGui::Button("Add BGM"))
		{
			ImGui::OpenPopup("bgm adder");
		}


		ImVec2 wndSize = ImGui::GetWindowSize();
		ImVec2 nextWindSz = { ImGui::CalcTextSize("BBBBBBBBBBBBBBBBBBBBB  |AAAAAAAAAAAAAAAAAAAAAAAAAAAA").x,ImGui::GetFrameHeight() * 7.0f };
		ImGui::SetNextWindowSize(nextWindSz);
		ImGui::SetNextWindowPos({ wndSize.x * 0.5f - nextWindSz.x * 0.5f,wndSize.y * 0.5f - nextWindSz.y * 0.5f });
		if (ImGui::BeginPopupModal("bgm adder", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImVec2 wndSize = ImGui::GetWindowSize();
			static char id_str[15];
			static WCHAR wav_file_path[MAX_PATH] = { 0 };
			auto charfilter = [](ImGuiInputTextCallbackData* data) -> int {
				if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
					if ((data->EventChar >= '0' && data->EventChar <= '9') || 
						(data->EventChar >= 'A' && data->EventChar <= 'Z') || 
						(data->EventChar >= 'a' && data->EventChar <= 'z')  || 
						(data->EventChar == '_')) {
						return 0;
					}
					return 1;
				}
				return 0;
				};
			ImGui::InputText("BGM id", id_str, sizeof(id_str), ImGuiInputTextFlags_CallbackCharFilter, charfilter);
			if (ImGui::Button("select wav file")){
				OPENFILENAME ofn = { 0 };
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.lpstrFilter = L"wav(*.wav)\0*.wav\0*.*\0\0";
				ofn.lpstrFile = wav_file_path;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&ofn)) {
					memset(wav_file_path, 0, sizeof(wav_file_path));
				}
			}
			ImGui::SameLine();
			ImGui::Text("%s##wav file", (char*)wstr2u8(wav_file_path).c_str());
			if (ImGui::Button("OK", { 100.0f,ImGui::GetFrameHeight() })){
				if (wav_file_path[0] != L'\0' && id_str[0]!='\0'){
					BGM_Info new_bgm = { 0 };
					new_bgm.idName = id_str;
					new_bgm.SetToWav(wav_file_path);
					dump.bgmInfos.push_back(new_bgm);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 100.0f,ImGui::GetFrameHeight() }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::EndDisabled(disabled_export);
		ImGui::BeginDisabled(!enable_play);
	}
	

	// progress bar
	ImGui::Separator();
	cur_bgm_played_pos = p_cur_bgm ? (double)g_player->GetCurPos() / p_cur_bgm->GetTotalLen() : 0.0f;
	float cur_bgm_tot_sec = p_cur_bgm ? p_cur_bgm->GetTotalLen_s() : 0.0f;
	ImGui::ProgressBar(cur_bgm_played_pos, ImVec2(-FLT_MIN, 0), std::format("{:.2f}/{:.2f}", cur_bgm_played_pos * cur_bgm_tot_sec , cur_bgm_tot_sec).c_str());
	ImVec2  prog_mx = ImGui::GetItemRectMax();
	ImVec2  prog_mn = ImGui::GetItemRectMin();
	static bool is_draging_tri1 = false, is_draging_tri2 = false;
	if (p_cur_bgm && ImGui::IsItemHovered(ImGuiMouseButton_Left) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !is_draging_tri1 && !is_draging_tri2) {
		cur_bgm_played_pos = (ImGui::GetMousePos().x - prog_mn.x) / (prog_mx.x - prog_mn.x);
		g_player->SetPos(p_cur_bgm, (DWORD)floor(cur_bgm_played_pos * p_cur_bgm->GetTotalLen()));
	}
	if (p_cur_bgm) {
		auto p = ImGui::GetWindowDrawList();
		float prog_loop = (float)p_cur_bgm->GetBeginLen() / p_cur_bgm->GetTotalLen();
		float prog_end = (float)p_cur_bgm->GetBeginLoopLen() / p_cur_bgm->GetTotalLen();
		float loop_x = (prog_mx.x - prog_mn.x) * prog_loop + prog_mn.x;
		float end_x = (prog_mx.x - prog_mn.x) * prog_end + prog_mn.x;
		DWORD loop_col = 0xFFFF0000;
		DWORD end_col = 0xFFFFFF00;
		p->AddLine({ loop_x,prog_mn.y }, { loop_x,prog_mx.y }, loop_col);
		p->AddLine({ end_x,prog_mn.y }, { end_x,prog_mx.y }, end_col);

		float prog_height = prog_mx.y - prog_mn.y;
		float tri_ratio = 0.2;
		ImVec2 lp1, lp2, lp3;
		lp1 = { loop_x,prog_mx.y };
		lp2 = { lp1.x - prog_height * tri_ratio,lp1.y + prog_height * 0.7f };
		lp3 = { lp1.x + prog_height * tri_ratio,lp1.y + prog_height * 0.7f };
		p->AddTriangleFilled(lp1, lp2, lp3, loop_col);

		if (ImGui::IsMouseHoveringRect({ lp2.x,lp1.y }, { lp3.x,lp3.y }) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !is_draging_tri2)
			is_draging_tri1 = true;
		if (is_draging_tri1)
		{
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || is_draging_tri2)
				is_draging_tri1 = false;
			float cur_mouse_pos = (ImGui::GetMousePos().x - prog_mn.x) / (prog_mx.x - prog_mn.x);
			p_cur_bgm->SetBeginLen(cur_mouse_pos);
			g_player->ResetCurBGM();
		}
		lp1 = { end_x,prog_mx.y };
		lp2 = { lp1.x - prog_height * tri_ratio,lp1.y + prog_height * 0.7f };
		lp3 = { lp1.x + prog_height * tri_ratio,lp1.y + prog_height * 0.7f };
		p->AddTriangleFilled(lp1, lp2, lp3, end_col);
		if (ImGui::IsMouseHoveringRect({ lp2.x,lp1.y }, { lp3.x,lp3.y }) && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !is_draging_tri1)
			is_draging_tri2 = true;
		if (is_draging_tri2)
		{
			if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || is_draging_tri1)
				is_draging_tri2 = false;
			float cur_mouse_pos = (ImGui::GetMousePos().x - prog_mn.x) / (prog_mx.x - prog_mn.x);
			p_cur_bgm->SetBeginLoopLen(cur_mouse_pos);
			g_player->ResetCurBGM();
		}
	}

	ImGui::NewLine();
	// UIs below progress bar
	{
		BGM_Info* bgm_to_change = p_cur_selected ? p_cur_selected : p_cur_bgm;
		static int begin_len = 0, beginloop_len = 0;
		float bgm_to_change_tot_sec = bgm_to_change ? bgm_to_change->GetTotalLen_s() : 0.0f;

		begin_len = bgm_to_change ? bgm_to_change->GetBeginLen() : 0;
		beginloop_len = bgm_to_change ? bgm_to_change->GetBeginLoopLen() : 0;
		static float begin_len_s, beginloop_len_s;
		begin_len_s = bgm_to_change ? bgm_to_change->GetBeginLen_s() : 0.0f;
		beginloop_len_s = bgm_to_change ? bgm_to_change->GetBeginLoopLen_s() : 0.0f;
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 300.0f);
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::InputInt("loop pos(hex)", &begin_len, 1, 100, ImGuiInputTextFlags_CharsHexadecimal) && bgm_to_change) {
			bgm_to_change->SetBeginLen(begin_len);
			if(bgm_to_change==p_cur_bgm)
				g_player->ResetCurBGM();
		}
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::DragFloat("loop pos(second)", &begin_len_s) && bgm_to_change) {
			bgm_to_change->SetBeginLen(begin_len_s / bgm_to_change_tot_sec);
			if (bgm_to_change == p_cur_bgm)
				g_player->ResetCurBGM();
		}
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::InputInt("end pos(hex)", &beginloop_len, 1, 100, ImGuiInputTextFlags_CharsHexadecimal) && bgm_to_change) {
			bgm_to_change->SetBeginLoopLen(beginloop_len);
			if (bgm_to_change == p_cur_bgm)
				g_player->ResetCurBGM();
		}
		ImGui::NextColumn();
		ImGui::SetNextItemWidth(150.0f);
		if (ImGui::DragFloat("end pos(second)", &beginloop_len_s) && bgm_to_change) {
			bgm_to_change->SetBeginLoopLen(beginloop_len_s / bgm_to_change_tot_sec);
			if (bgm_to_change == p_cur_bgm)
				g_player->ResetCurBGM();
		}
		ImGui::Columns(1);
	}
	ImGui::EndDisabled(!enable_play);

	
	ImGui::BeginDisabled(disabled_export);
	{
		if (ImGui::Button("Export")){
			std::wstring ws = LauncherWndFolderSelect(L"folder to export");
			if(!ws.empty())
				dump.ExportBGM(ws.c_str());
		}
		ImGui::SameLine();
		static std::string bgm4all_fmt="";
		if (ImGui::Button("BGM 4 all format")){
			bgm4all_fmt = dump.GetBGM4AllFormat();
			ImGui::OpenPopup("BGM 4 all fmt");
		}
		ImVec2 wndSize = ImGui::GetWindowSize();
		wndSize.x *= 0.8f;
		wndSize.y *= 0.8f;
		ImGui::SetNextWindowSize(wndSize);
		if (ImGui::BeginPopupModal("BGM 4 all fmt",0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImVec2 wndSize = ImGui::GetWindowSize();
			ImGui::InputTextMultiline("##fmt",(char*)bgm4all_fmt.c_str(),bgm4all_fmt.size(),ImVec2(wndSize.x *= 0.9f, wndSize.y *= 0.8f),ImGuiInputTextFlags_ReadOnly);
			if (ImGui::Button("OK", { 100.0f,ImGui::GetFrameHeight() }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Export to .wavs")) {
			ImGui::OpenPopup("Export to .wavs");
		}
		wndSize = ImGui::GetWindowSize();
		wndSize.x *= 0.6f;
		wndSize.y *= 0.6f;
		ImGui::SetNextWindowSize(wndSize);
		if (ImGui::BeginPopupModal("Export to .wavs",0,ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)){
			ImGui::NewLine();
			static BGMWavExportOption opt{true,2,1.0,5.0};
			ImGui::Checkbox("enable ending", &opt.enable_ending);
			if (ImGui::DragInt("loop time", &opt.loop_time,1.0f,1,10))
				opt.loop_time = std::clamp(opt.loop_time, 1, 10);
			ImGui::DragFloat("fade in time in sec", &opt.sec_for_fade_in, 1.0f, 0.0f, 10.0f);
			ImGui::DragFloat("fade out time in sec", &opt.sec_for_fade_out, 1.0f, 0.0f, 10.0f);

			if (ImGui::Button("Export", { 100.0f,ImGui::GetFrameHeight() })){
				std::wstring ws = LauncherWndFolderSelect(L"folder to export");
				if (!ws.empty())
					dump.ExportBGMWavs(ws.c_str(), &opt);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", { 100.0f,ImGui::GetFrameHeight() })) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

	}
	ImGui::EndDisabled(disabled_export);
	ImGui::SameLine();
	{
		ImGui::SetCursorPos({ ImGui::GetWindowSize().x - 100.0f, ImGui::GetCursorPos().y });
		if (ImGui::Button("About", ImVec2(100.0f, ImGui::GetFrameHeight()))) {
			ImGui::OpenPopup("about");
		}
		ImVec2 wndSize = ImGui::GetWindowSize();
		wndSize.x *= 0.8f;
		wndSize.y *= 0.8f;
		ImGui::SetNextWindowSize(wndSize);
		if (ImGui::BeginPopupModal("about",0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			ImVec2 wndSize = ImGui::GetWindowSize();
			static std::u8string about = u8"Author: Rue\nGithub: https://github.com/RUEEE/TH_BGM_Replacer\nDate: 2025/05/19";
			ImGui::InputTextMultiline("##abt", (char*)about.c_str(), about.size(), ImVec2(wndSize.x *= 0.9f, wndSize.y *= 0.8f), ImGuiInputTextFlags_ReadOnly);
			if (ImGui::Button("OK",{100.0f,ImGui::GetFrameHeight()}))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
	return false;
}


int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow)
{
#ifdef _DEBUG
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CON", "r", stdin);
	freopen_s(&stream, "CON", "w", stdout);
#endif

	ImGuiWindowInfo info = { 0 };
	info.className = L"windClass";
	info.title = L"TH BGM replacer(1.1)";
	info.hInstance = hInstance;
	info.initialWidth = 640;
	info.initialHeight = 480;
	info.maxWidth = 1280;
	info.maxHeight = 960;
	info.updateFunc = WindowUpdateFunc;


	auto res = ImGuiWindow::CreateGuiWindow(info);
	if (FAILED(res)) {
		return res;
	}
	g_player = std::make_unique<BGM_Player>();
	g_player->Init();

	//BGM_Dump dump;
	//dump.LoadBGM13(L"C:\\disk\\touhou\\TH15-GZZ\\thbgm.dat",L"C:\\disk\\touhou\\TH15-GZZ\\data\\thbgm.fmt",L"C:\\disk\\touhou\\TH15-GZZ\\data\\musiccmt.txt");
	//std::cout << dump.GetBGM4AllFormat13();
	//g_player->LoadBGM(&dump.bgmInfos[2]);
	//g_player->Play(&dump.bgmInfos[2], 100000);

	ImGuiWindow::GetWindow()->Update();
	return 0;
}
