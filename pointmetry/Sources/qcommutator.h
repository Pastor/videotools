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
    explicit QCommutator(QObject *parent, uint threads);
    ~QCommutator();

signals:
    void landmarksUpdated(const cv::Mat &img, float *pointer, uint length);
    void frametimeUpdated(double value);
    void eyesdistanceUpdated(double value);
    void facerectUpdated(const cv::Rect &rect);

    void imageProcessed();
    void imageUpdated(const cv::Mat &img);

public slots:
    void search_single(const cv::Mat &img);

private:
    int m_size;
    int m_pos;
    double m_frametime;
    double m_time;

    std::vector<QThread*> v_threads;
    std::vector<QStasm*> v_stasms;
    std::vector<bool> v_busy;

    void initialize();

private slots:
    int searchFreeThread();
    void updateQueue(int id);
};

#endif // QCOMMUTATOR_H
