#ifndef QSTASM_H
#define QSTASM_H

#include <QObject>
#include "opencv2/opencv.hpp"
#include "stasm_lib.h"

class QStasm : public QObject
{
    Q_OBJECT
public:
    explicit QStasm(QObject *parent = 0);

signals:
    void landmarksUpdated(const cv::Mat &img, float *pointer, uint length);
    void frametimeUpdated(double value);
    void facesEnds();

public slots:
   void search_single(const cv::Mat &image);

private:
   float pt_landmarks[2 * stasm_NLANDMARKS];
   float pt_buffer[2 * stasm_NLANDMARKS];
   double m_frametime;
   double m_time;
};

#endif // QSTASM_H
