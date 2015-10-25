#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/core/types_c.h>
#include <opencv2/objdetect.hpp>
#include <opencv2/objdetect/objdetect_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <fftw3.h>
#include <haar.h>
#include "library.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif
#define OBJECT_MINSIZE2         169
#define BOTTOM_LIMIT           0.7 // in s^-1, it is 42 bpm
#define TOP_LIMIT              4.5 // in s^-1, it is 270 bpm
#define SNR_TRESHOLD           1.5 // a treshold for the frequency update
#define HALF_INTERVAL          2   // defines the number of averaging indexes when frequency is evaluated, this value should be >= 1
#define DIGITAL_FILTER_LENGTH  2   // in counts
#define WATCH_DOG_TRESHOLD     1000 // expressed in milliseconds, it is period of doCompute() call for frequency update


class HardThread {
    enum ColorChannel { Red, Green, Blue };
public:
    HardThread(short lenData, short lenSize);
    ~HardThread();
    bool loadClassifier(const std::string &fileName);
    bool doProcess(const cv::Mat &mat, ProcessResult * result, int time);
protected:
    bool doOpencvPart(const cv::Mat &mat, ProcessResult * result, int time);
    bool doOneColorHarmonic(unsigned long color, unsigned long area, int time, ProcessResult * result);
    bool doRGBHarmonic(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, int time, ProcessResult * result);
    bool doCompute(ProcessResult *result);
    bool isSkinColor(unsigned char red, unsigned char green, unsigned char blue);
private:
    /** OpenCV */
    int                   m_framePeriod;
    //    cv::CascadeClassifier m_classifier;
    CvHaarClassifierCascade  *m_classifier;
    CvMemStorage             *m_storage;
    /** Harmonic */
    double               *ptCNSignal;  //a pointer to centered and normalized data (typedefinition from fftw3.h, a single precision complex float number type)
    fftw_complex         *ptSpectrum;  // a pointer to an array for FFT-spectrum
    double                SNRE; // a variable for signal-to-noise ratio estimation storing
    double               *ptData_ch1; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    double               *ptData_ch2; //a pointer to spattialy averaged data (you should use it to write data to an instance of a class)
    double               *ptDataForFFT; //a pointer to data prepared for FFT
    double               *ptAmplitudeSpectrum;
    double                ch1_mean;   //a variable for mean value in channel1 storing
    double                ch2_mean;   //a variable for mean value in channel2 storing
    int                  *ptTime; //a pointer to an array for frame periods storing (values in milliseconds thus unsigned int)
    double                HRfrequency; //a variable for storing a last evaluated frequency of the 'strongest' harmonic
    unsigned int          curpos; //a current position I meant
    short                 datalength; //a length of data array
    short                 bufferlength; //a lenght of sub data array for FFT (bufferlength should be <= datalength)
    double               *ptX; // a pointer to input counts history, for digital filtration
    short                 loop(short) const; //a function that return a loop-index
    short                 loop_for_ptX(short) const; //a function that return a loop-index
    bool                  RGB_flag;
    fftw_plan             m_plan;
    ColorChannel          m_channel; // determines which color channel is enrolled by WriteToDataOneColor(...) method
    double               *pt_Youtput; // a pointer to a vector of digital filter output
    double               *pt_Xoutput; // for intermediate result storage
    short                 m_watchDog;
};

inline short
HardThread::loop(short difference) const
{
    return ((datalength + (difference % datalength)) % datalength); // have finded it on wikipedia ), it always returns positive result
}
//---------------------------------------------------------------------------
inline short
HardThread::loop_for_ptX(short difference) const
{
    return ((DIGITAL_FILTER_LENGTH + (difference % DIGITAL_FILTER_LENGTH)) % DIGITAL_FILTER_LENGTH);
}
//---------------------------------------------------------------------------

#define MAGIC_NUMBER    0x00112233
#pragma pack(push, 1)
struct __SessionHandle {
    unsigned long                magic;
    SessionConfig                config;
    HardThread                  *hard;
};
#pragma pack(pop)

SessionHandle
CreateSession(SessionConfig * pConfig)
{
    auto hSession = static_cast<struct __SessionHandle *>(calloc(1, sizeof(struct __SessionHandle)));
    hSession->magic = MAGIC_NUMBER;
    std::memcpy(&hSession->config, pConfig, sizeof(SessionConfig));
    hSession->hard = new HardThread(256/**pConfig->iHarmonicDataSize*/, 256/*pConfig->iHarmonicBuffSize*/);
    if (!hSession->hard->loadClassifier(pConfig->szClassifierFile)) {
        DestroySession(hSession);
        return nullptr;
    }
    return hSession;
}

void
DestroySession(SessionHandle hSession)
{
    if (hSession != nullptr) {
        if (static_cast<struct __SessionHandle *>(hSession)->magic == MAGIC_NUMBER) {
            auto pSession = static_cast<struct __SessionHandle *>(hSession);
            delete pSession->hard;
            free(pSession);
        }
    }
}

int
ProcessSession(SessionHandle hSession, void *pImage, ProcessResult * pResult, int time)
{
    if (hSession != nullptr &&
        (static_cast<struct __SessionHandle *>(hSession)->magic) == MAGIC_NUMBER) {
        auto pSession = static_cast<struct __SessionHandle *>(hSession);
        auto mat = cv::cvarrToMat(static_cast<IplImage *>(pImage));
        return pSession->hard->doProcess(mat, pResult, time);
    }
    return FALSE;
}

HardThread::HardThread(short lenData, short lenSize)
    : SNRE(-5.0),
    ch1_mean(0.0),
    ch2_mean(0.0),
    HRfrequency(0.0),
    curpos(0),
    datalength(lenData),
    bufferlength(lenSize),
    RGB_flag(false),
    m_channel(Green),
    m_watchDog(WATCH_DOG_TRESHOLD)
{
    ptData_ch1 = new double[datalength];
    ptData_ch2 = new double[datalength];
    ptCNSignal = new double[datalength];
    ptTime = new int[datalength];
    ptX = new double[DIGITAL_FILTER_LENGTH];
    ptDataForFFT = new double[bufferlength];
    ptSpectrum = static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * (bufferlength / 2 + 1)));
    ptAmplitudeSpectrum = new double[bufferlength / 2 + 1];
    m_plan = fftw_plan_dft_r2c_1d(bufferlength, ptDataForFFT, ptSpectrum, FFTW_ESTIMATE);

    // Vectors initialization
    for (long i = 0; i < datalength; i++) {
        ptData_ch1[i] = 0.0;
        ptData_ch2[i] = 0.0;
        ptTime[i] = 60; // just for ensurance that at the begining there will not be any "division by zero"
        ptCNSignal[i] = 0.0;
    }
}

HardThread::~HardThread()
{
    fftw_destroy_plan(m_plan);
    delete[] ptData_ch1;
    delete[] ptData_ch2;
    delete[] ptCNSignal;
    delete[] ptTime;
    delete[] ptX;
    delete[] ptDataForFFT;
    fftw_free(ptSpectrum);
    delete[] ptAmplitudeSpectrum;
    if (m_classifier != nullptr)
        cvReleaseHaarClassifierCascade(&m_classifier);
    m_classifier = nullptr;
    if (m_storage != nullptr)
        cvReleaseMemStorage(&m_storage);
    m_storage = nullptr;
}

bool
HardThread::loadClassifier(const std::string & fileName)
{
    m_classifier = static_cast<CvHaarClassifierCascade *>(cvLoad(fileName.c_str()));
    if (m_classifier != nullptr) {
        m_storage = cvCreateMemStorage(0);
    }
    return m_classifier != nullptr;
}

bool
HardThread::doProcess(const cv::Mat &mat, ProcessResult * result, int time)
{
    return doOpencvPart(mat, result, time);
}

bool
HardThread::doOpencvPart(const cv::Mat & mat, ProcessResult * result, int time)
{
    cv::Mat output(mat);  // Copy the header and pointer to data of input object
    cv::Mat gray; // Create an instance of cv::Mat for temporary image storage
    cv::cvtColor(output, gray, CV_BGR2GRAY);
    cv::equalizeHist(gray, gray);
    std::vector<cv::Rect> faces_vector;

    __Detect(m_classifier, m_storage, &static_cast<IplImage>(gray), faces_vector, nullptr, 1.1, 11, CV_HAAR_DO_ROUGH_SEARCH | CV_HAAR_FIND_BIGGEST_OBJECT, OBJECT_MINSIZE2, OBJECT_MINSIZE2);
    unsigned long red = 0; // an accumulator for red color channel
    unsigned long green = 0; // an accumulator for green color channel
    unsigned long blue = 0; // an accumulator for blue color channel
    unsigned long area = 0; // an accumulator for the enrolled area in pixels

    if (faces_vector.size() != 0) { // if classifier has found something, then do...

        unsigned char *p; // this pointer will be used to store adresses of the image rows
        unsigned char tRed, tBlue, tGreen;
        unsigned int X = faces_vector[0].x; // take actual coordinate
        unsigned int Y = faces_vector[0].y; // take actual coordinate
        unsigned int rectwidth = faces_vector[0].width; // take actual size
        unsigned int rectheight = faces_vector[0].height; // take actual size
        auto dX = rectwidth / 4; // the horizontal portion of rect domain that will be enrolled
        auto dY = rectheight / 13; //...

        if (output.channels() == 3) {
            for (auto j = Y - dY; j < Y + rectheight + dY; j++) {
                if (static_cast<int>(j) > output.rows)
                    break;
                p = output.ptr(j); //takes pointer to beginning of data on row
                for (auto i = X; i < X + rectwidth; i++) {
                    tRed = p[3 * i + 2];
                    tGreen = p[3 * i + 1];
                    tBlue = p[3 * i];
                    if (isSkinColor(tRed, tGreen, tBlue) == true) {
                        area++;
                        blue += tBlue;
                        green += tGreen;
                        red += tRed;
                    }
                }
            }
        } else {
            for (auto j = (Y + dY); j < (Y + 3 * dY); j++) {
                p = output.ptr(j); //pointer to beginning of data on rows
                for (auto i = (X + dX); i < (X + rectwidth - dX); i++) {
                    green += p[i];
                }
            }
            blue = green;
            red = green;
            area = (rectwidth - 2 * dX) * (5 * dY);
        }
    }
    //-----end of if(faces_vector.size() != 0)-----

    m_framePeriod = time;
    result->iPeriod = m_framePeriod;
    m_watchDog -= time;

    if (area > 0) {
        if (RGB_flag == true) {
            return doRGBHarmonic(red, green, blue, area, m_framePeriod, result);
        }
        switch (m_channel){
        case Red:
            return doOneColorHarmonic(red, area, m_framePeriod, result);
        case Green:
            return doOneColorHarmonic(green, area, m_framePeriod, result);
        case Blue:
            return doOneColorHarmonic(blue, area, m_framePeriod, result);
        }
    }
    result->qValue1 = -1.0; // will signal that classifier has not found any objects
    result->qValue2 = -1.0; // will signal that classifier has not found any objects
    return false;
}

bool
HardThread::doRGBHarmonic(unsigned long red, unsigned long green, unsigned long blue, unsigned long area, int time, ProcessResult * result)
{
    auto ch1_temp = static_cast<double>(red - green) / area;
    auto ch2_temp = static_cast<double>(red + green - 2 * blue) / area;

    ch1_mean += (ch1_temp - ptData_ch1[curpos]) / datalength;
    ch2_mean += (ch2_temp - ptData_ch2[curpos]) / datalength;

    ptData_ch1[curpos] = ch1_temp;
    ptData_ch2[curpos] = ch2_temp;
    ptTime[curpos] = time;

    auto ch1_sko = 0.0;
    auto ch2_sko = 0.0;
    for (short i = 0; i < datalength; i++) {
        ch1_sko += (ptData_ch1[i] - ch1_mean)*(ptData_ch1[i] - ch1_mean);
        ch2_sko += (ptData_ch2[i] - ch2_mean)*(ptData_ch2[i] - ch2_mean);
    }
    ch1_sko = sqrt(ch1_sko / (datalength - 1));
    ch2_sko = sqrt(ch2_sko / (datalength - 1));

    ptX[loop_for_ptX(curpos)] = (ptData_ch1[curpos] - ch1_mean) / ch1_sko - (ptData_ch2[curpos] - ch2_mean) / ch2_sko;
    ptCNSignal[curpos] = (ptX[loop_for_ptX(curpos)] + ptCNSignal[loop(curpos - 1)]) / 2;

    curpos = (++curpos) % datalength; // for loop-like usage of ptData and the other arrays in this class
    return doCompute(result);
}

bool
HardThread::doOneColorHarmonic(unsigned long color, unsigned long area, int time, ProcessResult * result)
{
    auto ch1_temp = static_cast<double>(color) / area;

    ch1_mean += (ch1_temp - ptData_ch1[curpos]) / datalength;

    ptData_ch1[curpos] = ch1_temp;
    ptTime[curpos] = time;

    auto ch1_sko = 0.0;
    for (short i = 0; i < datalength; i++) {
        ch1_sko += (ptData_ch1[i] - ch1_mean) * (ptData_ch1[i] - ch1_mean);
    }
    ch1_sko = sqrt(ch1_sko / (datalength - 1));

    ptX[loop_for_ptX(curpos)] = (ptData_ch1[curpos] - ch1_mean) / ch1_sko;
    ptCNSignal[curpos] = (ptX[loop_for_ptX(curpos)] + ptCNSignal[loop(curpos - 1)]) / 2;

    curpos = (++curpos) % datalength; // for loop-like usage of ptData and the other arrays in this class    
    return doCompute(result);
}

bool
HardThread::doCompute(ProcessResult * result)
{
    if (m_watchDog <= 0)  {
        short temp_position = curpos - 1;
        auto buffer_duration = 0.0;

        for (short i = 0; i < bufferlength; i++) {
            auto pos = loop(temp_position - (bufferlength - 1) + i);
            ptDataForFFT[i] = ptCNSignal[pos];
            buffer_duration += ptTime[pos];
        }

        fftw_execute(m_plan); // Data were prepared, execute fftw_plan

        for (short i = 0; i < (bufferlength / 2 + 1); i++) {
            ptAmplitudeSpectrum[i] = ptSpectrum[i][0] * ptSpectrum[i][0] + ptSpectrum[i][1] * ptSpectrum[i][1];
        }
        short bottom_bound = static_cast<unsigned int>(BOTTOM_LIMIT * buffer_duration / 1000.0);
        short top_bound = static_cast<unsigned int>(TOP_LIMIT * buffer_duration / 1000.0);
        if (top_bound > bufferlength / 2 + 1)
            top_bound = bufferlength / 2 + 1;
        short index_of_maxpower = 0;
        auto maxpower = 0.0;
        for (short i = (bottom_bound + HALF_INTERVAL); i < (top_bound - HALF_INTERVAL); i++) {
            auto temp_power = ptAmplitudeSpectrum[i];
            if (maxpower < temp_power) {
                maxpower = temp_power;
                index_of_maxpower = i;
            }
        }
        /*-------------------------SNR estimation evaluation-----------------------*/
        auto noise_power = 0.0;
        auto signal_power = 0.0;
        for (auto i = bottom_bound; i < top_bound; i++) {
            if ((i >= (index_of_maxpower - HALF_INTERVAL)) && (i <= (index_of_maxpower + HALF_INTERVAL)))    {
                signal_power += ptAmplitudeSpectrum[i];
            } else  {
                noise_power += ptAmplitudeSpectrum[i];
            }
        }
        SNRE = 10 * log10(signal_power / noise_power);
        auto power = 0.0;
        auto power_multiplyed_by_index = 0.0;
        for (short i = (index_of_maxpower - HALF_INTERVAL + 1); i <= (index_of_maxpower + HALF_INTERVAL); i++) {
            power += ptAmplitudeSpectrum[i];
            power_multiplyed_by_index += i * ptAmplitudeSpectrum[i];
        }
        auto bias = static_cast<double>(index_of_maxpower) - (power_multiplyed_by_index / power);
        bias = sqrt(bias * bias);
        auto weight = (HALF_INTERVAL + 1 - bias) / (HALF_INTERVAL + 1);
        SNRE *= weight  * weight * weight * weight; // four multiplication in a row for "more" nonlinearity

        if (SNRE > SNR_TRESHOLD) {
            HRfrequency = (power_multiplyed_by_index / power) * 60000.0 / buffer_duration; // result will be expressed in minutes
        }
        m_watchDog = WATCH_DOG_TRESHOLD; // reset m_watchDog
    }

    result->qValue1 = HRfrequency;
    result->qValue2 = SNRE;
    return true;
}

bool inline
HardThread::isSkinColor(unsigned char red, unsigned char green, unsigned char blue)
{
    //Modified Kovac's rule
    if ((red > 115) &&
        (red > green) && (blue > 45) &&
        ((red - std::min(green, blue)) > 35) && // for the std::min() you have to provide #include <algorithm>, whitch contains in std library since C++98
        ((red - green) > 25)) {
        return true;
    }
    return false;
}
