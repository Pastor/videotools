#include "qvideowriter.h"

QVideoWriter::QVideoWriter(QObject *parent) :
    QObject(parent)
{
}

void QVideoWriter::updateFrame(const cv::Mat &img, float *v_landmarks, uint length)
{
    if(m_writer.isOpened()) {
        cv::Mat output = img.clone();
        for(uint i = 0; i < length/2; i++) {
            cv::circle(output, cv::Point(v_landmarks[2*i], v_landmarks[2*i+1]), 1, cv::Scalar(0,255,0), 2);
        }
        m_writer.write(output);
    }
}

void QVideoWriter::release()
{
    m_writer.release();
}

bool QVideoWriter::startRecordToFile(std::string filename, int codec, double fps, cv::Size frameSize)
{
    return m_writer.open(filename, codec, fps, frameSize);
}
