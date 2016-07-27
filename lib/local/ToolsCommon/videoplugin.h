#pragma once
#include <Windows.h>
#include <vector>

struct _IplImage;
struct CvSeq;

class  Logger;
class  Properties;
struct _VideoPlugin;
struct _VideoPluginContext;

/** */

typedef INT(*pfnLoadPlugin)(struct _VideoPlugin *pluginContext);

typedef INT(*pfnFreePlugin)(struct _VideoPlugin *pluginContext);

typedef INT(*pfnStartProcess)(struct _VideoPluginContext *startContext);

typedef INT(*pfnStopProcess)(struct _VideoPluginContext *startContext);

/** */

struct _VideoPlugin
{
    TCHAR            szPluginPath[MAX_PATH * 2];
    LPCWSTR          lpstrPluginName;
    HMODULE          hLibrary;
    WORD             wVersionMajor;
    WORD             wVersionMinor;
    LPVOID           pUserContext;
    int              isActive;
    DWORD            dwMenuId;
    Properties      *prop;
    Logger          *logger;

    pfnLoadPlugin    pLoad;
    pfnFreePlugin    pFree;
    pfnStartProcess  pStartProcess;
    pfnStopProcess   pStopProcess;
};
typedef struct _VideoPlugin VideoPlugin;
typedef std::vector<VideoPlugin *>  VideoPluginList;

typedef INT(*pfnResetPlugin)(LPWSTR lpwstrPluginName);
typedef INT(*pfnPluginProcess)(INT iFrame);

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

struct _VideoPluginContext
{
    VideoPlugin         *plugin;
    HWND                 hwnd;
    LPCSTR               pFileName;
    LPCSTR               pFileTemplate;
    Properties          *prop;
    DWORD                dwTotalDetectTime;
    DWORD                dwTotalTime;
    int                  iFrameCount;
    int                  iWidth;
    int                  iHeight;
    int                  fps;
    std::atomic_bool    *is_processing;

    /**TODO: Widget work */
    pfnResetPlugin       plugin_reset;
    pfnPluginProcess     plugin_process;

};
typedef struct _VideoPluginContext VideoPluginContext;

void LoadPlugins(LPCTSTR lpstrDirectory, Logger *logger, Properties *prop, VideoPluginList &list);
void FreePlugins(Logger *logger, VideoPluginList &list);

#define WM_FRAME_UPDATE  (WM_USER + 100)
#define WM_FRAME_NEXT    (WM_FRAME_UPDATE + 1)
#define WM_FRAME_STOP    (WM_FRAME_UPDATE + 2)
#define WM_FRAME_FAILED  (WM_FRAME_UPDATE + 3)
#define WM_FRAME_STATUS  (WM_FRAME_UPDATE + 4)
