#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

// Control & Menu IDs
#define ID_TRAY_ICON             1001
#define WM_TRAYNOTIFY            (WM_USER + 1)
#define ID_HOTKEY_TOGGLE         2001
#define ID_TIMER_TOPMOST         3001

#define IDM_TITLE_HEADER         4000
#define IDM_TOGGLE_SHOW          4001

#define IDM_PRESET_DEFAULT       4010
#define IDM_PRESET_EVALUATION    4011
#define IDM_PRESET_COPILOT       4012
#define IDM_PRESET_LINUX         4013
#define IDM_PRESET_GRASS         4014
#define IDM_PRESET_VIETNAMESE    4015
#define IDM_PRESET_CUSTOM        4016

#define IDM_OPACITY_30           4020
#define IDM_OPACITY_45           4021
#define IDM_OPACITY_60           4022
#define IDM_OPACITY_80           4023
#define IDM_OPACITY_100          4024

#define IDM_POS_BOTTOM_RIGHT     4030
#define IDM_POS_BOTTOM_LEFT      4031
#define IDM_POS_TOP_RIGHT        4032
#define IDM_POS_TOP_LEFT         4033

#define IDM_MONITOR_PRIMARY      4040
#define IDM_MONITOR_ALL          4041

#define IDM_EXIT                 4099

enum WatermarkPosition {
    POS_BOTTOM_RIGHT = 0,
    POS_BOTTOM_LEFT  = 1,
    POS_TOP_RIGHT    = 2,
    POS_TOP_LEFT     = 3
};

struct MonitorData {
    HMONITOR hMon;
    RECT rcWork;
    RECT rcMonitor;
    bool isPrimary;
};

// Global App State
HINSTANCE g_hInstance = NULL;
std::vector<HWND> g_watermarkWindows;
HWND g_hwndController = NULL;
NOTIFYICONDATAW g_nid = { 0 };
ULONG_PTR g_gdiplusToken = 0;

std::wstring g_title = L"Activate Windows";
std::wstring g_subtitle = L"Go to Settings to activate Windows.";
float g_opacityFactor = 0.60f; // Authentic ~60% alpha
bool g_isVisible = true;
bool g_showTray = true;
bool g_allMonitors = false;
int g_currentPreset = 0;
WatermarkPosition g_posMode = POS_BOTTOM_RIGHT;

// Forward declarations
void RenderAllWatermarks();
void UpdateWindowsList();
void ShowCustomDialog(HWND hParent);
HICON CreateWatermarkIcon();

// DPI Awareness
void EnablePerMonitorDpi() {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL (WINAPI *SetProcessDpiAwarenessContextProc)(HANDLE);
        SetProcessDpiAwarenessContextProc pSet = 
            (SetProcessDpiAwarenessContextProc)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
        if (pSet) {
            pSet((HANDLE)-4); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
            return;
        }
    }
    SetProcessDPIAware();
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM lParam) {
    auto* list = reinterpret_cast<std::vector<MonitorData>*>(lParam);
    MONITORINFO mi = { sizeof(MONITORINFO) };
    if (GetMonitorInfoW(hMon, &mi)) {
        MonitorData data;
        data.hMon = hMon;
        data.rcWork = mi.rcWork;
        data.rcMonitor = mi.rcMonitor;
        data.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
        list->push_back(data);
    }
    return TRUE;
}

std::vector<MonitorData> GetTargetMonitors() {
    std::vector<MonitorData> allMons;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&allMons));

    if (g_allMonitors) {
        return allMons;
    }

    std::vector<MonitorData> primaryOnly;
    for (const auto& m : allMons) {
        if (m.isPrimary) {
            primaryOnly.push_back(m);
            return primaryOnly;
        }
    }
    if (!allMons.empty()) {
        primaryOnly.push_back(allMons[0]);
    }
    return primaryOnly;
}

// Window procedure for transparent watermark windows
LRESULT CALLBACK WatermarkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_NCHITTEST:
            return HTTRANSPARENT; // Mouse clicks pass completely through!
        case WM_DESTROY:
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// Render watermark onto a specific window and monitor
void RenderSingleWatermark(HWND hwnd, const MonitorData& mon) {
    if (!hwnd) return;

    if (!g_isVisible) {
        ShowWindow(hwnd, SW_HIDE);
        return;
    }

    // Determine scale for this window / monitor
    float scale = 1.0f;
    HDC hdcMon = GetDC(hwnd);
    int dpiX = GetDeviceCaps(hdcMon, LOGPIXELSX);
    ReleaseDC(hwnd, hdcMon);
    scale = (float)dpiX / 96.0f;
    if (scale < 1.0f) scale = 1.0f;

    // Use Segoe UI (Standard Windows watermark font)
    std::unique_ptr<Gdiplus::FontFamily> fontFamily(new Gdiplus::FontFamily(L"Segoe UI"));
    if (!fontFamily->IsAvailable()) {
        fontFamily.reset(new Gdiplus::FontFamily(L"Segoe UI Variable Text"));
        if (!fontFamily->IsAvailable()) {
            fontFamily.reset(new Gdiplus::FontFamily(L"Arial"));
        }
    }

    // Font sizes in pixels: Title ~20px, Subtitle ~14px (scales with DPI)
    float fontSizeTitle = 20.0f * scale;
    float fontSizeSub   = 14.0f * scale;
    float lineSpacing   = 5.0f * scale;
    float padding       = 10.0f * scale;

    Gdiplus::Font fontTitle(fontFamily.get(), fontSizeTitle, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::Font fontSub(fontFamily.get(), fontSizeSub, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

    // Measure text
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMeasure = CreateCompatibleDC(hdcScreen);
    Gdiplus::Graphics gMeasure(hdcMeasure);

    Gdiplus::RectF boundTitle, boundSub;
    Gdiplus::PointF origin(0.0f, 0.0f);
    gMeasure.MeasureString(g_title.c_str(), -1, &fontTitle, origin, &boundTitle);
    gMeasure.MeasureString(g_subtitle.c_str(), -1, &fontSub, origin, &boundSub);

    DeleteDC(hdcMeasure);
    ReleaseDC(NULL, hdcScreen);

    float textWidth = std::max(boundTitle.Width, boundSub.Width);
    int winWidth = (int)(textWidth + padding * 2 + 10.0f);
    int winHeight = (int)(boundTitle.Height + boundSub.Height + lineSpacing + padding * 2);

    if (winWidth < 120) winWidth = 120;
    if (winHeight < 50) winHeight = 50;

    // Create 32-bit DIB Section
    BITMAPINFO bi = { 0 };
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = winWidth;
    bi.bmiHeader.biHeight = -winHeight; // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    HDC hdcTarget = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcTarget);
    void* pBits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // Render text with GDI+
    {
        Gdiplus::Bitmap gdiBmp(winWidth, winHeight, winWidth * 4, PixelFormat32bppARGB, (BYTE*)pBits);
        Gdiplus::Graphics g(&gdiBmp);

        g.Clear(Gdiplus::Color(0, 0, 0, 0));
        g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

        // Alpha calculation: authentic alpha ratio
        int aTitle = (int)(255 * g_opacityFactor);
        int aSub   = (int)(255 * g_opacityFactor * 0.88f);
        if (aTitle > 255) aTitle = 255;
        if (aTitle < 0) aTitle = 0;
        if (aSub > 255) aSub = 255;
        if (aSub < 0) aSub = 0;

        Gdiplus::SolidBrush brushTitle(Gdiplus::Color((BYTE)aTitle, 255, 255, 255));
        Gdiplus::SolidBrush brushSub(Gdiplus::Color((BYTE)aSub, 255, 255, 255));

        Gdiplus::PointF ptTitle(padding, padding);
        g.DrawString(g_title.c_str(), -1, &fontTitle, ptTitle, &brushTitle);

        Gdiplus::PointF ptSub(padding, padding + boundTitle.Height + lineSpacing);
        g.DrawString(g_subtitle.c_str(), -1, &fontSub, ptSub, &brushSub);

        // Alpha premultiplication required by UpdateLayeredWindow
        BYTE* p = (BYTE*)pBits;
        int totalPixels = winWidth * winHeight;
        for (int i = 0; i < totalPixels; ++i) {
            BYTE a = p[3];
            if (a > 0) {
                p[0] = (BYTE)((p[0] * a) / 255);
                p[1] = (BYTE)((p[1] * a) / 255);
                p[2] = (BYTE)((p[2] * a) / 255);
            }
            p += 4;
        }
    }

    // Margins from monitor edge / taskbar
    int marginX = (int)(40 * scale);
    int marginY = (int)(25 * scale);

    int posX = 0, posY = 0;
    switch (g_posMode) {
        case POS_BOTTOM_RIGHT:
            posX = mon.rcWork.right - winWidth - marginX;
            posY = mon.rcWork.bottom - winHeight - marginY;
            break;
        case POS_BOTTOM_LEFT:
            posX = mon.rcWork.left + marginX;
            posY = mon.rcWork.bottom - winHeight - marginY;
            break;
        case POS_TOP_RIGHT:
            posX = mon.rcWork.right - winWidth - marginX;
            posY = mon.rcWork.top + marginY;
            break;
        case POS_TOP_LEFT:
            posX = mon.rcWork.left + marginX;
            posY = mon.rcWork.top + marginY;
            break;
    }

    POINT ptSrc = { 0, 0 };
    POINT ptDst = { posX, posY };
    SIZE sizeWnd = { winWidth, winHeight };

    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    UpdateLayeredWindow(hwnd, hdcTarget, &ptDst, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcTarget);

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

// Synchronize watermark windows count with target monitors
void UpdateWindowsList() {
    auto monitors = GetTargetMonitors();

    // Destroy extra windows if monitors reduced
    while (g_watermarkWindows.size() > monitors.size()) {
        HWND h = g_watermarkWindows.back();
        DestroyWindow(h);
        g_watermarkWindows.pop_back();
    }

    // Create new windows if needed
    while (g_watermarkWindows.size() < monitors.size()) {
        HWND hwnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            L"ActivateWindowsWatermarkClass",
            L"Activate Windows",
            WS_POPUP,
            0, 0, 400, 100,
            NULL, NULL, g_hInstance, NULL
        );
        g_watermarkWindows.push_back(hwnd);
    }
}

void RenderAllWatermarks() {
    UpdateWindowsList();
    auto monitors = GetTargetMonitors();
    for (size_t i = 0; i < monitors.size() && i < g_watermarkWindows.size(); ++i) {
        RenderSingleWatermark(g_watermarkWindows[i], monitors[i]);
    }
}

// Programmatically create crisp Windows logo tray icon
HICON CreateWatermarkIcon() {
    int cx = GetSystemMetrics(SM_CXSMICON);
    int cy = GetSystemMetrics(SM_CYSMICON);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcColor = CreateCompatibleDC(hdcScreen);
    HDC hdcMask  = CreateCompatibleDC(hdcScreen);

    HBITMAP hbmColor = CreateCompatibleBitmap(hdcScreen, cx, cy);
    HBITMAP hbmMask  = CreateBitmap(cx, cy, 1, 1, NULL);

    HBITMAP hOldColor = (HBITMAP)SelectObject(hdcColor, hbmColor);
    HBITMAP hOldMask  = (HBITMAP)SelectObject(hdcMask, hbmMask);

    HBRUSH hbrBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
    HBRUSH hbrWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RECT rc = { 0, 0, cx, cy };
    FillRect(hdcColor, &rc, hbrBlack);
    FillRect(hdcMask, &rc, hbrWhite);

    // Windows Cyan Blue
    HBRUSH hbrTile = CreateSolidBrush(RGB(0, 164, 239));

    int gap = (cx >= 32) ? 3 : 2;
    int pad = (cx >= 32) ? 4 : 2;
    int tileW = (cx - pad * 2 - gap) / 2;
    int tileH = (cy - pad * 2 - gap) / 2;

    RECT r1 = { pad, pad, pad + tileW, pad + tileH };
    RECT r2 = { pad + tileW + gap, pad, pad + tileW * 2 + gap, pad + tileH };
    RECT r3 = { pad, pad + tileH + gap, pad + tileW, pad + tileH * 2 + gap };
    RECT r4 = { pad + tileW + gap, pad + tileH + gap, pad + tileW * 2 + gap, pad + tileH * 2 + gap };

    FillRect(hdcColor, &r1, hbrTile);
    FillRect(hdcColor, &r2, hbrTile);
    FillRect(hdcColor, &r3, hbrTile);
    FillRect(hdcColor, &r4, hbrTile);

    FillRect(hdcMask, &r1, hbrBlack);
    FillRect(hdcMask, &r2, hbrBlack);
    FillRect(hdcMask, &r3, hbrBlack);
    FillRect(hdcMask, &r4, hbrBlack);

    SelectObject(hdcColor, hOldColor);
    SelectObject(hdcMask, hOldMask);
    DeleteDC(hdcColor);
    DeleteDC(hdcMask);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(hbrTile);

    ICONINFO ii = { 0 };
    ii.fIcon = TRUE;
    ii.hbmColor = hbmColor;
    ii.hbmMask = hbmMask;

    HICON hIcon = CreateIconIndirect(&ii);

    DeleteObject(hbmColor);
    DeleteObject(hbmMask);

    return hIcon;
}

// Show Tray Context Menu
void ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    HMENU hMenuPresets = CreatePopupMenu();
    HMENU hMenuOpacity = CreatePopupMenu();
    HMENU hMenuPosition = CreatePopupMenu();
    HMENU hMenuMonitors = CreatePopupMenu();

    // Bold title header
    AppendMenuW(hMenu, MF_STRING | MF_DISABLED, IDM_TITLE_HEADER, L"Activate Windows (Prank Tool)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    // Toggle watermark visibility
    UINT showFlag = g_isVisible ? (MF_STRING | MF_CHECKED) : (MF_STRING | MF_UNCHECKED);
    AppendMenuW(hMenu, showFlag, IDM_TOGGLE_SHOW, L"&Show Watermark\tCtrl+Alt+W");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    // Presets Submenu
    AppendMenuW(hMenuPresets, (g_currentPreset == 0 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_DEFAULT, L"1. Activate Windows (Default)");
    AppendMenuW(hMenuPresets, (g_currentPreset == 1 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_EVALUATION, L"2. Windows 11 Enterprise (Evaluation Copy)");
    AppendMenuW(hMenuPresets, (g_currentPreset == 2 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_COPILOT, L"3. Activate Copilot ($30/mo Needed)");
    AppendMenuW(hMenuPresets, (g_currentPreset == 3 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_LINUX, L"4. Activate Linux (Arch btw)");
    AppendMenuW(hMenuPresets, (g_currentPreset == 4 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_GRASS, L"5. Touch Grass (Activate Real Life)");
    AppendMenuW(hMenuPresets, (g_currentPreset == 5 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_VIETNAMESE, L"6. K\u00edch ho\u1ea1t Windows (Ti\u1ebfng Vi\u1ec7t)");
    AppendMenuW(hMenuPresets, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenuPresets, (g_currentPreset == 6 ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_PRESET_CUSTOM, L"7. Custom Text...");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuPresets, L"🎭 Watermark &Presets");

    // Opacity Submenu
    AppendMenuW(hMenuOpacity, (g_opacityFactor == 0.30f ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_OPACITY_30, L"30% (Faint)");
    AppendMenuW(hMenuOpacity, (g_opacityFactor == 0.45f ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_OPACITY_45, L"45% (Subtle)");
    AppendMenuW(hMenuOpacity, (g_opacityFactor == 0.60f ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_OPACITY_60, L"60% (Authentic - Default)");
    AppendMenuW(hMenuOpacity, (g_opacityFactor == 0.80f ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_OPACITY_80, L"80% (Clear)");
    AppendMenuW(hMenuOpacity, (g_opacityFactor == 1.00f ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_OPACITY_100, L"100% (Solid White)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuOpacity, L"🔆 &Opacity");

    // Position Submenu
    AppendMenuW(hMenuPosition, (g_posMode == POS_BOTTOM_RIGHT ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_POS_BOTTOM_RIGHT, L"Bottom-Right (Classic)");
    AppendMenuW(hMenuPosition, (g_posMode == POS_BOTTOM_LEFT ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_POS_BOTTOM_LEFT, L"Bottom-Left");
    AppendMenuW(hMenuPosition, (g_posMode == POS_TOP_RIGHT ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_POS_TOP_RIGHT, L"Top-Right");
    AppendMenuW(hMenuPosition, (g_posMode == POS_TOP_LEFT ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_POS_TOP_LEFT, L"Top-Left");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuPosition, L"📍 &Position");

    // Monitor Submenu
    AppendMenuW(hMenuMonitors, (!g_allMonitors ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_MONITOR_PRIMARY, L"Primary Monitor Only");
    AppendMenuW(hMenuMonitors, (g_allMonitors ? MF_CHECKED : MF_UNCHECKED) | MF_STRING, 
                IDM_MONITOR_ALL, L"All Connected Monitors");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMenuMonitors, L"🖥️ &Displays");

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"❌ &Exit");

    // Make header menu item bold
    MENUITEMINFOW mii = { sizeof(mii) };
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DEFAULT;
    SetMenuItemInfoW(hMenu, IDM_TITLE_HEADER, FALSE, &mii);

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);

    DestroyMenu(hMenuPresets);
    DestroyMenu(hMenuOpacity);
    DestroyMenu(hMenuPosition);
    DestroyMenu(hMenuMonitors);
    DestroyMenu(hMenu);
}

// Custom Text Dialog
HWND g_hEditTitle = NULL;
HWND g_hEditSub = NULL;

LRESULT CALLBACK CustomDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            HWND hLbl1 = CreateWindowW(L"STATIC", L"Header (Line 1):", 
                WS_CHILD | WS_VISIBLE, 15, 15, 310, 20, hwnd, NULL, g_hInstance, NULL);
            SendMessageW(hLbl1, WM_SETFONT, (WPARAM)hFont, TRUE);

            g_hEditTitle = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", g_title.c_str(), 
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 15, 38, 310, 24, hwnd, (HMENU)101, g_hInstance, NULL);
            SendMessageW(g_hEditTitle, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hLbl2 = CreateWindowW(L"STATIC", L"Subtitle (Line 2):", 
                WS_CHILD | WS_VISIBLE, 15, 75, 310, 20, hwnd, NULL, g_hInstance, NULL);
            SendMessageW(hLbl2, WM_SETFONT, (WPARAM)hFont, TRUE);

            g_hEditSub = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", g_subtitle.c_str(), 
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 15, 98, 310, 24, hwnd, (HMENU)102, g_hInstance, NULL);
            SendMessageW(g_hEditSub, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hBtnOk = CreateWindowW(L"BUTTON", L"Apply", 
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 140, 140, 85, 28, hwnd, (HMENU)IDOK, g_hInstance, NULL);
            SendMessageW(hBtnOk, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hBtnCancel = CreateWindowW(L"BUTTON", L"Cancel", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 240, 140, 85, 28, hwnd, (HMENU)IDCANCEL, g_hInstance, NULL);
            SendMessageW(hBtnCancel, WM_SETFONT, (WPARAM)hFont, TRUE);

            SetFocus(g_hEditTitle);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDOK) {
                wchar_t bufTitle[256] = { 0 };
                wchar_t bufSub[256] = { 0 };
                GetWindowTextW(g_hEditTitle, bufTitle, 255);
                GetWindowTextW(g_hEditSub, bufSub, 255);

                g_title = bufTitle;
                g_subtitle = bufSub;
                g_currentPreset = 6; // Custom
                RenderAllWatermarks();
                DestroyWindow(hwnd);
            } else if (LOWORD(wParam) == IDCANCEL) {
                DestroyWindow(hwnd);
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void ShowCustomDialog(HWND hParent) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = CustomDialogProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = L"WatermarkCustomDlgClass";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    int dlgW = 355;
    int dlgH = 220;
    RECT rcScreen;
    GetWindowRect(GetDesktopWindow(), &rcScreen);
    int x = (rcScreen.right - dlgW) / 2;
    int y = (rcScreen.bottom - dlgH) / 2;

    HWND hDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"WatermarkCustomDlgClass",
        L"Custom Watermark Text",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, dlgW, dlgH,
        hParent, NULL, g_hInstance, NULL
    );
    UpdateWindow(hDlg);
}

// Controller Window Procedure
LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_TRAYNOTIFY:
            if (lParam == WM_RBUTTONUP) {
                ShowContextMenu(hwnd);
            } else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP) {
                g_isVisible = !g_isVisible;
                RenderAllWatermarks();
            }
            return 0;

        case WM_HOTKEY:
            if (wParam == ID_HOTKEY_TOGGLE) {
                g_isVisible = !g_isVisible;
                RenderAllWatermarks();
            }
            return 0;

        case WM_TIMER:
            if (wParam == ID_TIMER_TOPMOST) {
                if (g_isVisible) {
                    for (HWND h : g_watermarkWindows) {
                        SetWindowPos(h, HWND_TOPMOST, 0, 0, 0, 0, 
                                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    }
                }
            }
            return 0;

        case WM_DISPLAYCHANGE:
        case WM_SETTINGCHANGE:
            RenderAllWatermarks();
            return 0;

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDM_TOGGLE_SHOW:
                    g_isVisible = !g_isVisible;
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_DEFAULT:
                    g_currentPreset = 0;
                    g_title = L"Activate Windows";
                    g_subtitle = L"Go to Settings to activate Windows.";
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_EVALUATION:
                    g_currentPreset = 1;
                    g_title = L"Windows 11 Enterprise";
                    g_subtitle = L"Evaluation copy. Build 26100.ge_release";
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_COPILOT:
                    g_currentPreset = 2;
                    g_title = L"Activate Copilot";
                    g_subtitle = L"Go to Settings to buy Copilot Pro.";
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_LINUX:
                    g_currentPreset = 3;
                    g_title = L"Activate Linux";
                    g_subtitle = L"Go to Settings to install Arch Linux btw.";
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_GRASS:
                    g_currentPreset = 4;
                    g_title = L"Activate Real Life";
                    g_subtitle = L"Go outside and touch some grass.";
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_VIETNAMESE:
                    g_currentPreset = 5;
                    g_title = L"K\u00edch ho\u1ea1t Windows";
                    g_subtitle = L"V\u00e0o C\u00e0i \u0111\u1eb7t \u0111\u1ec3 k\u00edch ho\u1ea1t Windows.";
                    RenderAllWatermarks();
                    break;

                case IDM_PRESET_CUSTOM:
                    ShowCustomDialog(hwnd);
                    break;

                case IDM_OPACITY_30:
                    g_opacityFactor = 0.30f;
                    RenderAllWatermarks();
                    break;
                case IDM_OPACITY_45:
                    g_opacityFactor = 0.45f;
                    RenderAllWatermarks();
                    break;
                case IDM_OPACITY_60:
                    g_opacityFactor = 0.60f;
                    RenderAllWatermarks();
                    break;
                case IDM_OPACITY_80:
                    g_opacityFactor = 0.80f;
                    RenderAllWatermarks();
                    break;
                case IDM_OPACITY_100:
                    g_opacityFactor = 1.00f;
                    RenderAllWatermarks();
                    break;

                case IDM_POS_BOTTOM_RIGHT:
                    g_posMode = POS_BOTTOM_RIGHT;
                    RenderAllWatermarks();
                    break;
                case IDM_POS_BOTTOM_LEFT:
                    g_posMode = POS_BOTTOM_LEFT;
                    RenderAllWatermarks();
                    break;
                case IDM_POS_TOP_RIGHT:
                    g_posMode = POS_TOP_RIGHT;
                    RenderAllWatermarks();
                    break;
                case IDM_POS_TOP_LEFT:
                    g_posMode = POS_TOP_LEFT;
                    RenderAllWatermarks();
                    break;

                case IDM_MONITOR_PRIMARY:
                    g_allMonitors = false;
                    RenderAllWatermarks();
                    break;
                case IDM_MONITOR_ALL:
                    g_allMonitors = true;
                    RenderAllWatermarks();
                    break;

                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            return 0;
        }

        case WM_DESTROY:
            Shell_NotifyIconW(NIM_DELETE, &g_nid);
            UnregisterHotKey(hwnd, ID_HOTKEY_TOGGLE);
            KillTimer(hwnd, ID_TIMER_TOPMOST);
            for (HWND h : g_watermarkWindows) {
                DestroyWindow(h);
            }
            g_watermarkWindows.clear();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// Entry Point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    g_hInstance = hInstance;
    EnablePerMonitorDpi();

    // Parse command line arguments
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        for (int i = 1; i < argc; ++i) {
            std::wstring arg = argv[i];
            if (arg == L"--silent" || arg == L"-s") {
                g_showTray = false;
            } else if (arg == L"--all-monitors" || arg == L"-m") {
                g_allMonitors = true;
            } else if (arg == L"--preset" && i + 1 < argc) {
                int p = _wtoi(argv[++i]);
                if (p == 1) {
                    g_currentPreset = 1;
                    g_title = L"Windows 11 Enterprise";
                    g_subtitle = L"Evaluation copy. Build 26100.ge_release";
                } else if (p == 2) {
                    g_currentPreset = 2;
                    g_title = L"Activate Copilot";
                    g_subtitle = L"Go to Settings to buy Copilot Pro.";
                } else if (p == 3) {
                    g_currentPreset = 3;
                    g_title = L"Activate Linux";
                    g_subtitle = L"Go to Settings to install Arch Linux btw.";
                } else if (p == 4) {
                    g_currentPreset = 4;
                    g_title = L"Activate Real Life";
                    g_subtitle = L"Go outside and touch some grass.";
                } else if (p == 5) {
                    g_currentPreset = 5;
                    g_title = L"K\u00edch ho\u1ea1t Windows";
                    g_subtitle = L"V\u00e0o C\u00e0i \u0111\u1eb7t \u0111\u1ec3 k\u00edch ho\u1ea1t Windows.";
                }
            } else if (arg == L"--title" && i + 1 < argc) {
                g_title = argv[++i];
                g_currentPreset = 6;
            } else if (arg == L"--subtitle" && i + 1 < argc) {
                g_subtitle = argv[++i];
                g_currentPreset = 6;
            } else if (arg == L"--opacity" && i + 1 < argc) {
                int op = _wtoi(argv[++i]);
                if (op > 0 && op <= 100) g_opacityFactor = op / 100.0f;
            }
        }
        LocalFree(argv);
    }

    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    if (Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL) != Gdiplus::Ok) {
        return 1;
    }

    // Register Watermark Window Class
    WNDCLASSEXW wcx = { 0 };
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = WatermarkWndProc;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.lpszClassName = L"ActivateWindowsWatermarkClass";
    RegisterClassExW(&wcx);

    // Register Controller Window Class
    WNDCLASSEXW wcxTray = { 0 };
    wcxTray.cbSize = sizeof(wcxTray);
    wcxTray.lpfnWndProc = TrayWndProc;
    wcxTray.hInstance = hInstance;
    wcxTray.lpszClassName = L"ActivateWindowsTrayControllerClass";
    RegisterClassExW(&wcxTray);

    // Create Controller Message Window
    g_hwndController = CreateWindowExW(
        0, L"ActivateWindowsTrayControllerClass", L"WatermarkController",
        0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL
    );

    // Register Global Hotkey: Ctrl + Alt + W
    RegisterHotKey(g_hwndController, ID_HOTKEY_TOGGLE, MOD_CONTROL | MOD_ALT, 'W');

    // Timer to enforce topmost (every 2 seconds)
    SetTimer(g_hwndController, ID_TIMER_TOPMOST, 2000, NULL);

    // Setup System Tray Icon
    if (g_showTray) {
        g_nid.cbSize = sizeof(g_nid);
        g_nid.hWnd = g_hwndController;
        g_nid.uID = ID_TRAY_ICON;
        g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        g_nid.uCallbackMessage = WM_TRAYNOTIFY;
        g_nid.hIcon = CreateWatermarkIcon();
        wcscpy_s(g_nid.szTip, L"Activate Windows Watermark (Prank)");
        Shell_NotifyIconW(NIM_ADD, &g_nid);
    }

    // Render Initial Watermark
    RenderAllWatermarks();

    // Message Loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_showTray && g_nid.hIcon) {
        DestroyIcon(g_nid.hIcon);
    }

    Gdiplus::GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}
