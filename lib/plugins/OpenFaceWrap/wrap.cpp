#define _CRT_SECURE_NO_WARNINGS
#include <xstring.h>
#include <haar.h>
#include "wrap.h"
#include <opencv2/imgproc.hpp>
#include <LandmarkCoreIncludes.h>
#include <GazeEstimation.h>
#include <atomic>
#include <mutex>

#include <Windows.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")

#pragma warning(disable: 4297)

static DWORD dwCtxIndex;
static DWORD dwProcessId;
static int counter = 0;
static std::mutex _locker;

struct ProcessContext final
{
    LandmarkDetector::FaceModelParameters  det_params;
    LandmarkDetector::CLNF                *clnf_model;
    Point                                 *points;
    float fx = 0;
    float fy = 0;
    float cx = 0;
    float cy = 0;
    bool cx_undefined = true;
    bool fx_undefined = true;
    FILE *fd = nullptr;
};

static void __inline
__Get(struct ProcessContext **pCtx)
{
    (*pCtx) = static_cast<struct ProcessContext *>(TlsGetValue(dwCtxIndex));
}

static __inline void
__InitContext(struct ProcessContext *ctx)
{
    ctx->points = new Point[200];
    fopen_s(&ctx->fd, "wrap.log", "a+");
}

static __inline void
__Destroy(struct ProcessContext *ctx)
{
    std::lock_guard<std::mutex> ignored(_locker);
    if (ctx) {
        if (ctx->clnf_model) {
            ctx->clnf_model->Reset();
            delete ctx->clnf_model;
        }
        ctx->clnf_model = nullptr;
        ctx->cx_undefined = true;
        ctx->fx_undefined = true;
    }
}


static __inline void
__DestroyContext___(struct ProcessContext *ctx)
{
    __try {
        if (ctx) {
            __Destroy(ctx);
            if (ctx->points)
                delete[] ctx->points;
            ctx->points = nullptr;
            if (ctx->fd) {
                fprintf(ctx->fd, "DESTROY\n");
                fflush(ctx->fd);
                fclose(ctx->fd);
            }
            ctx->fd = nullptr;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER)
    {

    }
}

static __inline void
__DestroyContext(struct ProcessContext *ctx)
{
    std::lock_guard<std::mutex> ignored(_locker);
    __DestroyContext___(ctx);
}

void
create_wrap(const char * const modelPath)
{
    ProcessContext *ctx;

    __Get(&ctx);
    __Destroy(ctx);
    ctx->clnf_model = new LandmarkDetector::CLNF(modelPath);
    ctx->det_params = LandmarkDetector::FaceModelParameters();
    ctx->det_params.track_gaze = true;
    fprintf(ctx->fd, "CREATE\n");
    fflush(ctx->fd);
}

#define DEBUG() \
  fprintf(ctx->fd, "%d\n", __LINE__);\
  fflush(ctx->fd);

int
process_wrap(const void *pImage, Point **p, int *nSize)
{
    ProcessContext *ctx;

    __Get(&ctx);


    if (!CV_IS_IMAGE(pImage)) {
        fprintf(ctx->fd, "%d == %d\n", sizeof(IplImage), reinterpret_cast<const IplImage*>(p)->nSize);
        fflush(ctx->fd);
        return -1;
    }

    auto captured_image = cv::cvarrToMat(pImage, true);
    if (ctx->cx_undefined) {
        ctx->cx = captured_image.cols / 2.0f;
        ctx->cy = captured_image.rows / 2.0f;
    }

    if (ctx->fx_undefined) {
        ctx->fx = 500 * (captured_image.cols / 640.0);
        ctx->fy = 500 * (captured_image.rows / 480.0);

        ctx->fx = (ctx->fx + ctx->fy) / 2.0;
        ctx->fy = ctx->fx;
    }

    if (captured_image.cols > 0) {
        char buffer[256];
        cv::Mat_<float> depth_image;
        cv::Mat_<uchar> grayscale_image;
        auto disp_image = captured_image.clone();

        if (captured_image.channels() == 3) {
            cv::cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
        } else {
            grayscale_image = captured_image.clone();
        }
        ++counter;
        // The actual facial landmark detection / tracking
        auto detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, depth_image, *ctx->clnf_model, ctx->det_params);
        auto detection_certainty = (*ctx->clnf_model).detection_certainty;
        // Gaze tracking, absolute gaze direction
        cv::Point3f gazeDirection0(0, 0, -1);
        cv::Point3f gazeDirection1(0, 0, -1);

        if (ctx->det_params.track_gaze && detection_success && (*ctx->clnf_model).eye_model) {
            FaceAnalysis::EstimateGaze((*ctx->clnf_model), gazeDirection0, ctx->fx, ctx->fy, ctx->cx, ctx->cy, true);
            FaceAnalysis::EstimateGaze((*ctx->clnf_model), gazeDirection1, ctx->fx, ctx->fy, ctx->cx, ctx->cy, false);
        }
        (*nSize) = 0;
        (*p) = ctx->points;
        const auto visualisation_boundary = 0.2;
        if (detection_success && detection_certainty < visualisation_boundary) {
            auto n = ctx->clnf_model->detected_landmarks.rows / 2;
            (*nSize) = n;
            for (auto i = 0; i < n; ++i) {
                cv::Point featurePoint(
                    static_cast<int>(ctx->clnf_model->detected_landmarks.at<double>(i)),
                    static_cast<int>(ctx->clnf_model->detected_landmarks.at<double>(i + n)));
                (*p)[i].x = featurePoint.x;
                (*p)[i].y = featurePoint.y;
            }
            return 3;
        }
        return 2;
    }
    return 1;
}

void
destroy_wrap()
{
    ProcessContext *ctx;

    __Get(&ctx);
    __DestroyContext(ctx);
}

BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    LPVOID lpvData;
    BOOL fIgnore;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
    {
        dwProcessId = 0;
        if ((dwCtxIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES)
            return FALSE;
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData == nullptr) {
            lpvData = static_cast<LPVOID>(LocalAlloc(LPTR, sizeof(struct ProcessContext)));
            if (lpvData != nullptr) {
                fIgnore = TlsSetValue(dwCtxIndex, lpvData);
                __InitContext(static_cast<struct ProcessContext *>(lpvData));
            }
        }
        break;
    }

    case DLL_THREAD_ATTACH:
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData == nullptr) {
            lpvData = static_cast<LPVOID>(LocalAlloc(LPTR, sizeof(struct ProcessContext)));
            if (lpvData != nullptr) {
                fIgnore = TlsSetValue(dwCtxIndex, lpvData);
                __InitContext(static_cast<struct ProcessContext *>(lpvData));
            }
        }
        break;

    case DLL_THREAD_DETACH:
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData != nullptr) {
            __DestroyContext(static_cast<struct ProcessContext *>(lpvData));
            LocalFree(static_cast<HLOCAL>(lpvData));
        }
        break;

    case DLL_PROCESS_DETACH:
        dwProcessId = 0;
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData != nullptr) {
            __DestroyContext(static_cast<struct ProcessContext *>(lpvData));
            LocalFree(static_cast<HLOCAL>(lpvData));
        }
        TlsFree(dwCtxIndex);
        break;
    }
    return TRUE;
}





