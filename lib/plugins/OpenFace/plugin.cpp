#define _CRT_SECURE_NO_WARNINGS
#include <xstring.h>
#include <logger.h>
#include <properties.h>
#include <xstring.h>
#include  <system_helper.h>
#include <haar.h>
#include "plugin.h"
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <LandmarkCoreIncludes.h>
#include <GazeEstimation.h>
#include <fstream>
#include <sstream>

#include <Windows.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")

#pragma warning(disable: 4297)

static DWORD dwCtxIndex;
static DWORD dwProcessId;

struct ProcessContext
{
    CvHaarClassifierCascade *classifier;
    CvMemStorage            *storage;
    CvSeq                   *seqFaces;
    Logger                  *logger;
};

static void __inline
__Get(struct ProcessContext **pCtx)
{
    (*pCtx) = static_cast<struct ProcessContext *>(TlsGetValue(dwCtxIndex));
}

static __inline void
__InitContext(struct ProcessContext *ctx)
{
//    char buffer[256];
//    auto pid = GetCurrentProcessId();
//
//    wsprintfA(buffer, "openface.%05d.log", pid);
//    ctx->logger = new Logger(buffer);
//    ctx->logger->printf("START");
}

static __inline void
__DestroyContext(struct ProcessContext *ctx)
{
    __try {
        if (ctx != nullptr && ctx->classifier != nullptr)
            cvReleaseHaarClassifierCascade(&(ctx->classifier));
        if (ctx != nullptr && ctx->storage != nullptr)
            cvReleaseMemStorage(&(ctx->storage));
        if (ctx != nullptr && ctx->logger != nullptr) {
            ctx->logger->printf("DESTROY");
            delete ctx->logger;
        }
        ctx->logger = nullptr;
    } __except (EXCEPTION_EXECUTE_HANDLER)
    {

    }
}

static void
NonOverlapingDetections(const vector<LandmarkDetector::CLNF>& clnf_models, vector<cv::Rect_<double> >& face_detections)
{

    for (size_t model = 0; model < clnf_models.size(); ++model) {
        auto clnf = clnf_models[model];
        if (clnf.detected_landmarks.rows == 0)
            continue;
        auto model_rect = clnf.GetBoundingBox();
        for (int detection = face_detections.size() - 1; detection >= 0; --detection) {
            auto intersection_area = (model_rect & face_detections[detection]).area();
            auto union_area = model_rect.area() + face_detections[detection].area() - 2 * intersection_area;
            if (intersection_area / union_area > 0.5) {
                face_detections.erase(face_detections.begin() + detection);
            }
        }
    }
}

double fps_tracker = -1.0;
int64 t0 = 0;
void
write_result(cv::Mat& captured_image,
    cv::Mat_<float>& depth_image,
    const LandmarkDetector::CLNF& face_model,
    const LandmarkDetector::FaceModelParameters& det_parameters,
    cv::Point3f gazeDirection0,
    cv::Point3f gazeDirection1,
    int frame_count,
    double fx,
    double fy,
    double cx,
    double cy)
{

    // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
    auto detection_certainty = face_model.detection_certainty;
    auto detection_success = face_model.detection_success;

    auto visualisation_boundary = 0.2;

    // Only draw if the reliability is reasonable, the value is slightly ad-hoc
    if (detection_certainty < visualisation_boundary) {
        LandmarkDetector::Draw(captured_image, face_model);

        auto vis_certainty = detection_certainty;
        if (vis_certainty > 1)
            vis_certainty = 1;
        if (vis_certainty < -1)
            vis_certainty = -1;

        vis_certainty = (vis_certainty + 1) / (visualisation_boundary + 1);

        // A rough heuristic for box around the face width
        int thickness = static_cast<int>(std::ceil(2.0* static_cast<double>(captured_image.cols) / 640.0));

        cv::Vec6d pose_estimate_to_draw = LandmarkDetector::GetCorrectedPoseWorld(face_model, fx, fy, cx, cy);

        // Draw it in reddish if uncertain, blueish if certain
        LandmarkDetector::DrawBox(captured_image, pose_estimate_to_draw, cv::Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy);

        if (det_parameters.track_gaze && detection_success && face_model.eye_model) {
            FaceAnalysis::DrawGaze(captured_image, face_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
        }
    }

    // Work out the framerate
    if (frame_count % 10 == 0) {
        double t1 = cv::getTickCount();
        fps_tracker = 10.0 / (double(t1 - t0) / cv::getTickFrequency());
        t0 = t1;
    }
}

static INT
StartProcess(VideoPluginContext *ctx)
{
    FrameInfo fi = { 0, ctx->iFrameCount, 0, 0, 0, 0, 0, 0 };
    struct ProcessContext *pctx;
    auto it = 0;
    DWORD dwAllTime = 0;
    auto ldots = std::toString(ctx->pFileTemplate) + xstring(TEXT(".ldots"));
    auto wdots = std::toString(ctx->pFileTemplate) + xstring(TEXT(".avi"));
    auto startProcess = timeGetTime();
    __Get(&pctx);
    SendMessage(ctx->hwnd, WM_FRAME_STATUS, NULL, (LONG_PTR)TEXT("Подготовка моделей"));
    LandmarkDetector::FaceModelParameters det_params;
    det_params.track_gaze = true;
    LandmarkDetector::CLNF clnf_model(ctx->prop->getString("plugins.open_face.model", "model/main_clnf_general.txt"));
    float fx = 0, fy = 0, cx = 0, cy = 0;
    bool cx_undefined = false;
    bool fx_undefined = false;
    if (cx == 0 || cy == 0) {
        cx_undefined = true;
    }
    if (fx == 0 || fy == 0) {
        fx_undefined = true;
    }
    auto video_capture = cv::VideoCapture(ctx->pFileName);
    cv::Mat captured_image;
    video_capture >> captured_image;
    if (cx_undefined) {
        cx = captured_image.cols / 2.0f;
        cy = captured_image.rows / 2.0f;
    }
    // Use a rough guess-timate of focal length
    if (fx_undefined) {
        fx = 500 * (captured_image.cols / 640.0);
        fy = 500 * (captured_image.rows / 480.0);

        fx = (fx + fy) / 2.0;
        fy = fx;
    }
    double fps = 10;
    SendMessage(ctx->hwnd, WM_FRAME_STATUS, NULL, (LONG_PTR)TEXT("Обработка..."));
    while (it < ctx->iFrameCount && *ctx->is_processing) {
        if (it > 0)
            fi.iProcessPercent = (it * 100) / ctx->iFrameCount;
        fi.iFrame = it;
        fi.iQuality = 0;
        auto start = timeGetTime();
        {
            cv::Mat_<float> depth_image;
            cv::Mat_<uchar> grayscale_image;
            auto disp_image = captured_image.clone();

            if (captured_image.channels() == 3) {
                cv::cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
            } else {
                grayscale_image = captured_image.clone();
            }

            // The actual facial landmark detection / tracking
            bool detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, depth_image, clnf_model, det_params);
            if (detection_success)
                ++fi.iGoodFrames;
            // Visualising the results
            // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
            double detection_certainty = clnf_model.detection_certainty;

            // Gaze tracking, absolute gaze direction
            cv::Point3f gazeDirection0(0, 0, -1);
            cv::Point3f gazeDirection1(0, 0, -1);

            if (det_params.track_gaze && detection_success && clnf_model.eye_model) {
                FaceAnalysis::EstimateGaze(clnf_model, gazeDirection0, fx, fy, cx, cy, true);
                FaceAnalysis::EstimateGaze(clnf_model, gazeDirection1, fx, fy, cx, cy, false);
            }
            if (detection_success)
                write_result(captured_image, depth_image, clnf_model, det_params, gazeDirection0, gazeDirection1, it, fx, fy, cx, cy);
            video_capture >> captured_image;

        }
        dwAllTime += (timeGetTime() - start);
        fi.dwMiddleTime = it > 0 ? (dwAllTime / it) : 0;
        fi.dwProcessTime = timeGetTime() - startProcess;
        SendMessage(ctx->hwnd, WM_FRAME_NEXT, 0, reinterpret_cast<LPARAM>(&fi));
        ++it;
    }
    clnf_model.Reset();
    return TRUE;
}

static INT
EndProcess(VideoPluginContext *ctx)
{
    return TRUE;
}

INT
LoadPlugin(VideoPlugin *pc)
{
    pc->lpstrPluginName = TEXT("OpenFace");
    pc->wVersionMajor = 3;
    pc->wVersionMinor = 6;
    pc->pFree = reinterpret_cast<pfnFreePlugin>(FreePlugin);
    pc->isActive = pc->prop->getBoolean("plugins.open_face.enabled", true);
    pc->pStartProcess = StartProcess;
    pc->pStopProcess = EndProcess;
    return TRUE;
}

INT
FreePlugin(VideoPlugin *pluginContext)
{
    return TRUE;
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





