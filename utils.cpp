#include "utils.h"

std::wstring str2wstr(const char* string,UINT codepage)
{
	std::wstring res;
	int wslen = MultiByteToWideChar(codepage, 0, string, -1, NULL, 0);
	res.resize(wslen);
	MultiByteToWideChar(codepage, 0, string, -1, &(res[0]), wslen);
	return res;
}

std::string wstr2str(const WCHAR* wstring, UINT codepage)
{
	std::string res;
	int strlen = WideCharToMultiByte(codepage, 0, wstring, -1, NULL, 0, 0, FALSE);
	res.resize(strlen);
	WideCharToMultiByte(codepage, 0, wstring, -1, &(res[0]), strlen, 0, FALSE);
	return res;
}

std::u8string str2u8(const char* string, UINT codepage)
{
	std::wstring res1 = str2wstr(string, codepage);
	std::u8string res;
	int strlen = WideCharToMultiByte(CP_UTF8, 0, res1.c_str(), -1, NULL, 0, 0, FALSE);
	res.resize(strlen);
	WideCharToMultiByte(CP_UTF8, 0, res1.c_str(), -1, (char*)&(res[0]), strlen, 0, FALSE);
	return res;
}

bool IsKeyPressed(int key)
{
	return GetAsyncKeyState(key)&0x8000;
}
