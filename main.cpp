#include "window.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <format>
#include <imgui.h>
#include <memory>
#include "BGM_def.h"
#include "BGM_player.h"

std::unique_ptr<BGM_Player> g_player;
HANDLE g_hevent_loopbgm;

void BGM_for_th13()
{
	static BGM_Dump dump;
	static int cur_select = -1;
	static WCHAR dat_file[MAX_PATH] = L"C:\\disk\\touhou\\TH15-GZZ\\thbgm.dat";
	static WCHAR fmt_file[MAX_PATH] = L"C:\\disk\\touhou\\TH15-GZZ\\data\\thbgm.fmt";
	static WCHAR cmt_file[MAX_PATH] = L"C:\\disk\\touhou\\TH15-GZZ\\data\\musiccmt.txt";

	// load file
	if (ImGui::Button("Load")) {
		ImGui::OpenPopup("load files");
	}
	ImVec2 wndSize = ImGui::GetWindowSize();
	wndSize.x *= 0.8f;
	wndSize.y *= 0.8f;
	ImGui::SetNextWindowSize(wndSize);
	if (ImGui::BeginPopupModal("load files"))
	{
		ImGui::Columns(2);
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
		}
		ImGui::NextColumn();
		ImGui::Text("%ws", dat_file);
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
		}
		ImGui::NextColumn();
		ImGui::Text("%ws", fmt_file);
		ImGui::NextColumn();
		if (ImGui::Button("Select music.cmt(optional)"))
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
		ImGui::Text("%ws", cmt_file);
		ImGui::NextColumn();
		ImGui::Columns(1);
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
			dump.LoadBGM13(dat_file, fmt_file, cmt_file);
			cur_select = -1;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// bgm lists
	// id (name) begin loop end total WAVfile
	if (ImGui::BeginTable("bgm list", 7, ImGuiTableFlags_::ImGuiTableFlags_Borders | ImGuiTableFlags_::ImGuiTableFlags_Resizable | ImGuiTableFlags_::ImGuiTableFlags_ScrollY, 
		{ ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y*0.6f}))
	{
		float sz = ImGui::GetWindowSize().x;
		float size_text = ImGui::CalcTextSize("AAA").x;
		DWORD flag = ImGuiTableColumnFlags_WidthStretch;
		DWORD flag2 = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
		ImGui::TableSetupColumn("id", flag2, size_text);
		size_text = ImGui::CalcTextSize("NAMENAMENAMENAME").x;
		ImGui::TableSetupColumn("name", flag, size_text);
		size_text = ImGui::CalcTextSize("0x12345678").x;
		ImGui::TableSetupColumn("begin", flag2, size_text);
		size_text = ImGui::CalcTextSize("0x12345678/114.51s").x;
		ImGui::TableSetupColumn("loop len", flag2, size_text);
		ImGui::TableSetupColumn("endloop len", flag2, size_text);
		ImGui::TableSetupColumn("total len", flag2, size_text);
		size_text = ImGui::CalcTextSize("C:/ABCD/EFGH/IJKL").x;
		ImGui::TableSetupColumn("file", flag, size_text);
		ImGui::TableHeadersRow();

		for (int i = 0; i < dump.bgmInfos.size(); i++)
		{
			auto& it = dump.bgmInfos[i];
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (ImGui::Selectable(std::format("{}", i).c_str(), cur_select == i, ImGuiSelectableFlags_::ImGuiSelectableFlags_SpanAllColumns))
				cur_select = i;
			ImGui::TableNextColumn();
			if(it.bgmName.empty())
				ImGui::Text("%s", it.idName.c_str());
			else
				ImGui::Text("%s",str2u8(it.bgmName.c_str(), 932).c_str());
			//begin
			ImGui::TableNextColumn();
			ImGui::Text("%08X",it.GetEnterPos());
			//loop
			ImGui::TableNextColumn();
			ImGui::Text("%08X(%.2lfs)", it.GetLoopLen(),it.GetWavSec(it.GetLoopLen()));
			//endloop
			ImGui::TableNextColumn();
			ImGui::Text("%08X(%.2lfs)", it.GetEnterLoopLen(), it.GetWavSec(it.GetEnterLoopLen()));
			//total
			ImGui::TableNextColumn();
			ImGui::Text("%08X(%.2lfs)", it.GetTotalLen(), it.GetWavSec(it.GetTotalLen()));
			//file
			ImGui::TableNextColumn();
			ImGui::Text("%ws", it.type == DAT_FILE ? it.BGMFilePath_dat.c_str() : it.BGMFilePath_wav.c_str());
		}
		ImGui::EndTable();
	}
	ImGui::Separator();
	ImGui::BeginDisabled(cur_select == -1);
	if (ImGui::Button("Play"))
	{
		g_player->LoadBGM(&dump.bgmInfos[cur_select], g_hevent_loopbgm);
	}
	ImGui::Button("Stop");
	ImGui::Button("Pause");
	ImGui::EndDisabled();

}


bool WindowUpdateFunc(ImGuiWindow* pWind)
{
	if(ImGui::BeginTabBar("tb")){
		if (ImGui::BeginTabItem("bgm for th13 and later"))
		{
			BGM_for_th13();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
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
	info.title = L"wind";
	info.hInstance = hInstance;
	info.initialWidth = 640;
	info.initialHeight = 480;
	info.maxWidth = 1280;
	info.maxHeight = 960;
	info.updateFunc = WindowUpdateFunc;

	g_hevent_loopbgm =  CreateEvent(NULL,false,false,L"BGM_loop");

	auto res = ImGuiWindow::CreateGuiWindow(info);
	if (FAILED(res)) {
		return res;
	}
	g_player = std::make_unique<BGM_Player>(ImGuiWindow::GetWindow()->GetHWND());
	BGM_Dump dump;
	dump.LoadBGM13(L"C:\\disk\\touhou\\TH15-GZZ\\thbgm.dat",L"C:\\disk\\touhou\\TH15-GZZ\\data\\thbgm.fmt",L"C:\\disk\\touhou\\TH15-GZZ\\data\\musiccmt.txt");
	dump.bgmInfos[2].enterLen = 300000;
	dump.bgmInfos[2].enterloopLen = 400000;
	std::cout << dump.GetBGM4AllFormat13();
	g_player->LoadBGM(&dump.bgmInfos[2], g_hevent_loopbgm);
	g_player->Play();

	ImGuiWindow::GetWindow()->Update();
	return 0;
}
