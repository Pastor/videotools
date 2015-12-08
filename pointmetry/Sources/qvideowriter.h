#ifndef QVIDEOWRITER_H
#define QVIDEOWRITER_H

#include <QObject>
#include "opencv2/opencv.hpp"

class QVideoWriter : public QObject
{
    Q_OBJECT
public:
    explicit QVideoWriter(QObject *parent = 0);

signals:

public slots:
    void updateFrame(const cv::Mat &img, float *v_landmarks, uint length);
    void release();
    bool startRecordToFile(std::string filename, int codec = CV_FOURCC('M','P','4','2'), double fps = 15.0, cv::Size frameSize = cv::Size(640, 480));

private:
    cv::VideoWriter m_writer;

};

#endif // QVIDEOWRITER_H
