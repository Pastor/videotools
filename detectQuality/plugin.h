#include <Windows.h>
#include <videoplugin.h>

#if defined(__cplusplus)
extern "C" {
#endif
    __declspec(dllexport)
        INT LoadPlugin(VideoPlugin *pluginContext);

    __declspec(dllexport)
        INT FreePlugin(VideoPlugin *pluginContext);

    __declspec(dllexport)
        INT ProcessFrame(VideoPluginFrameContext *frameContext);

    __declspec(dllexport)
        INT StartProcess(VideoPluginStartContext *startContext);

    __declspec(dllexport)
        INT StopProcess(VideoPluginStartContext *startContext);
#if defined(__cplusplus)
}
#endif

//-------------------------------------------------
//static cv::CascadeClassifier m_facesClassifier;
//static cv::CascadeClassifier m_eyesClassifier;

typedef float real;

static void __calculateHistogram(const cv::Mat &inputImage, real *blue, real *green, *red);
static real __calculateGlobalContrast(const cv::Mat &inputImage);
static real __calculateSharpness(const cv::Mat &inputImage);
static real __calculateSNR(const cv::Mat &inputImage);
//static real __calculateEyesDistance(const cv::Mat &inputImage);
//-------------------------------------------------


