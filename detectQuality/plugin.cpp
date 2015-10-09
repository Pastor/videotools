#include <strsafe.h>
#include <shlobj.h>
#include <Dbt.h>
#include <shlobj.h>
#include <Shlwapi.h>
#include <xstring.h>
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

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "libcommon.lib")

typedef float real;

struct EyesContext {
	CvHaarClassifierCascade *left;
	CvHaarClassifierCascade *right;
	CvMemStorage            *storage;
};

static void __calculateHistogram(const cv::Mat &inputImage, real *blue, real *green, real *red);
static real __calculateGlobalContrast(const cv::Mat &inputImage);
static real __calculateSharpness(const cv::Mat &inputImage);
static real __calculateSNR(const cv::Mat &inputImage);
static real __calculateEyesDistance(const cv::Point_<real> &leftEyeCenter, const cv::Point_<real> &rightEyeCenter);
static bool __searchEyes(const cv::Mat &inputFace, struct EyesContext *ctx, cv::Point_<real> &leftEye, cv::Point_<real> &rightEye);

static void
__FreeEyesContext(LPVOID *ctx)
{
	if (ctx != nullptr && *ctx != nullptr) {
		cvReleaseHaarClassifierCascade(&(((struct EyesContext *)ctx)->left));
		cvReleaseHaarClassifierCascade(&(((struct EyesContext *)ctx)->right));
		cvReleaseMemStorage(&(((struct EyesContext *)ctx)->storage));
		LocalFree(*ctx);
		*ctx = nullptr;
	}
}

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

	real eyesDistance = -1.0;
	if (frameContext->seqFaces != nullptr && frameContext->seqFaces->total > 0) {
		auto left = ((struct EyesContext *)frameContext->plugin->pUserContext)->left;
		auto right = ((struct EyesContext *)frameContext->plugin->pUserContext)->right;

		if (left != nullptr && right != nullptr) {
			auto rect = reinterpret_cast<CvRect *>(cvGetSeqElem(frameContext->seqFaces, 0));
			auto realRect = cv::Rect(rect->x, rect->y, rect->width, rect->height);
			cv::Mat realFace(mat, realRect);

			cv::Point_<real> leftEye, rightEye;
			if (__searchEyes(realFace, ((struct EyesContext *)frameContext->plugin->pUserContext), leftEye, rightEye)) {
				eyesDistance = __calculateEyesDistance(leftEye, rightEye);
			}
		}
	}
	/*MESSAGE: Расчеты */
	real contrast = __calculateGlobalContrast(mat);
	real sharpness = __calculateSharpness(mat);
	real snr = __calculateSNR(mat);
	//TODO: куда результаты вычислений отдавать?

	return TRUE;
}

INT
StartProcess(VideoPluginStartContext *startContext)
{
	__FreeEyesContext(&startContext->plugin->pUserContext);
	startContext->plugin->pUserContext = LocalAlloc(LPTR, sizeof(struct EyesContext));
	RtlSecureZeroMemory(startContext->plugin->pUserContext, sizeof(struct EyesContext));

	/**TODO: Загружаем */
	{
		char szBuffer[1024 + 40];
		bool ret;

		GetModuleFileNameA(nullptr, szBuffer, sizeof(szBuffer) - 40);
		PathRemoveFileSpecA(szBuffer);
		PathCombineA(szBuffer, szBuffer, "haarcascade_mcs_lefteye.xml");
		((struct EyesContext *)startContext->plugin->pUserContext)->left = static_cast<CvHaarClassifierCascade *>(cvLoad(szBuffer));
		if (((struct EyesContext *)startContext->plugin->pUserContext)->left == nullptr) {
			/**FIXME: Ошибка загрузки левого */
			
			return FALSE;
		}
		GetModuleFileNameA(nullptr, szBuffer, sizeof(szBuffer) - 40);
		PathRemoveFileSpecA(szBuffer);
		PathCombineA(szBuffer, szBuffer, "haarcascade_mcs_righteye.xml");
		((struct EyesContext *)startContext->plugin->pUserContext)->right = static_cast<CvHaarClassifierCascade *>(cvLoad(szBuffer));
		if (((struct EyesContext *)startContext->plugin->pUserContext)->right == nullptr) {
			/**FIXME: Ошибка загрузки правого */
			cvReleaseHaarClassifierCascade(&((struct EyesContext *)startContext->plugin->pUserContext)->left);
			return FALSE;
		}
		((struct EyesContext *)startContext->plugin->pUserContext)->storage = cvCreateMemStorage(0);
	}
	return TRUE;
}

INT
StopProcess(VideoPluginStartContext *startContext)
{
	__FreeEyesContext(&startContext->plugin->pUserContext);
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

//-------------------------------------------------------------
void __calculateHistogram(const cv::Mat &input, real *blue, real *green, real *red)
{
	/* Calculates histograms of inputImage and copies them into input vectors,		    *
	 * it is caller responsability to allocate memory for them, each needs float[256]	*/

	int bins = 256;
	int histSize[] = { bins };
	float marginalRanges[] = { 0, 256 };
	const float* ranges[] = { marginalRanges };
	int channels[] = { 0 };
	cv::Mat hist;
	cv::calcHist(&input, 1, channels, cv::Mat(), // mask not used
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	auto pointer = hist.ptr<float>(0);
	for (int i = 0; i < 256; i++) {
		blue[i] = pointer[i];
	}

	channels[0] = 1;
	cv::calcHist(&input, 1, channels, cv::Mat(), // mask not used
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	pointer = hist.ptr<float>(0);
	for (int i = 0; i < 256; i++) {
		green[i] = pointer[i];
	}

	channels[0] = 2;
	cv::calcHist(&input, 1, channels, cv::Mat(), // mask not used
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	pointer = hist.ptr<float>(0);
	for (int i = 0; i < 256; i++) {
		red[i] = pointer[i];
	}
}

real __calculateGlobalContrast(const cv::Mat &inputImage)
{
	real blue[256], green[256], red[256];
	real meanBlue = 0.0;
	real meanGreen = 0.0;
	real meanRed = 0.0;
	real skoBlue = 0.0;
	real skoGreen = 0.0;
	real skoRed = 0.0;
	real area = inputImage.cols * inputImage.rows;

	__calculateHistogram(inputImage, blue, green, red);
	for (int i = 0; i < 256; i++) {
		meanBlue += i * blue[i];
		meanGreen += i * green[i];
		meanRed += i * red[i];
	}
	meanBlue /= area;
	meanGreen /= area;
	meanRed /= area;

	for (int i = 0; i < 256; i++) {
		skoBlue += (i - meanBlue)*(i - meanBlue)*blue[i];
		skoGreen += (i - meanGreen)*(i - meanGreen)*green[i];
		skoRed += (i - meanRed)*(i - meanRed)*red[i];
	}
	auto contrast = static_cast<real>(std::sqrt((skoBlue + skoGreen + skoRed) / area) / 255.0);
	return static_cast<real>(contrast);
}

real __calculateSharpness(const cv::Mat &inputImage)
{
	cv::Mat tempImage;
	cv::Sobel(inputImage, tempImage, CV_8U, 1, 1);
	cv::Scalar v_sharp = cv::sum(tempImage);
	real sharpness = std::sqrt((v_sharp[0] * v_sharp[0] + v_sharp[1] * v_sharp[1] + v_sharp[2] * v_sharp[2])) / (inputImage.cols * inputImage.rows * 255.0);
	return sharpness;
}

real __calculateSNR(const cv::Mat &inputImage)
{
	cv::Mat tempImage;
	cv::Laplacian(inputImage, tempImage, CV_8U);
	cv::Scalar v_stDev;
	cv::meanStdDev(tempImage, cv::Scalar(), v_stDev);
	real snr = 20.0 * std::log10(255.0 / std::sqrt(v_stDev[0] * v_stDev[0] + v_stDev[1] * v_stDev[1] + v_stDev[2] * v_stDev[2]));
	return snr;
}

real __calculateEyesDistance(const cv::Point_<real> &leftEyeCenter, const cv::Point_<real> &rightEyeCenter)
{
	cv::Point_<real> difference(rightEyeCenter - leftEyeCenter);
	return std::sqrt(difference.x * difference.x + difference.y * difference.y);
}

bool __searchEyes(const cv::Mat &face, struct EyesContext *ctx, cv::Point_<real> &leftEye, cv::Point_<real> &rightEye)
{
	cv::Size minEyeSize(20, 20);
	std::vector<cv::Rect> v_eyes;

	cv::Rect roi(0, 0, face.cols / 2, face.rows / 2);
	cv::Mat topLeftPart(face, roi);

	__Detect(ctx->left, ctx->storage, &(IplImage)face, v_eyes, nullptr, 1.05, 11, 0 | CV_HAAR_FIND_BIGGEST_OBJECT, 20, 20);
	if (v_eyes.size() > 0) {
		leftEye = cv::Point_<real>(v_eyes[0].x + (real)v_eyes[0].width / 2.0, v_eyes[0].y + (real)v_eyes[0].height / 2.0);
	} else {
		return false;
	}
	v_eyes.clear();
	roi = roi + cv::Point(face.cols / 2, 0);
	cv::Mat topRightPart(face, roi);
	__Detect(ctx->right, ctx->storage, &(IplImage)face, v_eyes, nullptr, 1.05, 11, 0 | CV_HAAR_FIND_BIGGEST_OBJECT, 20, 20);
	if (v_eyes.size() > 0) {
		v_eyes[0] = v_eyes[0] + cv::Point(face.cols / 2, 0);
		rightEye = cv::Point_<real>(v_eyes[0].x + (real)v_eyes[0].width / 2.0, v_eyes[0].y + (real)v_eyes[0].height / 2.0);
	} else {
		return false;
	}
	return true;
}


