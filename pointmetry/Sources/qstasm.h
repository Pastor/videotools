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
    void eyesdistanceUpdated(double value);
    void facerectUpdated(const cv::Rect &rect);
    void doneWork(int ID);

public slots:
   void search_single(const cv::Mat &image);
   void setID(int value);

private:
   float pt_landmarks[2 * stasm_NLANDMARKS];
   double m_frametime;
   double m_time;
   int m_id;
};

#endif // QSTASM_H
