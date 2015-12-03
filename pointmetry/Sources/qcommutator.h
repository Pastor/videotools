#ifndef QCOMMUTATOR_H
#define QCOMMUTATOR_H

#include <QObject>
#include <QThread>
#include "qstasm.h"

class QCommutator : public QObject
{
    Q_OBJECT
public:
    explicit QCommutator(QObject *parent = 0);
    ~QCommutator();

signals:
    void landmarksUpdated(const cv::Mat &img, float *pointer, uint length);
    void frametimeUpdated(double value);
    void eyesdistanceUpdated(double value);
    void facerectUpdated(const cv::Rect &rect);

    void imageUpdated(const cv::Mat &img);

public slots:
    void search_single(const cv::Mat &img);

private:
    double m_frametime;
    double m_time;
    int m_size;
    int m_pos;

    std::vector<QThread*> v_threads;
    std::vector<QStasm*> v_stasms;
};

#endif // QCOMMUTATOR_H
