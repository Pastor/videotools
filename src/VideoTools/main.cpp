#include <Windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <Shlwapi.h>

#include <logger.h>
#include <videoplugin.h>
#include <haar.h>
#include <atomic>
#include <mutex>

#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/videoio/videoio_c.h>

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

static Logger *gLogger = nullptr;
static Properties *gProp = nullptr;
static VideoPluginList gPlugins;
static std::mutex      gPluginsMutex;
static std::atomic_bool is_running;
static std::atomic_bool is_processing;

INT_PTR CALLBACK
MainDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND                    hMainWnd;
    MSG                     msg;
    DWORD                   newTime;
    DWORD                   oldTime;
    float                   ttlTime;
    float                   delta;
    int                     iFrame;

#if defined(TEST_PLUGIN)
    __TestPlugin();
#endif

    sqlite3_initialize();
    gLogger = new Logger("newtools.log");
    gProp = new Properties();
    gProp->load("settings.properties");
    gLogger->printf(TEXT("������"));
    LoadPlugins(TEXT("plugins"), gLogger, gProp, gPlugins);
    InitCommonControls();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    hMainWnd = CreateDialogParam(hInstance, TEXT("MAINDIALOG"), nullptr, MainDialog, reinterpret_cast<LPARAM>(hInstance));
    ShowWindow(hMainWnd, SW_SHOW);
    ttlTime = 0;
    oldTime = timeGetTime();
    iFrame = 0;
    is_running = true;
    do {
        if (PeekMessage(&msg, hMainWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            Sleep(10);
            newTime = timeGetTime();
            delta = static_cast<float>(newTime - oldTime) / 1000;
            oldTime = newTime;
            ttlTime += delta;
            if (iFrame >= FIX_FPS) {
                SendMessage(hMainWnd, WM_FRAME_UPDATE, 0, 0);
                iFrame = 0;
            } else {
                iFrame++;
            }
        }
    } while (msg.message != WM_QUIT && is_running);
    DestroyWindow(hMainWnd);
    FreePlugins(gLogger, gPlugins);
    gProp->save("settings.properties");
    delete gProp;
    gLogger->printf(TEXT("���������"));
    delete gLogger;
    return EXIT_SUCCESS;
}

static HBITMAP
IplImage2DIB(const IplImage* image)
{
    auto bpp = image->nChannels * 8;
    assert(image->width >= 0 && image->height >= 0 &&
        (bpp == 8 || bpp == 24 || bpp == 32));
    CvMat dst;
    void* dst_ptr = nullptr;
    HBITMAP hbmp = nullptr;
    unsigned char buffer[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
    auto bmi = reinterpret_cast<BITMAPINFO*>(buffer);
    auto bmih = &(bmi->bmiHeader);

    RtlZeroMemory(bmih, sizeof(BITMAPINFOHEADER));
    bmih->biSize = sizeof(BITMAPINFOHEADER);
    bmih->biWidth = image->width;
    bmih->biHeight = image->origin ? abs(image->height) : -abs(image->height);
    bmih->biPlanes = 1;
    bmih->biBitCount = bpp;
    bmih->biCompression = BI_RGB;

    if (bpp == 8) {
        auto palette = bmi->bmiColors;
        int i;
        for (i = 0; i < 256; i++) {
            palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = static_cast<BYTE>(i);
            palette[i].rgbReserved = 0;
        }
    }
    hbmp = CreateDIBSection(nullptr, bmi, DIB_RGB_COLORS, &dst_ptr, 0, 0);
    cvInitMatHeader(&dst, image->height, image->width, CV_8UC3,
        dst_ptr, (image->width * image->nChannels + 3) & -4);
    cvConvertImage(image, &dst, image->origin ? CV_CVTIMG_FLIP : 0);

    return hbmp;
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

static void
__VideoInfo(CvCapture *cap, HWND hMainWnd)
{
    int   ex;
    int   fc;
    HWND  hProgress;

    hProgress = GetDlgItem(hMainWnd, IDC_PROCESS_PROGRESS);
    for (auto id = 1001; id <= 1011; ++id) {
        auto hWnd = GetDlgItem(hMainWnd, id);
        SetWindowText(hWnd, TEXT(""));
        EnableWindow(hWnd, cap == nullptr ? FALSE : TRUE);
    }
    if (cap == nullptr)
        return;
    __WindowPrintf(GetDlgItem(hMainWnd, IDC_INFO_WIDTH),
        TEXT("%d"), static_cast<int>(cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH)));
    __WindowPrintf(GetDlgItem(hMainWnd, IDC_INFO_HEIGHT),
        TEXT("%d"), static_cast<int>(cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT)));
    __WindowPrintf(GetDlgItem(hMainWnd, IDC_INFO_FPS),
        TEXT("%d"), static_cast<int>(cvGetCaptureProperty(cap, CV_CAP_PROP_FPS)));
    fc = static_cast<int>(cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_COUNT));
    __WindowPrintf(GetDlgItem(hMainWnd, IDC_INFO_FRAMES),
        TEXT("%d"), fc);
    ex = static_cast<int>(cvGetCaptureProperty(cap, CV_CAP_PROP_FOURCC));
    __WindowPrintf(GetDlgItem(hMainWnd, IDC_INFO_FORMAT),
        TEXT("%c%c%c%c"),
        static_cast<TCHAR>(ex & 0XFF),
        static_cast<TCHAR>((ex & 0XFF00) >> 8),
        static_cast<TCHAR>((ex & 0XFF0000) >> 16),
        static_cast<TCHAR>((ex & 0XFF000000) >> 24));

    SendMessage(hProgress, PBM_SETSTEP, static_cast<WPARAM>(1), 0);
    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, fc));
}

struct tagProcessFramesParam
{
    CvCapture   *cvCapture;
    Properties  *prop;
    HWND        hMainWnd;
    LPCSTR      pFileName;
    LPCSTR      pFileTemplate;
};
typedef struct tagProcessFramesParam ProcessFramesParam;

static void
__PluginsStart(VideoPluginContext *ctx)
{
    std::lock_guard<std::mutex> locker(gPluginsMutex);

    for (auto it = gPlugins.begin(); it != gPlugins.end() && is_processing; ++it) {
        ctx->plugin = (*it);
        if ((*it)->isActive == TRUE && (*it)->pStartProcess != nullptr && (*it)->pStopProcess != nullptr) {
            SetDlgItemText(ctx->hwnd, IDC_INFO_PLUGIN_NAME, (*it)->lpstrPluginName);
            SetDlgItemText(ctx->hwnd, IDC_INFO_PLUGIN_STATUS, TEXT(""));
            (*(*it)->pStartProcess)(ctx);
            (*(*it)->pStopProcess)(ctx);
        }
    }
    SetDlgItemText(ctx->hwnd, IDC_INFO_PLUGIN_NAME, L"");
    SetDlgItemText(ctx->hwnd, IDC_INFO_PLUGIN_STATUS, TEXT(""));
}

static DWORD
__ProcessFrames(LPVOID pParam)
{
    auto p = static_cast<ProcessFramesParam *>(pParam);
    int        iStopFrame;
    FrameInfo  fi;
    VideoPluginContext ctx = { nullptr, p->hMainWnd };

    RtlSecureZeroMemory(&fi, sizeof(fi));
    is_processing = true;
    iStopFrame = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_COUNT));
    fi.iFrameTotal = iStopFrame;
    fi.dwDetectTime = 0;
    ctx.pFileName = p->pFileName;
    ctx.pFileTemplate = p->pFileTemplate;
    ctx.fps = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FPS));
    ctx.iWidth = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_WIDTH));
    ctx.iHeight = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_HEIGHT));
    ctx.iFrameCount = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_COUNT));
    ctx.prop = gProp;
    ctx.is_processing = &is_processing;
    __PluginsStart(&ctx);
    SendMessage(p->hMainWnd, WM_FRAME_STOP, 0, 0);
    return 0;
}

static void
__UpdatePluginsMenu(HMENU hMainMenu)
{
    std::lock_guard<std::mutex> locker(gPluginsMutex);
    MENUITEMINFO info;

    EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_GRAYED | MF_BYCOMMAND);
    RtlSecureZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_SUBMENU;
    GetMenuItemInfo(hMainMenu, ID_PLUGINS_LOADED, FALSE, &info);
    if (info.hSubMenu != nullptr) {
        DestroyMenu(info.hSubMenu);
        info.hSubMenu = nullptr;
    }
    if (gPlugins.size() > 0) {
        auto id = ID_PLUGINS_START;
        auto hSubMenu = CreateMenu();
        for (auto it = gPlugins.begin(); it != gPlugins.end(); ++it) {
            auto plugin = (*it);
            auto state = MF_CHECKED;
            plugin->dwMenuId = id;
            if (!plugin->isActive)
                state = MF_UNCHECKED;
            AppendMenu(hSubMenu, MF_ENABLED | MF_STRING | state, id, plugin->lpstrPluginName);
            ++id;
        }
        info.hSubMenu = hSubMenu;
        SetMenuItemInfo(hMainMenu, ID_PLUGINS_LOADED, FALSE, &info);
        EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_ENABLED | MF_BYCOMMAND);
    }
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

    static CvCapture *cvCapture = nullptr;
    static CvVideoWriter *writer = nullptr;

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

    static ProcessFramesParam  threadParam;

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
        __VideoInfo(cvCapture, hWnd);
        hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
        SetMenu(hWnd, hMainMenu);
        threadParam.hMainWnd = hWnd;
        threadParam.prop = gProp;
        SetWindowText(hProcessPercent, TEXT(""));
        SetWindowText(hVideoQuality, TEXT(""));

        SendMessage(hProcessPercent, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        SendMessage(hVideoQuality, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        SetDlgItemText(hWnd, IDC_INFO_PLUGIN_NAME, TEXT(""));
        SetDlgItemText(hWnd, IDC_INFO_PLUGIN_STATUS, TEXT(""));
        __UpdatePluginsMenu(hMainMenu);
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
        case ID_PLUGINS_UPDATE:
        {
            __UpdatePluginsMenu(hMainMenu);
            break;
        }
        case ID_FILE_QUIT:
        {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        case ID_FILE_OPEN:
        {
            OPENFILENAMEA ofn;
            if (cvCapture != nullptr)
                cvReleaseCapture(&cvCapture);
            RtlSecureZeroMemory(&ofn, sizeof(ofn));
            RtlSecureZeroMemory(szFileName, sizeof(szFileName));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = sizeof(szFileName);
            ofn.lpstrFilter = "���\0*.*\0���� AVI\0*.avi\0���� MP4\0*.mp4\0";
            ofn.nFilterIndex = 2;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
            if (GetOpenFileNameA(&ofn) == TRUE) {
                cvCapture = cvCreateFileCapture(ofn.lpstrFile);
                __VideoInfo(cvCapture, hWnd);
                StringCchCopyA(szFileTemplate, sizeof(szFileTemplate), szFileName);
                PathRemoveExtensionA(szFileTemplate);
                threadParam.cvCapture = cvCapture;
                threadParam.pFileName = szFileName;
                threadParam.pFileTemplate = szFileTemplate;
                cQualityColor = RGB(178, 34, 34);
                SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("������"));
            }
            EnableWindow(hStartProcess, cvCapture == nullptr ? FALSE : TRUE);
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            break;
        }
        case IDC_START_PROCESS:
        {
            hProcessThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(__ProcessFrames), &threadParam, 0, &dwProcessThreadId);
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
        default:
        {
            if (wCtrlId >= ID_PLUGINS_START) {
                std::lock_guard<std::mutex> locker(gPluginsMutex);

                for (auto it = gPlugins.begin(); it != gPlugins.end(); ++it) {
                    if (wCtrlId == (*it)->dwMenuId) {
                        MENUITEMINFO info;

                        (*it)->isActive = !(*it)->isActive;
                        RtlSecureZeroMemory(&info, sizeof(info));
                        info.cbSize = sizeof(info);
                        info.fMask = MIIM_STATE;
                        info.fState = (*it)->isActive ? MF_CHECKED : MF_UNCHECKED;
                        SetMenuItemInfo(hMainMenu, wCtrlId, FALSE, &info);
                        break;
                    }
                }
            }
            break;
        }
        }
        break;
    }
    case WM_FRAME_STATUS:
    {
        auto status = reinterpret_cast<LPCTSTR>(lParam);
        SetDlgItemText(hWnd, IDC_INFO_PLUGIN_STATUS, status);
        UpdateWindow(GetDlgItem(hWnd, IDC_INFO_PLUGIN_STATUS));
        break;
    }
    case WM_FRAME_NEXT:
    {
        auto fi = reinterpret_cast<FrameInfo *>(lParam);
        auto quality = (fi->iGoodFrames * 100) / fi->iFrameTotal;
        auto tail = fi->iFrameTotal - fi->iFrame;

        __WindowPrintf(hCurrentFrame, TEXT("%d"), fi->iFrame);
        __WindowPrintf(hProcessPercent, TEXT("%d%%"), fi->iProcessPercent);
        __WindowPrintf(hDetectedFaces, TEXT("%d"), fi->iGoodFrames);
        __WindowPrintf(hVideoQuality, TEXT("%d%%"), quality);
        {
            auto total = tail * fi->dwMiddleTime;
            auto sec = total / 1000 % 60;
            auto minutes = total / 1000 / 60 % 60;
            auto hours = total / 1000 / 60 / 60 % 24;
            auto days = total / 1000 / 60 / 60 / 24 % 7;
            __WindowPrintf(GetDlgItem(hWnd, IDC_INFO_VIDEO_TIME), TEXT("%02d:%02d:%02d"), hours, minutes, sec);
        }
        {
            auto sec = fi->dwProcessTime / 1000 % 60;
            auto minutes = fi->dwProcessTime / 1000 / 60 % 60;
            auto hours = fi->dwProcessTime / 1000 / 60 / 60 % 24;
            auto days = fi->dwProcessTime / 1000 / 60 / 60 / 24 % 7;
            __WindowPrintf(GetDlgItem(hWnd, IDC_INFO_VIDEO_TIME_COMPLETE), TEXT("%02d:%02d:%02d"), hours, minutes, sec);
        }
        if (quality < 25) {
            cQualityColor = RGB(178, 34, 34);
        } else if (quality >= 25 && quality < 50) {
            cQualityColor = RGB(255, 165, 0);
        } else {
            cQualityColor = RGB(0, 100, 0);
        }
        UpdateWindow(hVideoQuality);
        SendMessage(hProgress, PBM_SETPOS, fi->iFrame, 0);
        break;
    }
    case WM_FRAME_STOP:
    {
        if (hProcessThread != nullptr)
            CloseHandle(hProcessThread);
        hProcessThread = nullptr;
        dwProcessThreadId = 0;
        EnableWindow(hStartProcess, cvCapture == nullptr ? FALSE : TRUE);
        EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
        SetWindowText(hCurrentFrame, TEXT(""));
        SetWindowText(hProcessPercent, TEXT(""));
        SetWindowText(hVideoQuality, TEXT(""));
        SetWindowText(hDetectedFaces, TEXT(""));
        SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("������"));
        EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_ENABLED | MF_BYCOMMAND);
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
        if (cvCapture != nullptr)
            cvReleaseCapture(&cvCapture);
        if (writer != nullptr)
            cvReleaseVideoWriter(&writer);
        EndDialog(hWnd, lParam);
        PostQuitMessage(0);
        is_running = false;
        return TRUE;
    }
    }
    return FALSE;
}
