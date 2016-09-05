#define _CRT_SECURE_NO_WARNINGS
#include <xstring.h>
#include <logger.h>
#include <haar.h>
#include "wrap.h"
#include <opencv2/imgproc.hpp>
#include <LandmarkCoreIncludes.h>
#include <GazeEstimation.h>

#include <Windows.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")

#pragma warning(disable: 4297)

static DWORD dwCtxIndex;
static DWORD dwProcessId;

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
__DestroyContext(struct ProcessContext *ctx)
{
    __try {
        if (ctx) {
            if (ctx->clnf_model) {
                ctx->clnf_model->Reset();
                delete ctx->clnf_model;
            }
            ctx->clnf_model = nullptr;
            ctx->cx_undefined = true;
            ctx->fx_undefined = true;
            delete[] ctx->points;
            if (ctx->fd)
                fclose(ctx->fd);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER)
    {

    }
}

void
create_wrap(const char * const modelPath)
{
    ProcessContext *ctx;

    __Get(&ctx);
    destroy_wrap();
    ctx->clnf_model = new LandmarkDetector::CLNF(modelPath);
    ctx->det_params = LandmarkDetector::FaceModelParameters();
    ctx->det_params.track_gaze = true;
    fprintf(ctx->fd, "START\n");
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

    auto captured_image = cv::cvarrToMat(pImage);
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
        cv::Mat_<float> depth_image;
        cv::Mat_<uchar> grayscale_image;
        auto disp_image = captured_image.clone();

        if (captured_image.channels() == 3) {
            cv::cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
        } else {
            grayscale_image = captured_image.clone();
        }
        fprintf(ctx->fd, "Cols: %d\n", captured_image.cols);
        fprintf(ctx->fd, "Rows: %d\n", captured_image.rows);
        fflush(ctx->fd);
        // The actual facial landmark detection / tracking
        auto detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, depth_image, *ctx->clnf_model, ctx->det_params);
        auto detection_certainty = (*ctx->clnf_model).detection_certainty;
        DEBUG();
        // Gaze tracking, absolute gaze direction
        cv::Point3f gazeDirection0(0, 0, -1);
        cv::Point3f gazeDirection1(0, 0, -1);
        DEBUG();
        if (ctx->det_params.track_gaze && detection_success && (*ctx->clnf_model).eye_model) {
            FaceAnalysis::EstimateGaze((*ctx->clnf_model), gazeDirection0, ctx->fx, ctx->fy, ctx->cx, ctx->cy, true);
            FaceAnalysis::EstimateGaze((*ctx->clnf_model), gazeDirection1, ctx->fx, ctx->fy, ctx->cx, ctx->cy, false);
        }
        DEBUG();
        (*nSize) = 0;
        (*p) = ctx->points;
        const auto visualisation_boundary = 0.2;
        if (detection_success/* && detection_certainty < visualisation_boundary*/) {
            auto n = ctx->clnf_model->detected_landmarks.rows / 2;
            (*nSize) = n;
            for (auto i = 0; i < n; ++i) {
                cv::Point featurePoint(
                    static_cast<int>(ctx->clnf_model->detected_landmarks.at<double>(i)),
                    static_cast<int>(ctx->clnf_model->detected_landmarks.at<double>(i + n)));
                (*p)[i].x = featurePoint.x;
                (*p)[i].y = featurePoint.y;
            }
            DEBUG();
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
    //    __DestroyContext(ctx);
}

static void
write_ldots(FILE *fd, const cv::Mat_<double>& shape2D)
{
    auto n = shape2D.rows / 2;
    for (auto i = 0; i < n; ++i) {
        cv::Point featurePoint(static_cast<int>(shape2D.at<double>(i)), static_cast<int>(shape2D.at<double>(i + n)));
        fprintf(fd, ";%05d;%05d", featurePoint.x, featurePoint.y);
    }
}

static cv::Point3f
GetPupilPosition(cv::Mat_<double> eyeLdmks3d)
{

    eyeLdmks3d = eyeLdmks3d.t();

    cv::Mat_<double> irisLdmks3d = eyeLdmks3d.rowRange(0, 8);

    cv::Point3f p(mean(irisLdmks3d.col(0))[0], mean(irisLdmks3d.col(1))[0], mean(irisLdmks3d.col(2))[0]);
    return p;
}

static void
write_gaze(FILE *fd, cv::Mat img, const LandmarkDetector::CLNF& clnf_model, cv::Point3f gazeVecAxisLeft, cv::Point3f gazeVecAxisRight, float fx, float fy, float cx, float cy)
{

    cv::Mat cameraMat = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 0);

    auto part_left = -1;
    auto part_right = -1;
    for (size_t i = 0; i < clnf_model.hierarchical_models.size(); ++i) {
        if (clnf_model.hierarchical_model_names[i].compare("left_eye_28") == 0) {
            part_left = i;
        }
        if (clnf_model.hierarchical_model_names[i].compare("right_eye_28") == 0) {
            part_right = i;
        }
    }

    cv::Mat eyeLdmks3d_left = clnf_model.hierarchical_models[part_left].GetShape(fx, fy, cx, cy);
    auto pupil_left = GetPupilPosition(eyeLdmks3d_left);

    cv::Mat eyeLdmks3d_right = clnf_model.hierarchical_models[part_right].GetShape(fx, fy, cx, cy);
    auto pupil_right = GetPupilPosition(eyeLdmks3d_right);

    vector<cv::Point3d> points_left;
    points_left.push_back(cv::Point3d(pupil_left));
    points_left.push_back(cv::Point3d(pupil_left + gazeVecAxisLeft*50.0));

    vector<cv::Point3d> points_right;
    points_right.push_back(cv::Point3d(pupil_right));
    points_right.push_back(cv::Point3d(pupil_right + gazeVecAxisRight*50.0));

    cv::Mat_<double> proj_points;
    cv::Mat_<double> mesh_0 = (cv::Mat_<double>(2, 3) << points_left[0].x, points_left[0].y, points_left[0].z, points_left[1].x, points_left[1].y, points_left[1].z);
    LandmarkDetector::Project(proj_points, mesh_0, fx, fy, cx, cy);
    //    line(img, cv::Point(proj_points.at<double>(0, 0), proj_points.at<double>(0, 1)), cv::Point(proj_points.at<double>(1, 0), proj_points.at<double>(1, 1)), cv::Scalar(110, 220, 0), 2, 8);
    fprintf(fd, ";%05d;%05d", static_cast<int>(proj_points.at<double>(0, 0)), static_cast<int>(proj_points.at<double>(0, 1)));
    fprintf(fd, ";%05d;%05d", static_cast<int>(proj_points.at<double>(1, 0)), static_cast<int>(proj_points.at<double>(1, 1)));

    cv::Mat_<double> mesh_1 = (cv::Mat_<double>(2, 3) << points_right[0].x, points_right[0].y, points_right[0].z, points_right[1].x, points_right[1].y, points_right[1].z);
    LandmarkDetector::Project(proj_points, mesh_1, fx, fy, cx, cy);
    //    line(img, cv::Point(proj_points.at<double>(0, 0), proj_points.at<double>(0, 1)), cv::Point(proj_points.at<double>(1, 0), proj_points.at<double>(1, 1)), cv::Scalar(110, 220, 0), 2, 8);
    fprintf(fd, ";%05d;%05d", static_cast<int>(proj_points.at<double>(0, 0)), static_cast<int>(proj_points.at<double>(0, 1)));
    fprintf(fd, ";%05d;%05d", static_cast<int>(proj_points.at<double>(1, 0)), static_cast<int>(proj_points.at<double>(1, 1)));
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





