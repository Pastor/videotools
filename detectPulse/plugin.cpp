#include <strsafe.h>
#include <shlobj.h>
#include <Dbt.h>
#include <shlobj.h>
#include <Shlwapi.h>
#include <xstring.h>
#include <logger.h>
#include <xstring.h>
#include <properties.h>
#include  <system_helper.h>
#include "plugin.h"
#include <haar.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/objdetect/objdetect_c.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/video/tracking_c.h>
#include <opencv2/videoio/videoio_c.h>

#include <unordered_map>
#include <map>

#include "library.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "libcommon.lib")

static DWORD dwCtxIndex;
static DWORD dwProcessId;

struct PulseContext {
    CvMemStorage            *storage;
    Logger                  *logger;
    SessionHandle            hSession;
    SessionConfig            config;
    ProcessResult            result;
    int                      fps;
};

static void __inline
__Get(struct PulseContext **pCtx)
{
    (*pCtx) = static_cast<struct PulseContext *>(TlsGetValue(dwCtxIndex));
}

static void
__FreePulseContext(struct PulseContext *ctx)
{
    if (ctx != nullptr) {
        cvReleaseMemStorage(&(ctx->storage));
        if (ctx->logger != nullptr)
            delete ctx->logger;
        ctx->logger = nullptr;
        if (ctx->hSession != nullptr)
            DestroySession(ctx->hSession);
        ctx->hSession = nullptr;
    }
}

INT
LoadPlugin(VideoPlugin *pc)
{
    pc->lpstrPluginName = TEXT("BloodPump");
    pc->wVersionMajor = 1;
    pc->wVersionMinor = 0;
    pc->pFree = reinterpret_cast<pfnFreePlugin>(FreePlugin);
    pc->pProcessFrame = reinterpret_cast<pfnProcessFrame>(ProcessFrame);
    pc->pStartProcess = reinterpret_cast<pfnStartProcess>(StartProcess);
    pc->pStopProcess = reinterpret_cast<pfnStopProcess>(StopProcess);
    pc->isActive = pc->prop->getBoolean("plugins.pulse.enabled", true);

    return TRUE;
}

INT
FreePlugin(VideoPlugin *pluginContext)
{
    return TRUE;
}

INT
ProcessFrame(VideoPluginFrameContext *frameContext)
{
    struct PulseContext *ctx;

    __Get(&ctx);
    if (frameContext->seqFaces != nullptr && frameContext->seqFaces->total > 0 && ctx->hSession != nullptr) {
        auto ret = ProcessSession(ctx->hSession, frameContext->frame, &ctx->result, 1000 / ctx->fps);
        if (ret) {
            ctx->logger->printf(TEXT("   %08d|  %03d| %03d|"), frameContext->iFrame, static_cast<int>(ctx->result.qValue1), static_cast<int>(ctx->result.qValue2));
        }
    }
    frameContext->iQuality += 0;
    return TRUE;
}

INT
StartProcess(VideoPluginStartContext *startContext)
{
    struct PulseContext *ctx;

    __Get(&ctx);
    {
        ctx->fps = startContext->fps;
        if (ctx->storage == nullptr) {
            ctx->storage = cvCreateMemStorage(0);
        }
        if (ctx->logger == nullptr) {
            auto fileTemplate = std::string(startContext->pFileTemplate);
            ctx->logger = new Logger((fileTemplate + ".pulses").c_str());
            ctx->logger->printf(TEXT("Номер кадра|Пульс| Шум|"));
        }
        if (ctx->hSession == nullptr) {
            auto path = absFilePath("haarcascade_frontalface_alt2.xml");
            ctx->config.iHarmonicBuffSize = 256;
            ctx->config.iHarmonicDataSize = 256;
            strcpy_s(ctx->config.szClassifierFile, sizeof(ctx->config.szClassifierFile), path.c_str());
            ctx->hSession = CreateSession(&ctx->config);
        }
    }
    return TRUE;
}

INT
StopProcess(VideoPluginStartContext *startContext)
{
    struct PulseContext *ctx;

    __Get(&ctx);
    return TRUE;
}

BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    LPVOID lpvData;
    BOOL fIgnore;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        dwProcessId = 0;
        if ((dwCtxIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        break;
    }

    case DLL_THREAD_ATTACH:
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData == nullptr) {
            lpvData = static_cast<LPVOID>(LocalAlloc(LPTR, sizeof(struct PulseContext)));
            if (lpvData != nullptr)
                fIgnore = TlsSetValue(dwCtxIndex, lpvData);
        }
        break;

    case DLL_THREAD_DETACH:
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData != nullptr) {
            __FreePulseContext(static_cast<struct PulseContext *>(lpvData));
            LocalFree(static_cast<HLOCAL>(lpvData));
        }
        break;

    case DLL_PROCESS_DETACH:
        dwProcessId = 0;
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData != nullptr) {
            __FreePulseContext(static_cast<struct PulseContext *>(lpvData));
            LocalFree(static_cast<HLOCAL>(lpvData));
        }
        TlsFree(dwCtxIndex);
        break;
    }
    return TRUE;
}
