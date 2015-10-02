//#include <opencv2/core/types_c.h>
#include <xstring.h>
#include "plugin.h"
#include "stasm_lib.h"
#include "stasm.h"
#include <opencv2/imgcodecs/imgcodecs_c.h>

typedef std::vector<stasm::DetPar> vec_DetPar;

void __cdecl __Set(std::vector<stasm::DetPar>& faces, int idx);
void __cdecl __Set(IplImage *pImage);

INT
LoadPlugin(VideoPlugin *pc)
{
    pc->lpstrPluginName = TEXT("Stasm");
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
    int         foundface;
    float       landmarks[2 * stasm_NLANDMARKS];
    vec_DetPar  detpars;
    auto        leftborder = 0;
    auto        topborder = 0;

    if (frameContext == nullptr || frameContext->seqFaces == nullptr)
        return FALSE;
    if (frameContext->seqFaces->total == 0)
        return TRUE;
    detpars.resize(frameContext->seqFaces->total);
    for (auto i = 0; i < frameContext->seqFaces->total; i++) {
        auto facerect = reinterpret_cast<CvRect *>(cvGetSeqElem(frameContext->seqFaces, i));
        stasm::DetPar detpar; // detpar constructor sets all fields INVALID
        // detpar.x and detpar.y is the center of the face rectangle
        detpar.x = facerect->x + facerect->width / 2.;
        detpar.y = facerect->y + facerect->height / 2.;
        detpar.x -= leftborder; // discount the border we added earlier
        detpar.y -= topborder;
        detpar.width = double(facerect->width);
        detpar.height = double(facerect->height);
        detpar.yaw = 0; // assume face has no yaw in this version of Stasm
        detpar.eyaw = stasm::EYAW00;
        detpars[i] = detpar;
    }
    __Set(detpars, 0);
    __Set(frameContext->frame);

    if (!stasm_search_auto_ext(&foundface, landmarks, nullptr))
        return FALSE;
    {
        char szBuffer[256];
        auto dest = cvCloneImage(frameContext->frame);
        auto rect = cvRect(0, 0, 0, 0);

        //cvSet(dest, CV_RGB(192, 192, 192));
        
        sprintf(szBuffer, "D:\\GitHub\\bioapp\\videotools\\Catched\\cat%08d.jpeg", frameContext->iFrame);
        for (auto i = 0; i < frameContext->seqFaces->total; i++) {
            auto facerect = reinterpret_cast<CvRect *>(cvGetSeqElem(frameContext->seqFaces, i));
            rect = *facerect;
            cvRectangle(dest, cvPoint(facerect->x, facerect->y), cvPoint(facerect->x + facerect->width, facerect->y + facerect->height), CV_RGB(255, 255, 0), 1);
            break;
        }
        for (auto i = 0; i < 2 * stasm_NLANDMARKS && landmarks[i] != 0.; i += 2) {
            auto x = landmarks[i];
            auto y = landmarks[i + 1];
            cvCircle(dest, cvPoint(x, y), 1, CV_RGB(255, 0, 0));
        }
        cvSetImageROI(dest, rect);
        cvSaveImage(szBuffer, dest);
        cvReleaseImage(&dest);
    }
    return foundface == TRUE;
}

INT
StartProcess(VideoPluginStartContext *startContext)
{
    auto basedir = std::toString(startContext->plugin->szPluginPath);
    auto ret = stasm_init(basedir.c_str(), FALSE);
    return ret == TRUE;
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


