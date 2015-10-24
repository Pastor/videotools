#include <Windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <shlobj.h>
#include <Dbt.h>
#include <shlobj.h>
#include <Shlwapi.h>

#include <logger.h>
#include <videoplugin.h>
#include <haar.h>

#include <opencv2/core/core_c.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/objdetect/objdetect_c.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/video/tracking_c.h>
#include <opencv2/videoio/videoio_c.h>

#include "constants.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "libcommon.lib")

#pragma warning(disable: 4996)

#define FIX_FPS           30
#define WM_FRAME_UPDATE  (WM_USER + 100)
#define WM_FRAME_NEXT    (WM_FRAME_UPDATE + 1)
#define WM_FRAME_STOP    (WM_FRAME_UPDATE + 2)
#define WM_FRAME_FAILED  (WM_FRAME_UPDATE + 3)

#define ID_PLUGINS_START  41000

#define OBJECT_MINSIZE   80
#include <properties.h>
#include "../3rdparty/sqlite3/sqlite3.h"

static Logger *gLogger = nullptr;
static Properties *gProp = nullptr;
static VideoPluginList gPlugins;
static std::mutex      gPluginsMutex;

INT_PTR CALLBACK
MainDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//#define TEST_PLUGIN
#if defined(TEST_PLUGIN)
#pragma comment(lib, "detectStasm.lib")
extern "C" {
    __declspec(dllimport)
        INT __CreateProcess(const char *directory);

    __declspec(dllimport)
        INT __FrameProcess(IplImage *iFrame, float landmarks[200], int *iLandmarks);
}

static void
__TestPlugin()
{
    auto i = cvCreateImage(cvSize(2000, 2000), IPL_DEPTH_8U, 1);
    cvSet(i, CV_RGB(255, 0, 0));
    auto ret = __CreateProcess("");
    if (ret) {
        float landmarks[200];
        int   iLandmarks;

        __FrameProcess(i, landmarks, &iLandmarks);
        __asm nop;
    }
    cvReleaseImage(&i);
}

#endif

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
    gLogger = new Logger("videotools.log");
    gProp = new Properties(gLogger);
    gProp->load("settings.properties");
    gLogger->printf(TEXT("Запуск"));
    LoadPlugins(TEXT("Plugins"), gLogger, gProp, gPlugins);
    InitCommonControls();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    //DialogBoxParam(hInstance, TEXT("MAINDIALOG"), NULL, MainDialog, (LPARAM)hInstance);
    hMainWnd = CreateDialogParam(hInstance, TEXT("MAINDIALOG"), nullptr, MainDialog, reinterpret_cast<LPARAM>(hInstance));
    ShowWindow(hMainWnd, SW_SHOW);
    ttlTime = 0;
    oldTime = timeGetTime();
    iFrame = 0;
    do {
        if (PeekMessage(&msg, hMainWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Sleep(10);
            newTime = timeGetTime();
            delta = static_cast<float>(newTime - oldTime) / 1000;
            oldTime = newTime;
            ttlTime += delta;
            if (iFrame >= FIX_FPS) {
                SendMessage(hMainWnd, WM_FRAME_UPDATE, 0, 0);
                iFrame = 0;
            }
            else {
                iFrame++;
            }
        }
    } while (msg.message != WM_QUIT);
    DestroyWindow(hMainWnd);
    FreePlugins(gLogger, gPlugins);
    gProp->save("settings.properties");
    delete gProp;
    gLogger->printf(TEXT("Остановка"));
    delete gLogger;
    return EXIT_SUCCESS;
}

static HBITMAP
IplImage2DIB(const IplImage* Image)
{
    int bpp = Image->nChannels * 8;
    assert(Image->width >= 0 && Image->height >= 0 &&
        (bpp == 8 || bpp == 24 || bpp == 32));
    CvMat dst;
    void* dst_ptr = nullptr;
    HBITMAP hbmp = nullptr;
    unsigned char buffer[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
    auto bmi = reinterpret_cast<BITMAPINFO*>(buffer);
    auto bmih = &(bmi->bmiHeader);

    RtlZeroMemory(bmih, sizeof(BITMAPINFOHEADER));
    bmih->biSize = sizeof(BITMAPINFOHEADER);
    bmih->biWidth = Image->width;
    bmih->biHeight = Image->origin ? abs(Image->height) : -abs(Image->height);
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
    cvInitMatHeader(&dst, Image->height, Image->width, CV_8UC3,
        dst_ptr, (Image->width * Image->nChannels + 3) & -4);
    cvConvertImage(Image, &dst, Image->origin ? CV_CVTIMG_FLIP : 0);

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
        HWND hWnd = GetDlgItem(hMainWnd, id);
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

struct tagFrameInfo
{
    int         iFrame;
    int         iFrameTotal;
    int         iProcessPercent;
    int         iGoodFrames;
    int         iQuality;

    DWORD       dwMiddleTime;
    DWORD       dwProcessTime;
    DWORD       dwDetectTime;
};
typedef struct tagFrameInfo FrameInfo;

struct tagProcessFramesParam
{
    CvCapture   *cvCapture;
    Properties  *prop;
    HWND        hMainWnd;
    LPCSTR      pFileName;
    LPCSTR      pFileTemplate;
};
typedef struct tagProcessFramesParam ProcessFramesParam;
static volatile bool gProcessFramesRuning = false;

static void
__PluginsStart(VideoPluginStartContext *ctx)
{
    std::lock_guard<std::mutex> locker(gPluginsMutex);

    for (auto it = gPlugins.begin(); it != gPlugins.end(); ++it) {
        ctx->plugin = (*it);
        if ((*it)->isActive == TRUE && (*it)->pStartProcess != nullptr)
            (*(*it)->pStartProcess)(ctx);
    }
}

static void
__PluginsStop(VideoPluginStartContext *ctx)
{
    std::lock_guard<std::mutex> locker(gPluginsMutex);

    for (auto it = gPlugins.begin(); it != gPlugins.end(); ++it) {
        ctx->plugin = (*it);
        if ((*it)->isActive == TRUE && (*it)->pStartProcess != nullptr)
            (*(*it)->pStopProcess)(ctx);
    }
}


static void
__PluginsProcessFrame(VideoPluginFrameContext *ctx)
{
    std::lock_guard<std::mutex> locker(gPluginsMutex);

    for (auto it = gPlugins.begin(); it != gPlugins.end(); ++it) {
        if ((*it)->pProcessFrame != nullptr && (*it)->isActive == TRUE) {
            ctx->plugin = (*it);
            (*(*it)->pProcessFrame)(ctx);
        }
    }
    ctx->plugin = nullptr;
}

static DWORD
__ProcessFrames(LPVOID pParam)
{
    auto p = static_cast<ProcessFramesParam *>(pParam);
    IplImage  *i;
    int        iFrame;
    int        iStopFrame;
    int        iFailedFrame;
    FrameInfo  fi;
    DWORD      dwStartTime;
    DWORD      dwStartProcess;
    DWORD      dwStopDetect;
    DWORD      dwAllTime = 0;
#if defined(USE_CPP_HAAR)
    //cv::CascadeClassifier classifier;
#else
    CvHaarClassifierCascade * classifier;
    CvMemStorage* storage;
#endif
    std::vector<cv::Rect>   faces;
    VideoPluginStartContext ctx = { nullptr, p->hMainWnd };
    VideoPluginFrameContext frameCtx;

    iFrame = 0;
    RtlSecureZeroMemory(&fi, sizeof(fi));
    gProcessFramesRuning = true;
    iStopFrame = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_COUNT));
    fi.iFrameTotal = iStopFrame;
    fi.dwDetectTime = 0;
#if defined(USE_CPP_HAAR)
    classifier.load("haarcascade_frontalface_alt2.xml");
    if (classifier.empty()) {
        SendMessage(p->hMainWnd, WM_FRAME_STOP, 0, 0);
        return -1;
    }
#else
    {
        /*TODO: Переписать */
        char szBuffer[1024 + 40];

        GetModuleFileNameA(nullptr, szBuffer, sizeof(szBuffer) - 40);
        PathRemoveFileSpecA(szBuffer);
        PathCombineA(szBuffer, szBuffer, "haarcascade_frontalface_alt2.xml");
        classifier = static_cast<CvHaarClassifierCascade *>(cvLoad(szBuffer));
    }
    if (classifier == nullptr) {
        SendMessage(p->hMainWnd, WM_FRAME_STOP, 0, 0);
        return -1;
    }
    storage = cvCreateMemStorage(0);
#endif
    faces.reserve(1024);
    ctx.pFileName = p->pFileName;
    ctx.pFileTemplate = p->pFileTemplate;
    ctx.fps = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FPS));
    ctx.iWidth = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_WIDTH));
    ctx.iHeight = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_HEIGHT));
    ctx.iFrameCount = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_FRAME_COUNT));
    ctx.prop = gProp;
    __PluginsStart(&ctx);
    dwStartProcess = timeGetTime();
    iFailedFrame = 0;
    SendMessage(p->hMainWnd, WM_FRAME_FAILED, 0, iFailedFrame);
    while ((gProcessFramesRuning) && p->cvCapture != nullptr && iFrame < iStopFrame) {
        if ((i = cvQueryFrame(p->cvCapture)) == nullptr) {
            ++iFrame;
            ++iFailedFrame;
            cvSetCaptureProperty(p->cvCapture, CV_CAP_PROP_POS_FRAMES, iFrame);
            SendMessage(p->hMainWnd, WM_FRAME_FAILED, 0, iFailedFrame);
            continue;
        }
        iFrame = static_cast<int>(cvGetCaptureProperty(p->cvCapture, CV_CAP_PROP_POS_FRAMES));
        frameCtx.iQuality = 0;
        frameCtx.iFrame = iFrame;
        frameCtx.frame = i;
        frameCtx.logger = gLogger;
        fi.iFrame = iFrame;
        fi.iProcessPercent = (iFrame * 100) / iStopFrame;
        dwStartTime = timeGetTime();
#if defined(USE_CPP_HAAR)
        __Detect(classifier, i, faces);
#else
        __Detect(classifier, storage, i, faces, &frameCtx.seqFaces);
#endif
        fi.dwDetectTime += (timeGetTime() - dwStartTime);
        __PluginsProcessFrame(&frameCtx);
        if (faces.size() > 0 && frameCtx.iQuality)
            fi.iGoodFrames++;
        dwAllTime += (timeGetTime() - dwStartTime);
        fi.dwMiddleTime = iFrame > 0 ? (dwAllTime / iFrame) : 0;
        fi.dwProcessTime = timeGetTime() - dwStartProcess;
        SendMessage(p->hMainWnd, WM_FRAME_NEXT, 0, reinterpret_cast<LPARAM>(&fi));
    }
    SendMessage(p->hMainWnd, WM_FRAME_STOP, 0, 0);
    ctx.iFrameGood = fi.iGoodFrames;
    ctx.iFrameProcessed = iFrame - iFailedFrame;
    ctx.dwTotalDetectTime = fi.dwDetectTime;
    ctx.dwTotalTime = dwAllTime;
    __PluginsStop(&ctx);
#if defined(USE_CPP_HAAR)
    //
#else
    cvReleaseMemStorage(&storage);
    cvReleaseHaarClassifierCascade(&classifier);
#endif
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
    static HWND  hFailedFrames = nullptr;

    static HANDLE hProcessThread = nullptr;
    static DWORD  dwProcessThreadId;

    static ProcessFramesParam  threadParam;

    switch (uMsg) {
    case WM_INITDIALOG: {
        hInstance = reinterpret_cast<HINSTANCE>(lParam);

        hProgress = GetDlgItem(hWnd, IDC_PROCESS_PROGRESS);
        hStartProcess = GetDlgItem(hWnd, IDC_START_PROCESS);
        hStopProcess = GetDlgItem(hWnd, IDC_STOP_PROCESS);
        hCurrentFrame = GetDlgItem(hWnd, IDC_INFO_CURRENT_FRAME);
        hProcessPercent = GetDlgItem(hWnd, IDC_PROCESS_PERCENT);
        hDetectedFaces = GetDlgItem(hWnd, IDC_INFO_DETECTED_FACES);
        hVideoQuality = GetDlgItem(hWnd, IDC_INFO_VIDEO_QUALITY);
        hFailedFrames = GetDlgItem(hWnd, IDC_INFO_FAILED_FRAMES);
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
        SetWindowText(hFailedFrames, TEXT(""));

        SendMessage(hProcessPercent, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        SendMessage(hVideoQuality, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        __UpdatePluginsMenu(hMainMenu);
        {
            auto name = gProp->getWString(Constants::ApplicationName);
            if (name.size() > 0) {
                SetWindowText(hWnd, name.c_str());
            }
        }
        return TRUE;
    }
    case WM_NOTIFY: {
        auto wCtrlId = LOWORD(wParam);
        auto lHdr = reinterpret_cast<LPNMHDR>(lParam);
        break;
    }
    case WM_COMMAND: {
        auto wNotifyId = HIWORD(wParam);
        auto wCtrlId = LOWORD(wParam);

        switch (wCtrlId) {
        case IDC_FRAME_BITMAP: {
            if (hBitmap) {
                //DeleteObject(hBitmap);
                //SendDlgItemMessage(hWnd, IDC_FRAME_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
            }
            break;
        }
        case ID_PLUGINS_UPDATE: {
            __UpdatePluginsMenu(hMainMenu);
            break;
        }
        case ID_FILE_QUIT: {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        case ID_FILE_OPEN: {
            //
            OPENFILENAMEA ofn;
            if (cvCapture != nullptr)
                cvReleaseCapture(&cvCapture);
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
                cvCapture = cvCreateFileCapture(ofn.lpstrFile);
                __VideoInfo(cvCapture, hWnd);
                StringCchCopyA(szFileTemplate, sizeof(szFileTemplate), szFileName);
                PathRemoveExtensionA(szFileTemplate);
                threadParam.cvCapture = cvCapture;
                threadParam.pFileName = szFileName;
                threadParam.pFileTemplate = szFileTemplate;
                cQualityColor = RGB(178, 34, 34);
                SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("Запуск"));
            }
            EnableWindow(hStartProcess, cvCapture == nullptr ? FALSE : TRUE);
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            break;
        }
        case IDC_START_PROCESS: {
            hProcessThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(__ProcessFrames), &threadParam, 0, &dwProcessThreadId);
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            EnableWindow(hStartProcess, FALSE);
            EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_DISABLED | MF_BYCOMMAND);
            break;
        }
        case IDC_STOP_PROCESS: {
            gProcessFramesRuning = false;
            EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_ENABLED | MF_BYCOMMAND);
            break;
        }
        default: {
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
    case WM_FRAME_NEXT: {
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
        }
        else if (quality >= 25 && quality < 50) {
            cQualityColor = RGB(255, 165, 0);
        }
        else {
            cQualityColor = RGB(0, 100, 0);
        }
        UpdateWindow(hVideoQuality);
        SendMessage(hProgress, PBM_SETPOS, fi->iFrame, 0);
        break;
    }
    case WM_FRAME_STOP: {
        if (hProcessThread != nullptr)
            CloseHandle(hProcessThread);
        hProcessThread = nullptr;
        dwProcessThreadId = 0;
        EnableWindow(hStartProcess, cvCapture == nullptr ? FALSE : TRUE);
        EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
        SetWindowText(hCurrentFrame, TEXT(""));
        SetWindowText(hProcessPercent, TEXT(""));
        //SetWindowText(hDetectedFaces, TEXT(""));
        SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("Запуск"));
        if (cvCapture != nullptr) {
            auto iCurrent = static_cast<int>(cvGetCaptureProperty(cvCapture, CV_CAP_PROP_POS_FRAMES));
            auto iFrames = static_cast<int>(cvGetCaptureProperty(cvCapture, CV_CAP_PROP_FRAME_COUNT));

            if (iCurrent >= iFrames) {
                cvSetCaptureProperty(cvCapture, CV_CAP_PROP_POS_FRAMES, 0);
            }
            else {
                SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("Продолжить"));
            }
        }
        EnableMenuItem(hMainMenu, ID_PLUGINS_LOADED, MF_ENABLED | MF_BYCOMMAND);
        break;
    }
    case WM_FRAME_FAILED: {
        int iFailedFrame = lParam;
        __WindowPrintf(hFailedFrames, TEXT("%d"), iFailedFrame);
        UpdateWindow(hFailedFrames);
        break;
    }
    case WM_FRAME_UPDATE: {
        /*IplImage *frame;
        if (cvCapture) {
        HWND hBitmapWindows = GetDlgItem(hWnd, IDC_FRAME_BITMAP);
        frame = cvQueryFrame(cvCapture);
        if (frame) {
        IplImage *result;
        RECT      rect;

        GetClientRect(hWnd, &rect);
        result = cvCreateImage(cvSize(rect.right, rect.bottom), frame->depth, frame->nChannels);
        if (hBitmap)
        DeleteObject(hBitmap);
        hBitmap = NULL;
        cvResize(frame, result, CV_INTER_LINEAR);
        hBitmap = IplImage2DIB(result);
        SendDlgItemMessage(hWnd, IDC_FRAME_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
        UpdateWindow(hBitmapWindows);
        if (writer)
        cvWriteFrame(writer, frame);
        cvReleaseImage(&result);
        }
        }*/
        break;
    }
    case WM_CTLCOLORSTATIC: {
        auto hdc = reinterpret_cast<HDC>(wParam);
        if (reinterpret_cast<HWND>(lParam) == nullptr) {
            SetTextColor(hdc, cStatusColor);
            SetBkColor(hdc, cDefColor);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(hDefBrush);
        }
        else if (reinterpret_cast<HWND>(lParam) == hVideoQuality) {
            SetTextColor(hdc, cQualityColor);
            SetBkColor(hdc, cDefColor);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(hDefBrush);
        }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    case WM_CLOSE: {
        if (hProcessThread != nullptr) {
            gProcessFramesRuning = false;
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
        return TRUE;
    }
    }
    return FALSE;
}
