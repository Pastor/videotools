#include "qstasm.h"

QStasm::QStasm(QObject *parent) :
    QObject(parent)
{
    #ifdef DIRECTORY_OF_FACE_DETECTOR_FILES
        stasm_init( QString(DIRECTORY_OF_FACE_DETECTOR_FILES).toUtf8().data() , 0);
    #else
        stasm_init( "data/" , 0);
    #endif
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
        cv::Rect rect = stasm_get_face_rect();
        emit facerectUpdated(rect);
        emit landmarksUpdated(image, pt_landmarks, 2 * stasm_NLANDMARKS);
        emit eyesdistanceUpdated( std::sqrt((pt_landmarks[2*38] - pt_landmarks[2*39])*(pt_landmarks[2*38] - pt_landmarks[2*39]) +
                                            (pt_landmarks[2*38+1] - pt_landmarks[2*39+1])*(pt_landmarks[2*38+1] - pt_landmarks[2*39+1])));
    } else emit facesEnds();

    m_frametime = (cv::getTickCount() -  m_time) * 1000.0 / cv::getTickFrequency(); // result is calculated in milliseconds
    m_time = cv::getTickCount();
    emit frametimeUpdated(m_frametime);
}
