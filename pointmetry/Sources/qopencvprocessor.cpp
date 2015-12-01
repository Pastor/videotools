/*------------------------------------------------------------------------------------------------------
Taranov Alex, 2014									     SOURCE FILE
Class that wraps opencv functions into Qt SIGNAL/SLOT interface
The simplest way to use it - rewrite appropriate section in QOpencvProcessor::custom_algorithm(...) slot
------------------------------------------------------------------------------------------------------*/

#include "qopencvprocessor.h"

//------------------------------------------------------------------------------------------------------

QOpencvProcessor::QOpencvProcessor(QObject *parent):
    QObject(parent)
{
    //Initialization
    m_cvRect.width = 0;
    m_cvRect.height = 0;
    m_timeCounter = cv::getTickCount();
}

//-----------------------------------------------------------------------------------------------------

void QOpencvProcessor::update_timeCounter()
{
    m_timeCounter = cv::getTickCount();
}

//------------------------------------------------------------------------------------------------------

void QOpencvProcessor::custom_algorithm(const cv::Mat &input)
{
    //-------------CUSTOM ALGORITHM--------------

    //qWarning("Input imnage have %d channels", input.channels());
    //You can do here almost whatever you wnat...
    //cv::cvtColor(input,output,CV_BGR2GRAY);
    //cv::equalizeHist(temp,temp);
    //cv::Canny(temp,output,20,500);

    //----------END OF CUSTOM ALGORITHM----------

    //---------Drawing of rectangle--------------
    /*if( (m_cvRect.width > 0) && (m_cvRect.height > 0) )
    {
        cv::rectangle( output , m_cvRect, cv::Scalar(255,255,255)); // white color
    }*/

    //-------------Time measurement--------------
    __calculateContrast(input);
    //__calculateSharpness(input);
    __calculateSNR(input);

    m_framePeriod = (cv::getTickCount() -  m_timeCounter) * 1000.0 / cv::getTickFrequency(); // result is calculated in milliseconds
    m_timeCounter = cv::getTickCount();

    emit frame_was_processed(input, m_framePeriod);
}

//------------------------------------------------------------------------------------------------------

void QOpencvProcessor::setRect(const cv::Rect &input_rect)
{
     m_cvRect = input_rect;
}

//------------------------------------------------------------------------------------------------------

void QOpencvProcessor::__calculateHist(const cv::Mat &input)
{
    /*	Computes histogramm for B, G & R channels of CV_U8 cv::Mat image	*
    *	and returns histogram as cv::Mat object					*/

        int bins = 256;
        int histSize[] = {bins};
        float marginalRanges[] = {0, 256};
        const float* ranges[] = { marginalRanges };
        int channels[] = {0};
        cv::Mat histBlue, histGreen, histRed;
        cv::calcHist( &input, 1, channels, cv::Mat(), // mask not used
                 histBlue, 1, histSize, ranges,
                 true, // the histogram is uniform
                 false );
        channels[0] = 1;
        cv::calcHist( &input, 1, channels, cv::Mat(), // mask not used
                 histGreen, 1, histSize, ranges,
                 true, // the histogram is uniform
                 false );
        channels[0] = 2;
        cv::calcHist( &input, 1, channels, cv::Mat(), // mask not used
                 histRed, 1, histSize, ranges,
                 true, // the histogram is uniform
                 false );
        float *ptBlue = histBlue.ptr<float>(0);
        float *ptGreen = histGreen.ptr<float>(0);
        float *ptRed = histRed.ptr<float>(0);
        for(int i = 0 ; i < bins; i++ )
        {
            v_blueHist[i] = ptBlue[i];
            v_greenHist[i] = ptGreen[i];
            v_redHist[i] = ptRed[i];
        }
        emit blueHistogramUpdated(v_blueHist, 256);
        emit redHistogramUpdated(v_redHist, 256);
        emit greenHistogramUpdated(v_greenHist, 256);
}

//------------------------------------------------------------------------------------------------------

qreal QOpencvProcessor::__calculateSharpness(const cv::Mat &input)
{
    cv::Mat temp;
    cv::Sobel(input, temp, CV_8U, 1, 1);
    cv::Scalar v_sharp = cv::sum(temp);
    qreal sharpness = std::sqrt((v_sharp[0]*v_sharp[0] + v_sharp[1]*v_sharp[1] + v_sharp[2]*v_sharp[2]))/(255.0*input.cols*input.rows);
    emit sharpnessUpdated(sharpness);
    qWarning("Sharpness: %f\t, %f, %f, %f", sharpness , v_sharp[0], v_sharp[1], v_sharp[2]);
    return sharpness;
}

qreal QOpencvProcessor::__calculateContrast(const cv::Mat &input)
{
    __calculateHist(input);
    qreal area = input.cols*input.rows;
    qreal meanBlue = 0.0;
    qreal meanGreen = 0.0;
    qreal meanRed = 0.0;
    for(int i = 0; i < 256; i++) {
        meanBlue += i * v_blueHist[i];
        meanGreen += i * v_greenHist[i];
        meanRed += i * v_redHist[i];
    }
    meanBlue /= area;
    meanGreen /= area;
    meanRed /= area;

    qreal skoBlue = 0.0;
    qreal skoGreen = 0.0;
    qreal skoRed = 0.0;
    for(int i = 0; i < 256; i++) {
        skoBlue += (i - meanBlue)*(i - meanBlue)*v_blueHist[i];
        skoGreen += (i - meanGreen)*(i - meanGreen)*v_greenHist[i];
        skoRed += (i - meanRed)*(i - meanRed)*v_redHist[i];
    }
    qreal Contrast = std::sqrt((skoBlue + skoGreen + skoRed) / area) / 255.0;
    emit contrastUpdated(Contrast);
    qWarning("Contrast: %f", Contrast);
    return Contrast;
}

qreal QOpencvProcessor::__calculateSNR(const cv::Mat &input)
{
    cv::Mat temp;
    cv::Laplacian(input, temp, CV_8U);
    cv::Scalar v_stDev;
    cv::meanStdDev(temp, cv::Scalar(), v_stDev);
    qreal snr = 20* std::log10(255.0 / std::sqrt(v_stDev[0]*v_stDev[0] + v_stDev[1]*v_stDev[1] + v_stDev[2]*v_stDev[2]));
    qWarning("SNR: %f", snr);
    emit snrUpdated(snr);
    return snr;
}
