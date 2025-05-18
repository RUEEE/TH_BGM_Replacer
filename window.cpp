#include "window.h"
#include "BGM_player.h"
#include "resource.h"

IDirect3D9* g_pD3D;
D3DPRESENT_PARAMETERS g_d3dpp;
IDirect3DDevice9* g_pd3dDevice;


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImGuiWindow* ImGuiWindow::g_Window=nullptr;
bool ImGuiWindow::g_canMove=false;

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_NCHITTEST:
        if (ImGuiWindow::WindCanMove())
            return HTCAPTION; // allows dragging of the window
        else
            break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

HRESULT ImGuiWindow::CreateGuiWindow(ImGuiWindowInfo info)
{
   g_Window= new ImGuiWindow(info);
   ImGuiWindow* pwind = g_Window;
   WNDCLASSW wndClass = {};
    wndClass.style = CS_CLASSDC;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = info.hInstance;
    wndClass.lpszClassName = info.className;
    wndClass.hIcon = LoadIcon(info.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    ATOM wca = RegisterClassW(&wndClass);

    pwind->mHwnd = ::CreateWindow((LPCWSTR)wca, info.title,
        WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 0, 0, info.initialWidth, info.initialHeight,
        NULL, NULL, info.hInstance, NULL);

    SetWindowPos(pwind->mHwnd, HWND_TOP, 320, 240, 0, 0, SWP_NOSIZE);
    if (!pwind->mHwnd)
    {
        return FAIL_TO_CREATE_WINDOW;
    }
    // Initialize Direct3D
    auto d3d9Lib = LoadLibraryW(L"d3d9.dll");
    if (!d3d9Lib) {
        DestroyWindow(pwind->mHwnd);
        return FAIL_TO_LOAD_D3D9;
    }
    decltype(Direct3DCreate9)* d3dCreate9 = (decltype(Direct3DCreate9)*)GetProcAddress(d3d9Lib, "Direct3DCreate9");
    if (!d3dCreate9 || (g_pD3D = d3dCreate9(D3D_SDK_VERSION)) == NULL)
    {
        return FAIL_TO_CREATE_D3D;
    }

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    g_d3dpp.BackBufferWidth = info.maxWidth;
    g_d3dpp.BackBufferHeight = info.maxHeight;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pwind->mHwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return FAIL_TO_CREATE_D3D_DEVICE;

    // Show the window
    ShowWindow(pwind->mHwnd, SW_SHOWDEFAULT);
    UpdateWindow(pwind->mHwnd);

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(pwind->mHwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    // io.IniFilename = nullptr;
    // style.WindowRounding = 0.0f;
    // style.ScrollbarRounding = 0.0f;
    // style.WindowBorderSize = 0.0f;
    // io.ImeWindowHandle = g_hwnd;
    // io.DisplaySize = ImVec2((float)info.renderWidth, (float)(info.renderHeight));
    // io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    // io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    // io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    // io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    // io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    // io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    // io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    // io.KeyMap[ImGuiKey_Home] = VK_HOME;
    // io.KeyMap[ImGuiKey_End] = VK_END;
    // io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    // io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    // io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    // io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    // io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    // io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    // io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    // io.KeyMap[ImGuiKey_A] = 'A';
    // io.KeyMap[ImGuiKey_C] = 'C';
    // io.KeyMap[ImGuiKey_V] = 'V';
    // io.KeyMap[ImGuiKey_X] = 'X';
    // io.KeyMap[ImGuiKey_Y] = 'Y';
    // io.KeyMap[ImGuiKey_Z] = 'Z';
    //io.Fonts->AddFontFromFileTTF("c:\\windows\\fonts\\simhei.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    io.Fonts->AddFontFromFileTTF("c:\\windows\\fonts\\simhei.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    //io.Fonts->AddFontFromFileTTF("c:\\windows\\fonts\\msgothic.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    return S_OK;
}

bool ImGuiWindow::GuiNewFrame()
{
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_QUIT)
            return false;
    }
    ImGuiIO& io = ImGui::GetIO();
    io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;

    //ResizeWindow(__thprac_lc_hwnd, wndPos, wndSize);
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    mCanMove = false;
    return true;
}

bool ImGuiWindow::GuiEndFrame()
{
    ImVec2 wndPos = ImGui::GetWindowPos();
    ImVec2 wndSize = ImGui::GetWindowSize();
    mCanMove &= !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive();
    ImGui::End();

    g_canMove = mCanMove;
    static bool moved = false;

    RECT wndRect;
    GetWindowRect(mHwnd, &wndRect);
    if ((LONG)wndSize.x != wndRect.right - wndRect.left || (LONG)wndSize.y != wndRect.bottom - wndRect.top) {
        moved = true;
        RECT rect = { 0, 0, (LONG)wndSize.x, (LONG)wndSize.y };
        AdjustWindowRectEx(&rect, WS_POPUP, FALSE, WS_EX_APPWINDOW); // Client to Screen
        SetWindowPos(mHwnd, NULL,
            wndRect.left + (LONG)wndPos.x, wndRect.top + (LONG)wndPos.y,
            rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
    GetClientRect(mHwnd, &wndRect);
    RECT renderRect = { (LONG)wndPos.x, (LONG)wndPos.y,
        (LONG)(wndSize.x) + (LONG)(wndPos.x), (LONG)(wndSize.y) + (LONG)(wndPos.y) };

    ImGui::EndFrame();

    // Rendering
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(
        (int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f),
        (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

    if (g_pd3dDevice->BeginScene() >= 0) {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        g_pd3dDevice->EndScene();
    }

    HRESULT result = g_pd3dDevice->Present(&renderRect, NULL, NULL, NULL);
    if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
        if (hr == D3DERR_INVALIDCALL)
            IM_ASSERT(0);
        ImGui_ImplDX9_CreateDeviceObjects();
    }
    if (moved) {
        if (!(GetKeyState(VK_LBUTTON) < 0)) {
            moved = false;
        }
    }
    ImGui::GetIO().DisplaySize = moved ? ImVec2(5000.0f, 5000.0f) : ImVec2(wndSize.x, wndSize.y);

    return true;
}

void ImGuiWindow::Update()
{

    while (GuiNewFrame()) {
        ImGui::SetNextWindowSizeConstraints(ImVec2(640.0f, 480.0f), ImVec2(1280.0f, 960.0f));
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(960.0f, 720.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.9f);

        ImGui::Begin(mTitleImGui.c_str(), &mIsOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove, &mIsMinimize);

        mCanMove = false;
        if (!mIsOpen)
            break;
        if (mIsMinimize) {
            mIsMinimize = false;
            ShowWindow(mHwnd, SW_MINIMIZE);
        }
        mCanMove = ImGui::IsItemHovered();

        //-------
        bool fin = false;
        if (mUpdateFunc){
            fin=mUpdateFunc(this);
        }
        
        GuiEndFrame();
        if (fin)
            break;
    }
}
