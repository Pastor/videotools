#include <Windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <shlobj.h>
#include <Dbt.h>
#include <shlobj.h>
#include <Shlwapi.h>

#include <logger.h>

#include "constants.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")


#pragma warning(disable: 4996)

#define FIX_FPS           30

#define ID_PLUGINS_START  41000

#define OBJECT_MINSIZE   80
#include <properties.h>
#include <sqlite3.h>
#include <sqlite.hpp>

static Logger *gLogger = nullptr;
static Properties *gProp = nullptr;
static std::atomic_bool is_running;
static std::atomic_bool is_processing;

INT_PTR CALLBACK
MainDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND                    hMainWnd;
    MSG                     msg;

    sqlite3_initialize();
    gLogger = new Logger("newtools.log");
    gProp = new Properties(gLogger);
    gProp->load("settings.properties");
    gLogger->printf(TEXT("Запуск"));
    InitCommonControls();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    //DialogBoxParam(hInstance, TEXT("MAINDIALOG"), NULL, MainDialog, (LPARAM)hInstance);
    hMainWnd = CreateDialogParam(hInstance, TEXT("MAINDIALOG"), nullptr, MainDialog, reinterpret_cast<LPARAM>(hInstance));
    ShowWindow(hMainWnd, SW_SHOW);
    is_running = true;
    do {
        if (PeekMessage(&msg, hMainWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (msg.message != WM_QUIT && is_running);
    DestroyWindow(hMainWnd);
    gProp->save("settings.properties");
    delete gProp;
    gLogger->printf(TEXT("Остановка"));
    delete gLogger;
    return EXIT_SUCCESS;
}


static void
__WindowPrintf(HWND hWnd, LPCTSTR lpstrFormat, ...)
{
    va_list args;
    int     ret;
    LPTSTR  lpPrepared;
    auto    len = std::wcslen(lpstrFormat) * 4;

    lpPrepared = static_cast<LPWSTR>(std::malloc(len * sizeof(WCHAR)));
    va_start(args, lpstrFormat);
    ret = vswprintf(lpPrepared, len, lpstrFormat, args);
    va_end(args);
    lpPrepared[ret] = static_cast<TCHAR>(0);
    SetWindowText(hWnd, lpPrepared);
    std::free(lpPrepared);
}


INT_PTR CALLBACK
MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CHAR  szFileName[MAX_PATH * 2];
    static CHAR  szFileTemplate[MAX_PATH * 2];
    static HINSTANCE hInstance = nullptr;
    static HFONT hFont = nullptr;
    static auto  cDefColor = RGB(240, 240, 240);
    static auto  cStatusColor = RGB(0, 0, 0);
    static auto  cQualityColor = RGB(0, 0, 0);
    static HBRUSH hDefBrush = nullptr;

    static HBITMAP hBitmap = nullptr;

    static HMENU hMainMenu = nullptr;
    static HWND  hProgress = nullptr;
    static HWND  hStartProcess = nullptr;
    static HWND  hStopProcess = nullptr;
    static HWND  hCurrentFrame = nullptr;
    static HWND  hProcessPercent = nullptr;
    static HWND  hDetectedFaces = nullptr;
    static HWND  hVideoQuality = nullptr;

    static HANDLE hProcessThread = nullptr;
    static DWORD  dwProcessThreadId;

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        hInstance = reinterpret_cast<HINSTANCE>(lParam);

        hProgress = GetDlgItem(hWnd, IDC_PROCESS_PROGRESS);
        hStartProcess = GetDlgItem(hWnd, IDC_START_PROCESS);
        hStopProcess = GetDlgItem(hWnd, IDC_STOP_PROCESS);
        hCurrentFrame = GetDlgItem(hWnd, IDC_INFO_CURRENT_FRAME);
        hProcessPercent = GetDlgItem(hWnd, IDC_PROCESS_PERCENT);
        hDetectedFaces = GetDlgItem(hWnd, IDC_INFO_DETECTED_FACES);
        hVideoQuality = GetDlgItem(hWnd, IDC_INFO_VIDEO_QUALITY);
        hDefBrush = CreateSolidBrush(cDefColor);
        hFont = CreateFont(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("Lucida Console"));
        hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
        SetMenu(hWnd, hMainMenu);
        SetWindowText(hProcessPercent, TEXT(""));
        SetWindowText(hVideoQuality, TEXT(""));

        SendMessage(hProcessPercent, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        SendMessage(hVideoQuality, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        SetDlgItemText(hWnd, IDC_INFO_PLUGIN_NAME, TEXT(""));
        SetDlgItemText(hWnd, IDC_INFO_PLUGIN_STATUS, TEXT(""));
        {
            auto name = gProp->getWString(Constants::ApplicationName);
            if (name.size() > 0) {
                SetWindowText(hWnd, name.c_str());
            }
        }
        return TRUE;
    }
    case WM_NOTIFY:
    {
        auto wCtrlId = LOWORD(wParam);
        auto lHdr = reinterpret_cast<LPNMHDR>(lParam);
        break;
    }
    case WM_COMMAND:
    {
        auto wNotifyId = HIWORD(wParam);
        auto wCtrlId = LOWORD(wParam);

        switch (wCtrlId) {
        case IDC_FRAME_BITMAP:
        {
            if (hBitmap) {
                //DeleteObject(hBitmap);
                //SendDlgItemMessage(hWnd, IDC_FRAME_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
            }
            break;
        }
        case ID_PLUGINS_UPDATE:
        {
            break;
        }
        case ID_FILE_QUIT:
        {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        case ID_FILE_OPEN:
        {
            //
            OPENFILENAMEA ofn;
            RtlSecureZeroMemory(&ofn, sizeof(ofn));
            RtlSecureZeroMemory(szFileName, sizeof(szFileName));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = sizeof(szFileName);
            ofn.lpstrFilter = "Все\0*.*\0Файл AVI\0*.avi\0Файл MP4\0*.mp4\0";
            ofn.nFilterIndex = 2;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
            if (GetOpenFileNameA(&ofn) == TRUE) {
                StringCchCopyA(szFileTemplate, sizeof(szFileTemplate), szFileName);
                PathRemoveExtensionA(szFileTemplate);
                cQualityColor = RGB(178, 34, 34);
                SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("Запуск"));
            }
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            break;
        }
        case IDC_START_PROCESS:
        {
//            hProcessThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(__ProcessFrames), &threadParam, 0, &dwProcessThreadId);
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            EnableWindow(hStartProcess, FALSE);
            EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_DISABLED | MF_BYCOMMAND);
            break;
        }
        case IDC_STOP_PROCESS:
        {
            is_processing = false;
            EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_ENABLED | MF_BYCOMMAND);
            break;
        }
        
        }
        break;
    }
    
    case WM_CTLCOLORSTATIC:
    {
        auto hdc = reinterpret_cast<HDC>(wParam);
        if (reinterpret_cast<HWND>(lParam) == nullptr) {
            SetTextColor(hdc, cStatusColor);
            SetBkColor(hdc, cDefColor);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(hDefBrush);
        } else if (reinterpret_cast<HWND>(lParam) == hVideoQuality) {
            SetTextColor(hdc, cQualityColor);
            SetBkColor(hdc, cDefColor);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(hDefBrush);
        }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    case WM_CLOSE:
    {
        if (hProcessThread != nullptr) {
            is_processing = false;
            Sleep(1000);
            CloseHandle(hProcessThread);
            hProcessThread = nullptr;
        }
        EndDialog(hWnd, lParam);
        PostQuitMessage(0);
        is_running = false;
        return TRUE;
    }
    }
    return FALSE;
}
