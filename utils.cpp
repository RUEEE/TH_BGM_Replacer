#include "utils.h"
#include <Shlobj.h>
#pragma warning(default : 4091)


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

std::u8string wstr2u8(const WCHAR* string, UINT codepage)
{
	return str2u8(wstr2str(string,codepage).c_str(),codepage);
}

bool IsKeyPressed(int key)
{
	return GetAsyncKeyState(key)&0x8000;
}



typedef struct {
	ITEMIDLIST* path;
	int attempts;
} initial_path_t;
int CALLBACK SetInitialBrowsePathProc(HWND hWnd, UINT uMsg, [[maybe_unused]] LPARAM lp, LPARAM pData)
{
	initial_path_t* ip = (initial_path_t*)pData;
	if (ip) {
		switch (uMsg) {
		case BFFM_INITIALIZED:
			ip->attempts = 0;
			[[fallthrough]];
		case BFFM_SELCHANGED:
			if (ip->attempts < 2) {
				SendMessageW(hWnd, BFFM_SETSELECTION, FALSE, (LPARAM)ip->path);
				ip->attempts++;
			}
		}
	}
	return 0;
}

template <typename T>
struct ComRAII {
	T* p;

	operator bool() const { return !!p; }
	operator T* () const { return p; }
	T** operator&() { return &p; }
	T* operator->() const { return p; }
	T& operator*() const { return *p; }

	ComRAII()
		: p(nullptr)
	{
	}
	explicit ComRAII(T* p)
		: p(p)
	{
	}
	ComRAII(const ComRAII<T>& other) = delete;
	ComRAII<T>& operator=(const ComRAII<T>& other) = delete;
	~ComRAII()
	{
		if (p) {
			p->Release();
			p = nullptr;
		}
	}
};


static int SelectFolderVista(HWND owner, PIDLIST_ABSOLUTE initial_path, PIDLIST_ABSOLUTE& pidl, const wchar_t* window_title)
{
	// Those two functions are absent in XP, so we have to load them dynamically
	HMODULE shell32 = LoadLibraryW(L"Shell32.dll");
	if (!shell32)
		return -1;
	auto pSHCreateItemFromIDList = (HRESULT(WINAPI*)(PCIDLIST_ABSOLUTE, REFIID, void**))GetProcAddress(shell32, "SHCreateItemFromIDList");
	auto pSHGetIDListFromObject = (HRESULT(WINAPI*)(IUnknown*, PIDLIST_ABSOLUTE*))GetProcAddress(shell32, "SHGetIDListFromObject");
	if (!pSHCreateItemFromIDList || !pSHGetIDListFromObject)
		return -1;

	ComRAII<IFileDialog> pfd;
	if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
		return -1;
	if (!pfd)
		return -1;

	{
		ComRAII<IShellItem> psi;
		pSHCreateItemFromIDList(initial_path, IID_PPV_ARGS(&psi));
		if (!psi)
			return -1;
		pfd->SetDefaultFolder(psi);
	}

	pfd->SetOptions(
		FOS_NOCHANGEDIR | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM
		| FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_DONTADDTORECENT);
	pfd->SetTitle(window_title);
	HRESULT hr = pfd->Show(owner);
	ComRAII<IShellItem> psi;
	if (SUCCEEDED(hr) && SUCCEEDED(pfd->GetResult(&psi))) {
		pSHGetIDListFromObject(psi, &pidl);
		return 0;
	}

	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
		return 0;
	return -1;
}



static int SelectFolderXP(HWND owner, PIDLIST_ABSOLUTE initial_path, PIDLIST_ABSOLUTE& pidl, const wchar_t* window_title)
{
	BROWSEINFOW bi = { 0 };
	initial_path_t ip = { 0 };
	ip.path = initial_path;

	bi.lpszTitle = window_title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON | BIF_USENEWUI;
	bi.hwndOwner = owner;
	bi.lpfn = SetInitialBrowsePathProc;
	bi.lParam = (LPARAM)&ip;
	pidl = SHBrowseForFolderW(&bi);
	return 0;
}
template <typename F>
struct privDefer {
	F f;
	explicit privDefer(F f)
		: f(f)
	{
	}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f)
{
	return privDefer<F>(f);
}
#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) auto DEFER_3(_defer_) = defer_func([&]() { code; })

std::wstring LauncherWndFolderSelect(const wchar_t* title) {
	if (FAILED(CoInitialize(nullptr)))
		return L"";
	defer(CoUninitialize());
	PIDLIST_ABSOLUTE initial_path = nullptr;
	wchar_t path[MAX_PATH] = {};

	GetCurrentDirectoryW(MAX_PATH, path);
	SHParseDisplayName(path, nullptr, &initial_path, 0, nullptr);

	PIDLIST_ABSOLUTE pidl = nullptr;
	if (-1 == SelectFolderVista(NULL, initial_path, pidl, title)) {
		SelectFolderXP(NULL, initial_path, pidl, title);
	}

	if (pidl) {
		defer(CoTaskMemFree(pidl));
		if (SHGetPathFromIDListW(pidl, path)) {
			return path;
		}
	}
	return L"";
}
