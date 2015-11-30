#include "qstasm.h"

QStasm::QStasm(QObject *parent) :
    QObject(parent)
{
    stasm_init( QString(DIRECTORY_OF_FACE_DETECTOR_FILES).toUtf8().data() , 0);
}

void QStasm::search_single(const cv::Mat &image)
{
    cv::Mat temp;

    if(image.channels() == 3)
        cv::cvtColor(image, temp, CV_BGR2GRAY);
    else if(image.channels() == 1)
        temp = image;

    stasm_open_image((char*)temp.data,
                     temp.cols,
                     temp.rows,
                     "",
                     0,
                     15);
    int facesFound = 0;

    if( stasm_search_auto(&facesFound, pt_landmarks)) {
        for(uint i = 0; i < 2 * stasm_NLANDMARKS; i++)
            pt_buffer[i] = pt_landmarks[i];
        emit landmarksUpdated(image, pt_buffer, 2 * stasm_NLANDMARKS);
    } else emit facesEnds();

    m_frametime = (cv::getTickCount() -  m_time) * 1000.0 / cv::getTickFrequency(); // result is calculated in milliseconds
    m_time = cv::getTickCount();
    emit frametimeUpdated(m_frametime);
}
