//#include <opencv2/core/types_c.h>
#include <xstring.h>
#include "plugin.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>

INT
LoadPlugin(VideoPlugin *pc)
{
    pc->lpstrPluginName = TEXT("Quality");
    pc->wVersionMajor = 0;
    pc->wVersionMinor = 1;
    pc->pFree = reinterpret_cast<pfnFreePlugin>(FreePlugin);
    pc->pProcessFrame = reinterpret_cast<pfnProcessFrame>(ProcessFrame);
    pc->pStartProcess = reinterpret_cast<pfnStartProcess>(StartProcess);
    pc->pStopProcess = reinterpret_cast<pfnStopProcess>(StopProcess);
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
    cv::Mat mat = cv::cvarrToMat(frameContext->frame);
	
	if (frameContext->seqFaces != nullptr && frameContext->seqFaces->total > 0) {
		/**Лицо найдено в количестве бельше нуля. Берем первый */
		auto rect = reinterpret_cast<CvRect *>(cvGetSeqElem(frameContext->seqFaces, 0));
		auto realRect = cv::Rect(rect->x, rect->y, rect->width, rect->height);
		/** realRect - это область лица */
		/**TODO: А теперь работаем с кадром */
	}    
    return TRUE;
}

INT
StartProcess(VideoPluginStartContext *startContext)
{
    return TRUE;
}

INT
StopProcess(VideoPluginStartContext *startContext)
{
    return TRUE;
}

BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: {
        break;
    }

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


