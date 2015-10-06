//#include <opencv2/core/types_c.h>
#include <xstring.h>
#include "plugin.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>

//----------------------------------------------------------------------
//cv::CascadeClassifie m_facesClassifier;
//cv::CascadeClassifier m_eyesClassifier;
//----------------------------------------------------------------------


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
    /**TODO: А теперь работаем с кадром */	
	real contrast = __calculateGlobalContrast(mat);
 	real sharpness = __calculateSharpness(mat);
 	real snr = __calculateSNR(mat);
	//real eyeDistance = __calculateEyesDistance(mat);
	//TO DO: куда результаты вычислений отдавать?

    return TRUE;
}

INT
StartProcess(VideoPluginStartContext *startContext)
{
    	//? Мы можем загружать файлы в объекты здесь?
	//m_facesClassifier.load("haarcascades/haarcascade_frontalface_alt.xml"); // Нужно будет добавить эти файлы к проекту
	//m_eyesClassifier.load("haarcascades/haarcascade_eye.xml");
	
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

//-------------------------------------------------------------
void __calculateHistogram(const cv::Mat &inputImage, real *blue, real *green, *red)
{
/* Calculates histograms of inputImage and copies them into input vectors,		*
 * it is caller responsability to allocate memory for them, each needs float[256]	*/ 
	
	int bins = 256;
	int histSize[] = {bins};
	float marginalRanges[] = {0, 256};
	const float* ranges[] = { marginalRanges };
	int channels[] = {0};
	cv::Mat hist;
	cv::calcHist( &input, 1, channels, cv::Mat(), // mask not used
	         hist, 1, histSize, ranges,
	         true, // the histogram is uniform
	         false );
	float *pointer = hist.ptr<float>(0);
	for(int i = 0; i < 256; i++) {
		blue[i] = pointer[i];
	}

	channels[0] = 1;
	cv::calcHist( &input, 1, channels, cv::Mat(), // mask not used
	         hist, 1, histSize, ranges,
	         true, // the histogram is uniform
	         false );
	pointer = hist.ptr<float>(0);
	for(int i = 0; i < 256; i++) {
		green[i] = pointer[i];
	}

	channels[0] = 2;
	cv::calcHist( &input, 1, channels, cv::Mat(), // mask not used
	         hist, 1, histSize, ranges,
	         true, // the histogram is uniform
	         false );
	pointer = hist.ptr<float>(0);
	for(int i = 0; i < 256; i++) {
		red[i] = pointer[i];
	}	
} 

real __calculateGlobalContrast(const cv::Mat &inputImage)
{
	real blue[256], green[256], red[256];	
	__calculateHistogram(inputImage, blue, green, red);
	
	real area = inputImage.cols * inputImage.rows;
	real meanBlue = 0.0;
	real meanGreen = 0.0;
	real meanRed = 0.0;
	for(int i = 0; i < 256; i++) {
		meanBlue += i * blue[i];
		meanGreen += i * green[i];
		meanRed += i * red[i];
	}
	meanBlue /= area;
	meanGreen /= area;
	meanRed /= area;  
	
	real skoBlue = 0.0;
	real skoGreen = 0.0;
	real skoRed = 0.0;
	for(int i = 0; i < 256; i++) {
		skoBlue += (i - meanBlue)*(i - meanBlue)*blue[i];
		skoGreen += (i - meanGreen)*(i - meanGreen)*green[i];
		skoRed += (i - meanRed)*(i - meanRed)*red[i];
	}
	real contrast = std::sqrt((skoBlue + skoGreen + skoRed) / area) / 255.0 ;	
	return contrast;
}

real __calculateSharpness(const cv::Mat &inputImage)
{
	cv::Mat tempImage;
	cv::Sobel(inputImage, tempImage, cv::CV_8U, 1, 1);
	cv::Scalar v_sharp = cv::sum(tempImage);
	real sharpness = std::sqrt((v_sharp[0]*v_sharp[0] + v_sharp[1]*v_sharp[1] + v_sharp[2]*v_sharp[2]))/(input.cols * input.rows * 255.0);
	return sharpness;
}

real __calculateSNR(const cv::Mat &inputImage)
{
	cv::Mat tempImage;
    cv::Laplacian(inputImage, tempImage, CV_8U);
    cv::Scalar v_stDev;
    cv::meanStdDev(tempImage, cv::Scalar(), v_stDev);
    real snr = 20.0 * std::log10(255.0 / std::sqrt(v_stDev[0]*v_stDev[0] + v_stDev[1]*v_stDev[1] + v_stDev[2]*v_stDev[2]));
	return snr;
}

/*real __calculateEyesDistance(const cv::Mat &inputImage)
{
	//TO DO...
	return 0.0;
}*/

//-------------------------------------------------------------


