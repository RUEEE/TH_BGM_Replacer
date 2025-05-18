#pragma once


#include <Windows.h>
#include <string>


std::wstring str2wstr(const char* string, UINT codepage = CP_ACP);
std::string wstr2str(const WCHAR* wstring, UINT codepage = CP_ACP);
std::u8string str2u8(const char* string, UINT codepage = CP_ACP);

bool IsKeyPressed(int key);