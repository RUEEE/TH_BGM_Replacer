#pragma once
#include <Windows.h>
#include <d3d9.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>
#include <cstdint>
#include <string>
#include "utils.h"

#define FAIL_TO_CREATE_WINDOW -1
#define FAIL_TO_LOAD_D3D9 -2
#define FAIL_TO_CREATE_D3D -3
#define FAIL_TO_CREATE_D3D_DEVICE -4

class ImGuiWindow;
typedef bool (*ImGuiWindowUpdateFunc)(ImGuiWindow* pWind);

struct ImGuiWindowInfo
{
	HINSTANCE hInstance;
	LPCWSTR className;
	LPCWSTR title;
	uint32_t initialWidth;
	uint32_t initialHeight;
	uint32_t maxWidth;
	uint32_t maxHeight;
	ImGuiWindowUpdateFunc updateFunc;
};

class ImGuiWindow
{
	HINSTANCE mInstance;
	LPCWSTR mClassName;

	std::wstring mTitle;
	std::string mTitleImGui;

	uint32_t mInitialWidth;
	uint32_t mInitialHight;
	uint32_t mRenderWidth;
	uint32_t mRenderHeight;
	HWND mHwnd;

	bool mIsOpen;
	bool mIsMinimize;
	bool mCanMove;

	ImGuiWindowUpdateFunc mUpdateFunc;
private:
	ImGuiWindow(ImGuiWindowInfo info) :
		mInstance(info.hInstance), mClassName(info.className), mTitle(std::wstring(info.title)), mInitialHight(info.initialHeight), mInitialWidth(info.initialWidth),
		mRenderHeight(info.maxHeight), mRenderWidth(info.maxWidth), mUpdateFunc(info.updateFunc), mHwnd(NULL)
	{
		mIsOpen = true;
		mIsMinimize = false;
		mCanMove = false;

		mTitleImGui = wstr2str(mTitle.c_str());

	};

	bool GuiNewFrame();
	bool GuiEndFrame();

private:
	static ImGuiWindow* g_Window;
	static bool g_canMove;
public:
	static HRESULT CreateGuiWindow(ImGuiWindowInfo info);
	static ImGuiWindow* GetWindow() { return g_Window; };
	static bool WindCanMove() { return g_canMove; }

public:
	void Update();
	HWND GetHWND() { return mHwnd; }
};
