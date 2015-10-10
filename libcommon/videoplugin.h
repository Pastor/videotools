#pragma once
#include <Windows.h>
#include <vector>

//#include <opencv2/core/core_c.h>
struct _IplImage;
struct CvSeq;

class  Logger;
class  Properties;
struct _VideoPlugin;
struct _VideoPluginStartContext;
struct _VideoPluginFrameContext;

/** */

typedef INT(*pfnLoadPlugin)(struct _VideoPlugin *pluginContext);

typedef INT(*pfnFreePlugin)(struct _VideoPlugin *pluginContext);

typedef INT(*pfnProcessFrame)(struct _VideoPluginFrameContext *frameContext);

typedef INT(*pfnStartProcess)(struct _VideoPluginStartContext *startContext);

typedef INT(*pfnStopProcess)(struct _VideoPluginStartContext *startContext);

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
    pfnProcessFrame  pProcessFrame;

    pfnStartProcess  pStartProcess;
    pfnStopProcess   pStopProcess;
};
typedef struct _VideoPlugin VideoPlugin;
typedef std::vector<VideoPlugin *>  VideoPluginList;

struct _VideoPluginFrameContext
{
    int                  iFrame;
    int                  iQuality;
    struct _IplImage    *frame;
    CvSeq               *seqFaces;
    VideoPlugin         *plugin;
    Properties          *prop;
    Logger              *logger;
};
typedef struct _VideoPluginFrameContext VideoPluginFrameContext;

struct _VideoPluginStartContext
{
    VideoPlugin         *plugin;
    HWND                 hMainWnd;
    LPCSTR               pFileName;
    Properties          *prop;
    int                  iFrameCount;
    int                  iWidth;
    int                  iHeight;
    int                  fps;
};
typedef struct _VideoPluginStartContext VideoPluginStartContext;

void LoadPlugins(LPCTSTR lpstrDirectory, Logger *logger, Properties *prop, VideoPluginList &list);
void FreePlugins(Logger *logger, VideoPluginList &list);