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

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "libcommon.lib")

typedef std::unordered_map<long, int>   FrameLenCollection;
typedef float real;

static DWORD dwCtxIndex;
static DWORD dwProcessId;

struct EyesContext {
	CvHaarClassifierCascade *left;
	CvHaarClassifierCascade *right;
	CvMemStorage            *storage;
    Logger                  *logCalculator;
    Logger                  *logMetainfo;
    /** */
	real					 eyesdistanceTreshold;
	real					 contrastTreshold;
	real				  	 sharpnessTreshold;
	real				 	 snrTreshold;
	/** */
	FrameLenCollection      *collection;
	long                     lFramesCount;
};

static void __inline
__Get(struct EyesContext **pCtx)
{
    (*pCtx) = static_cast<struct EyesContext *>(TlsGetValue(dwCtxIndex));
}

static bool __inline
__IsSet(real value, real minValue, real maxValue)
{
    if (value < minValue)
        return false;
    if (maxValue == -1)
        return true;
    return value <= maxValue;
}

static bool __inline
__checkTreshold(real value, real treshold)
{
    return value >= treshold;
}

static void __calculateHistogram(const cv::Mat &inputImage, real *blue, real *green, real *red);
static real __calculateGlobalContrast(const cv::Mat &inputImage);
static real __calculateSharpness(const cv::Mat &inputImage);
static real __calculateSNR(const cv::Mat &inputImage);
static real __calculateEyesDistance(const cv::Point_<real> &leftEyeCenter, const cv::Point_<real> &rightEyeCenter);
static bool __searchEyes(const cv::Mat &inputFace, struct EyesContext *ctx, cv::Point_<real> &leftEye, cv::Point_<real> &rightEye);

static void
__FreeClassifiers(struct EyesContext *ctx)
{
    __try {
        cvReleaseHaarClassifierCascade(&(ctx->left));
        cvReleaseHaarClassifierCascade(&(ctx->right));
        cvReleaseMemStorage(&(ctx->storage));
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }
}

static void
__FreeEyesContext(struct EyesContext *ctx)
{
	if (ctx != nullptr) {
        __FreeClassifiers(ctx);
        if (ctx->logCalculator != nullptr)
            delete ctx->logCalculator;
        ctx->logCalculator = nullptr;
        if (ctx->logMetainfo != nullptr)
            delete ctx->logMetainfo;
        ctx->logMetainfo = nullptr;
		if (ctx->collection != nullptr)
			delete ctx->collection;
		ctx->collection = nullptr;
	}
}

INT
LoadPlugin(VideoPlugin *pc)
{
	pc->lpstrPluginName = TEXT("Quality");
	pc->wVersionMajor = 1;
	pc->wVersionMinor = 3;
	pc->pFree = reinterpret_cast<pfnFreePlugin>(FreePlugin);
	pc->pProcessFrame = reinterpret_cast<pfnProcessFrame>(ProcessFrame);
	pc->pStartProcess = reinterpret_cast<pfnStartProcess>(StartProcess);
	pc->pStopProcess = reinterpret_cast<pfnStopProcess>(StopProcess);
    pc->isActive = pc->prop->getBoolean("plugins.quality.enabled", true);

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
    struct EyesContext *ctx;

    __Get(&ctx);
	if (frameContext->seqFaces != nullptr && frameContext->seqFaces->total > 0) {
		auto left = ctx->left;
		auto right = ctx->right;

		if (left != nullptr && right != nullptr) {
			auto rect = reinterpret_cast<CvRect *>(cvGetSeqElem(frameContext->seqFaces, 0));
			auto realRect = cv::Rect(rect->x, rect->y, rect->width, rect->height);
			cv::Mat realFace(mat, realRect);

			cv::Point_<real> leftEye, rightEye;
			if (__searchEyes(realFace, ctx, leftEye, rightEye)) {
				eyesDistance = __calculateEyesDistance(leftEye, rightEye);
			}
		}
	}
	/*MESSAGE: Расчеты */
    auto contrast = __calculateGlobalContrast(mat);
    auto sharpness = __calculateSharpness(mat);
    auto snr = __calculateSNR(mat);
	auto result = __checkTreshold(eyesDistance, ctx->eyesdistanceTreshold) &&
				  __checkTreshold(contrast, ctx->contrastTreshold) &&
				  __checkTreshold(sharpness, ctx->sharpnessTreshold) &&
				  __checkTreshold(snr, ctx->snrTreshold);
	//TODO: куда результаты вычислений отдавать?
    ctx->logCalculator->printf(TEXT("%08d|     %5.01f|   %04.03f|   %04.03f|  %05.01f|        %d|"), frameContext->iFrame, eyesDistance, contrast, sharpness, snr, (result ? 1 : 0));
	if (result) {
		++ctx->lFramesCount;
	} else {
		if (ctx->lFramesCount > 0) {
			(*ctx->collection)[ctx->lFramesCount]++;
		}
		ctx->lFramesCount = 0;
	}
    frameContext->iQuality += (result ? 1 : 0);
	return TRUE;
}

INT
StartProcess(VideoPluginStartContext *startContext)
{
    struct EyesContext *ctx;

    __Get(&ctx);
    {
        auto path = absFilePath("haarcascade_mcs_lefteye.xml");
        auto fileTemplate = std::string(startContext->pFileTemplate);
        if (ctx->logCalculator == nullptr) {
            ctx->logCalculator = new Logger((fileTemplate + ".quality").c_str());
        }
        if (ctx->logMetainfo == nullptr) {
            ctx->logMetainfo = new Logger((fileTemplate + ".metainf").c_str());
        }
        if (ctx->left == nullptr) {
            ctx->left = static_cast<CvHaarClassifierCascade *>(cvLoad(path.c_str()));
            if (ctx->left == nullptr) {
                /**FIXME: Ошибка загрузки левого */
                ctx->logCalculator->printf(TEXT("Файл %ls не найден"), std::toString(path).c_str());
                return FALSE;
            }
        }
        if (ctx->right == nullptr) {
            path = absFilePath("haarcascade_mcs_righteye.xml");
            ctx->right = static_cast<CvHaarClassifierCascade *>(cvLoad(path.c_str()));
            if (ctx->right == nullptr) {
                /**FIXME: Ошибка загрузки правого */
                ctx->logCalculator->printf(TEXT("Файл %ls не найден"), std::toString(path).c_str());
                cvReleaseHaarClassifierCascade(&ctx->left);
                return FALSE;
            }
        }
        if (ctx->storage == nullptr) {
            ctx->storage = cvCreateMemStorage(0);
        }  
		if (ctx->collection == nullptr) {
			ctx->collection = new FrameLenCollection;
		}
    }
	/**TODO: Загружаем */
    ctx->logMetainfo->event(++dwProcessId);
    ctx->logMetainfo->printf(TEXT("Открывается файл %ls"), std::toString(startContext->pFileName).c_str());
    ctx->logMetainfo->printf(TEXT("Кадров в секунду: %d"), startContext->fps);
    ctx->logMetainfo->printf(TEXT("Ширина кадра    : %d"), startContext->iWidth);
    ctx->logMetainfo->printf(TEXT("Высота кадра    : %d"), startContext->iHeight);
    ctx->logMetainfo->printf(TEXT("Всего кадров    : %d"), startContext->iFrameCount);
	
	ctx->eyesdistanceTreshold = startContext->prop->getFloat("limits.frame.distance.treshold", -1);   
    ctx->contrastTreshold = startContext->prop->getFloat("limits.frame.contrast.treshold", -1);
    ctx->sharpnessTreshold = startContext->prop->getFloat("limits.frame.sharp.treshold", -1);
    ctx->snrTreshold = startContext->prop->getFloat("limits.frame.noise.treshold", -1);
    ctx->logMetainfo->printf(TEXT("Пороговое значение расстояния между глазами: %05.03f"), ctx->eyesdistanceTreshold);
    ctx->logMetainfo->printf(TEXT("Пороговое значение контраста               : %05.03f"), ctx->contrastTreshold);
    ctx->logMetainfo->printf(TEXT("Пороговое значение отношения сигнал шум    : %05.03f"), ctx->snrTreshold);
    ctx->logMetainfo->printf(TEXT("Пороговое значение резкости                : %05.03f"), ctx->sharpnessTreshold);
    ctx->logCalculator->printf(TEXT("    Кадр|Расстояние|Контраст|Резкость|    Шум|Результат|"));
	return TRUE;
}

INT
StopProcess(VideoPluginStartContext *startContext)
{
	FILE *fd;
	struct EyesContext *ctx;

	__Get(&ctx);
	if (ctx->lFramesCount > 0) {
		(*ctx->collection)[ctx->lFramesCount]++;
	}
	ctx->lFramesCount = 0;
	const auto &cc = (*ctx->collection);
	std::map<long, int> ordered(cc.begin(), cc.end());
    auto fileTemplate = std::string(startContext->pFileTemplate);

	fopen_s(&fd, (fileTemplate + ".chipses").c_str(), "a");
//	fprintf(fd, "Для файла: %s\n", startContext->pFileName);
	fprintf(fd, "|Продолжительность|Количество|\n");
	for (auto it = ordered.begin(); it != ordered.end(); ++it) {
		fprintf(fd, "|        %09d| %09d|\n", (*it).first, (*it).second);
	}
	fflush(fd);
	fclose(fd);
    /**TODO: MetaInf */
	{
        auto total = startContext->dwTotalTime;
        auto sec = total / 1000 % 60;
        auto minutes = total / 1000 / 60 % 60;
        auto hours = total / 1000 / 60 / 60 % 24;
        ctx->logMetainfo->printf(TEXT("Общее время работы                         : %02d:%02d:%02d"), hours, minutes, sec);
        ctx->logMetainfo->printf(TEXT("Всего кадров                               : %06d"), startContext->iFrameCount);
        ctx->logMetainfo->printf(TEXT("Обработано кадров                          : %06d"), startContext->iFrameProcessed);
        ctx->logMetainfo->printf(TEXT("Обработано хороших                         : %06d, %d%%"), startContext->iFrameGood, (startContext->iFrameGood * 100) / startContext->iFrameCount);
        total = startContext->dwTotalDetectTime;
        sec = total / 1000 % 60;
        minutes = total / 1000 / 60 % 60;
        hours = total / 1000 / 60 / 60 % 24;
        ctx->logMetainfo->printf(TEXT("Общее время поиска лица                    : %02d:%02d:%02d"), hours, minutes, sec);
	}
    
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
            lpvData = static_cast<LPVOID>(LocalAlloc(LPTR, sizeof(struct EyesContext)));
            if (lpvData != nullptr)
                fIgnore = TlsSetValue(dwCtxIndex, lpvData);
        }        
		break;

	case DLL_THREAD_DETACH:
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData != nullptr) {
            __FreeEyesContext(static_cast<struct EyesContext *>(lpvData));
            LocalFree(static_cast<HLOCAL>(lpvData));
        }
		break;

	case DLL_PROCESS_DETACH:
        dwProcessId = 0;
        lpvData = TlsGetValue(dwCtxIndex);
        if (lpvData != nullptr) {
            __FreeEyesContext(static_cast<struct EyesContext *>(lpvData));
            LocalFree(static_cast<HLOCAL>(lpvData));
        }
        TlsFree(dwCtxIndex);
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
	for (auto i = 0; i < 256; i++) {
		blue[i] = pointer[i];
	}

	channels[0] = 1;
	cv::calcHist(&input, 1, channels, cv::Mat(), // mask not used
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	pointer = hist.ptr<float>(0);
	for (auto i = 0; i < 256; i++) {
		green[i] = pointer[i];
	}

	channels[0] = 2;
	cv::calcHist(&input, 1, channels, cv::Mat(), // mask not used
		hist, 1, histSize, ranges,
		true, // the histogram is uniform
		false);
	pointer = hist.ptr<float>(0);
	for (auto i = 0; i < 256; i++) {
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
	for (auto i = 0; i < 256; i++) {
		meanBlue += i * blue[i];
		meanGreen += i * green[i];
		meanRed += i * red[i];
	}
	meanBlue /= area;
	meanGreen /= area;
	meanRed /= area;

	for (auto i = 0; i < 256; i++) {
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

	__Detect(ctx->left, ctx->storage, &static_cast<IplImage>(face), v_eyes, nullptr, 1.05, 11, 0 | CV_HAAR_FIND_BIGGEST_OBJECT, 20, 20);
	if (v_eyes.size() > 0) {
		leftEye = cv::Point_<real>(v_eyes[0].x + static_cast<real>(v_eyes[0].width) / 2.0, v_eyes[0].y + static_cast<real>(v_eyes[0].height) / 2.0);
	} else {
		return false;
	}
	v_eyes.clear();
	roi = roi + cv::Point(face.cols / 2, 0);
	cv::Mat topRightPart(face, roi);
	__Detect(ctx->right, ctx->storage, &static_cast<IplImage>(face), v_eyes, nullptr, 1.05, 11, 0 | CV_HAAR_FIND_BIGGEST_OBJECT, 20, 20);
	if (v_eyes.size() > 0) {
		v_eyes[0] = v_eyes[0] + cv::Point(face.cols / 2, 0);
		rightEye = cv::Point_<real>(v_eyes[0].x + static_cast<real>(v_eyes[0].width) / 2.0, v_eyes[0].y + static_cast<real>(v_eyes[0].height) / 2.0);
	} else {
		return false;
	}
	return true;
}


